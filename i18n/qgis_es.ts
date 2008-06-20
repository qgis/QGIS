<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1">
<context>
    <name>Dialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="13"/>
        <source>QGIS Plugin Installer</source>
        <translation>Instalador de complementos de QGIS</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="142"/>
        <source>Name of plugin to install</source>
        <translation>Nombre del complemento a instalar</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="61"/>
        <source>Get List</source>
        <translation>Obtener lista</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="176"/>
        <source>Done</source>
        <translation>Hecho</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="152"/>
        <source>Install Plugin</source>
        <translation>Instalar complemento</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="163"/>
        <source>The plugin will be installed to ~/.qgis/python/plugins</source>
        <translation>El complemento se instalará en ~/.qgis/python/plugins</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="117"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="122"/>
        <source>Version</source>
        <translation>Versión</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="127"/>
        <source>Description</source>
        <translation>Descripción</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="132"/>
        <source>Author</source>
        <translation>Autor</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="19"/>
        <source>Select repository, retrieve the list of available plugins, select one and install it</source>
        <translation>Seleccionar repositorio, obtener la lista de complementos disponibles, seleccionar uno e instalarlo</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="26"/>
        <source>Repository</source>
        <translation>Repositorio</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="34"/>
        <source>Active repository:</source>
        <translation>Repositorio activo</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="81"/>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="88"/>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="95"/>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="55"/>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Bienvenido a su complemento generado automáticamente</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="56"/>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>Este es sólo un punto de inicio. Ahora necesita modificar el código para que haga algo útil... continúe leyendo para más información sobre cómo empezar.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="57"/>
        <source>Documentation:</source>
        <translation>Documentación:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="58"/>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>Ahora necesita leer la documentación del API de QGIS en:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="59"/>
        <source>In particular look at the following classes:</source>
        <translation>En particular mire las siguientes clases:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="62"/>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin es una ABC que define los comportamientos requeridos que su complemento debe proporcionar. Vea más abajo para más detalles.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="63"/>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>¿Para qué son todos los archivos generados en mi directorio de complementos?</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="64"/>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>Este es el archivo CMake que construye el complemento. Debería añadir las dependencias específicas de su aplicación y los archivos fuente a este archivo.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="65"/>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>Esta el la clase que proporciona el &quot;pegamento&quot; entre la lógica de su aplicación personal y la aplicación QGIS. Verá que ya hay implementados algunos métodos para usted - incluyendo algunos ejemplos de cómo añadir una capa ráster o vectorial a la vista principal del mapa de la aplicación. Esta clase es una instancia concreta de la interfaz QgisPlugin que define el comportamiento requerido para un complemento. En particular, un complemento tiene un número de métodos estáticos y miembros de forma que el QgsPluginManager y la lógica del cargador de complementos puedan identificar cada complemento, crear una entrada de menú apropiada para él, etc. Tenga en cuenta que no hay nada que le pare creando múltiples iconos de barras de herramientas y entradas de menú para un solo complemento. Sin embargo, de forma predeterminada sólo se crea una entrada de menú y un botón de barra de herramientas y está preconfigurado para llamar al método run() de esa clase cuando se selecciona. Esta implementación predeterminada proporcionada por el constructor de complementos está bien documentada, así que acuda al código para más información.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="66"/>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>Este es un archivo &quot;ui&quot; del diseñador de Qt. Define el aspecto del diálogo predeterminadado del complemento sin implementar ninguna lógica de la aplicación. Puede modificar este formulario para adaptarlo a sus necesidades o eliminarlo por completo si su complemento no necesita mostrar un formulario de usuario (por ej. para personalizar MapTools).</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="67"/>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>Esta es la clase concreta donde debe ir la lógica de la aplicación para el diálogo mencionado anteriormente. Aquí es donde está el quiz de la cuestión...</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="68"/>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>Este es el archivo de recursos de Qt4 para su complemento. El Makefile generado para su complemento está configurado por completo para compilar el archivo de recursos, así que todo lo que tiene que hacer es añadir sus iconos adicionales, etc. usando el sencillo formato de archivo xml. Fíjese en el namespace usado para todos sus recursos, ej. (&quot;:/[pluginname]/&quot;). Es importante usar este prefijo para todos sus recursos. Le sugerimos que incluya cualquier otra imagen y datos de tiempo de ejecución también en este archivo de recurso.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="69"/>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>Este es el icono que se usará para la entrada de menú e icono de barra de herramientas de su complemento. Simplemente sustituya este icono con el suyo para distinguirlo del resto.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="70"/>
        <source>This file contains the documentation you are reading now!</source>
        <translation>¡Este archivo contiene la documentación que está leyendo ahora!</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="71"/>
        <source>Getting developer help:</source>
        <translation>Obtener ayuda de los desarrolladores:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="72"/>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>Para preguntas y comentarios sobre la plantilla del constructor de complementos y la creación de sus elementos en QGIS usando la interfaz de complementos, por favor póngase en contacto con nosotros via:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="73"/>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; la lista de correo de desarrolladores de QGIS o&lt;/li&gt;&lt;li&gt; IRC (#qgis en freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="74"/>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS se distribuye bajo la Licencia Pública Gnu (GPL). Si crea un complemento útil, por favor valore ponerlo a su vez a disposición de la comunidad.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="75"/>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Diviértase y gracias por elegir QGIS.</translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="13"/>
        <source>Enter map coordinates</source>
        <translation>Introducir coordenadas de mapa</translation>
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
        <translation>&amp;Aceptar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="172"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="28"/>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>Introducir las coordenadas X e Y que correspondan con el punto seleccionado en la imagen. De forma alternativa, seleccionar el botón con el icono de un lápiz y luego hacer clic en un punto correspondiente sobre la vista del mapa de QGIS para rellenar las coordenadas de ese punto.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="137"/>
        <source> from map canvas</source>
        <translation> de la vista del mapa</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="108"/>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation>Guardar informe del experimento a formato de documento portátil (.pdf)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="666"/>
        <source>Load layer properties from style file (.qml)</source>
        <translation>Cargar propiedades de la capa de archivo de estilo (.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="727"/>
        <source>Save layer properties as style file (.qml)</source>
        <translation>Guardar propiedades de la capa como archivo de estilo (.qml)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="303"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Capturada una excepción del sistema de coordenadas mientras se intentaba transformar un punto.No se puede calcular la longitud de la línea.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="394"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation>Capturada una excepción del sistema de coordenadas mientras se intentaba transformar un punto.No se puede calcular el área del polígono.</translation>
    </message>
    <message>
        <location filename="../src/core/qgslabelattributes.cpp" line="58"/>
        <source>Label</source>
        <translation>Etiqueta</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="87"/>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation>No hay complementos de proveedores de datos</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="89"/>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>No se pueden cargar capas vectoriales. Compruebe su instalación de QGIS.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="92"/>
        <source>No Data Providers</source>
        <translation>No hay proveedores de datos</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="251"/>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>No hay complementos de proveedores de datos disponibles. No se pueden cargar capas vectoriales.</translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="289"/>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>No se ha encontrado la columna indicada: </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="293"/>
        <source>Division by zero.</source>
        <translation>División entre cero.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2995"/>
        <source>QGis files (*.qgs)</source>
        <translation>Archivos QGis (*.qgs)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="75"/>
        <source>No active layer</source>
        <translation>No hay capa activa</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="154"/>
        <source>Band</source>
        <translation>Banda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="454"/>
        <source>Length</source>
        <translation>Longitud</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="460"/>
        <source>Area</source>
        <translation>Área</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="467"/>
        <source>action</source>
        <translation>acción</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="476"/>
        <source> features found</source>
        <translation> objetos espaciales encontrados</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="480"/>
        <source> 1 feature found</source>
        <translation> 1 objeto espacial encontrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="486"/>
        <source>No features found</source>
        <translation>No se han encontrado objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="486"/>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>No se han encontrado objetos espaciales en la capa activa en el punto en el que se ha pinchado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="587"/>
        <source>Could not identify objects on</source>
        <translation>No se pudieron identificar objetos en</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="587"/>
        <source>because</source>
        <translation>porque</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source>Project file read error: </source>
        <translation>Error de lectura del archivo del proyecto: </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source> at line </source>
        <translation> en la línea </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="772"/>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="778"/>
        <source> for file </source>
        <translation> en el archivo </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="934"/>
        <source>Unable to save to file </source>
        <translation>No se puede guardar a un archivo </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="32"/>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>Ajustar a una transformación lineal requiere al menos 2 puntos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="71"/>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Ajustar a una transformación de Helmert requiere al menos 2 puntos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="123"/>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Ajustar a una transformación afín requiere al menos 4 puntos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="72"/>
        <source>New centroid</source>
        <translation>Nuevo centroide</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>New point</source>
        <translation>Nuevo punto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="134"/>
        <source>New vertex</source>
        <translation>Nuevo vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>Undo last point</source>
        <translation>Deshacer último punto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>Close line</source>
        <translation>Cerrar línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="543"/>
        <source>Select vertex</source>
        <translation>Seleccionar vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="296"/>
        <source>Select new position</source>
        <translation>Seleccionar nueva posición</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="427"/>
        <source>Select line segment</source>
        <translation>Seleccionar segmento de línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="414"/>
        <source>New vertex position</source>
        <translation>Posición del nuevo vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="414"/>
        <source>Release</source>
        <translation>Liberar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="530"/>
        <source>Delete vertex</source>
        <translation>Borrar vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="530"/>
        <source>Release vertex</source>
        <translation>Liberar vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="784"/>
        <source>Select element</source>
        <translation>Seleccionar elemento</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="597"/>
        <source>New location</source>
        <translation>Nueva localización</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="673"/>
        <source>Release selected</source>
        <translation>Liberar selección</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="673"/>
        <source>Delete selected / select next</source>
        <translation>Borrar selección / seleccionar siguiente</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="736"/>
        <source>Select position on line</source>
        <translation>Seleccionar posición en la línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="754"/>
        <source>Split the line</source>
        <translation>Dividir la línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="754"/>
        <source>Release the line</source>
        <translation>Liberar la línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="768"/>
        <source>Select point on line</source>
        <translation>Seleccionar punto en la línea</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="332"/>
        <source>Couldn&apos;t open the data source: </source>
        <translation>No se pudo abrir la fuente de datos: </translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="354"/>
        <source>Parse error at line </source>
        <translation>Error de análisis en la línea </translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="55"/>
        <source>GPS eXchange format provider</source>
        <translation>Proveedor de formato eXchange GPS</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="166"/>
        <source>GRASS plugin</source>
        <translation>Complemento de GRASS</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="136"/>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>QGIS no pudo encontrar su instalación de GRASS.
¿Podría especificar la ruta (GISBASE) de su instalación de GRASS?</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="150"/>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Seleccionar ruta de instalación de GRASS (GISBASE)</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="167"/>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>Los datos de GRASS no estarán accesibles si no se especifica una GISBASE.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="51"/>
        <source>CopyrightLabel</source>
        <translation>Etiqueta de Copyright</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="52"/>
        <source>Draws copyright information</source>
        <translation>Dibuja información de copyright</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="31"/>
        <source>Version 0.1</source>
        <translation>Versión 0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="44"/>
        <source>Version 0.2</source>
        <translation>Versión 0.2</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="45"/>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation>Carga y muestra archivos de texto delimitado que contienen coordenadas X, Y</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="161"/>
        <source>Add Delimited Text Layer</source>
        <translation>Añadir capa de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="57"/>
        <source>Georeferencer</source>
        <translation>Georreferenciador</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="58"/>
        <source>Adding projection info to rasters</source>
        <translation>Añadir información de proyección a los rásters</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="55"/>
        <source>GPS Tools</source>
        <translation>Herramientas de GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="57"/>
        <source>Tools for loading and importing GPS data</source>
        <translation>Herramientas para cargar e importar datos de GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="829"/>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="835"/>
        <source>GRASS layer</source>
        <translation>Capa de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="43"/>
        <source>Graticule Creator</source>
        <translation>Creador de cuadrícula</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="44"/>
        <source>Builds a graticule</source>
        <translation>Construye una cuadrícula</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="60"/>
        <source>NorthArrow</source>
        <translation>Flecha de Norte</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="61"/>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Muestra una flecha de Norte superpuesta en el mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="38"/>
        <source>[menuitemname]</source>
        <translation>[nombredeelementodemenu]</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="39"/>
        <source>[plugindescription]</source>
        <translation>[descripcióndecomplemento]</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="62"/>
        <source>ScaleBar</source>
        <translation>Barra de escala</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="63"/>
        <source>Draws a scale bar</source>
        <translation>Dibuja una barra de escala</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="37"/>
        <source>SPIT</source>
        <translation>SPIT</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="38"/>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>Herramienta de importación de shapefiles a PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="29"/>
        <source>WFS plugin</source>
        <translation>Complemento de WFS</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="30"/>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>Añade capas WFS a la vista de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="42"/>
        <source>Not a vector layer</source>
        <translation>No es una capa vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="43"/>
        <source>The current layer is not a vector layer</source>
        <translation>La capa actual no es vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="72"/>
        <source>Layer cannot be added to</source>
        <translation>La capa no se puede añadir a</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="73"/>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>El proveedor de datos de esta capa no da soporte para añadir objetos espaciales.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="49"/>
        <source>Layer not editable</source>
        <translation>Capa no editable</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="51"/>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>No se puede editar la capa vectorial. Para hacerla editable vaya al archivo de la capa, haga clic derecho y marque &apos;Permitir edición&apos;.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="76"/>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Para seleccionar objetos espaciales debe elegir una capa vectorial haciendo clic en su nombre en la leyenda</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="193"/>
        <source>Python error</source>
        <translation>Error de Python</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="477"/>
        <source>Couldn&apos;t load plugin </source>
        <translation>No se pudo cargar el complemento </translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="481"/>
        <source> due an error when calling its classFactory() method</source>
        <translation> debido a un error al llamar a su método classFactory()</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="485"/>
        <source> due an error when calling its initGui() method</source>
        <translation> debido a un error al llamar a su método initGui()</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="497"/>
        <source>Error while unloading plugin </source>
        <translation>Error al descargar complemento </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="59"/>
        <source>2.5D shape type not supported</source>
        <translation>El tipo shape 2.5D no está soportado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="59"/>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>Añadir objetos espaciales a tipo shape 2.5 aún no está soportado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="207"/>
        <source>Wrong editing tool</source>
        <translation>Herramienta de edición incorrecta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="92"/>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>No se puede aplicar la herramienta &apos;capturar punto&apos; en esta capa vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="65"/>
        <source>Coordinate transform error</source>
        <translation>Error de transformación de coordenadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="66"/>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>No se puede transformar el punto al sistema de coordenadas de las capas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="200"/>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>No se puede aplicar la herramienta &apos;capturar línea&apos; en esta capa vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="208"/>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>No se puede aplicar la herramienta &apos;capturar polígono&apos; en esta capa vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="427"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="416"/>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>No se puede añadir el objeto espacial. Tipo WKB desconocido.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdisland.cpp" line="113"/>
        <source>Error, could not add island</source>
        <translation>Error, no se pudo añadir la isla</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="91"/>
        <source>A problem with geometry type occured</source>
        <translation>Ocurrió un problema con el tipo de geometría</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="95"/>
        <source>The inserted Ring is not closed</source>
        <translation>El anillo insertado no está cerrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="99"/>
        <source>The inserted Ring is not a valid geometry</source>
        <translation>El anillo insertado no es una geometría válida</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="103"/>
        <source>The inserted Ring crosses existing rings</source>
        <translation>El anillo insertado cruza anillos existentes</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="107"/>
        <source>The inserted Ring is not contained in a feature</source>
        <translation>El anillo insertado no está contenido en un objeto espacial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="111"/>
        <source>An unknown error occured</source>
        <translation>Ocurrió un error desconocido</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="113"/>
        <source>Error, could not add ring</source>
        <translation>Error, no se pudo añadir el anillo</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="640"/>
        <source> km2</source>
        <translation> km²</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="645"/>
        <source> ha</source>
        <translation> Ha</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="650"/>
        <source> m2</source>
        <translation> m²</translation>
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
        <translation> milla²</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="692"/>
        <source> sq ft</source>
        <translation> pies²</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="699"/>
        <source> mile</source>
        <translation> milla</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="705"/>
        <source> foot</source>
        <translation> pie</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="707"/>
        <source> feet</source>
        <translation> pies</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="714"/>
        <source> sq.deg.</source>
        <translation> grados²</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="719"/>
        <source> degree</source>
        <translation> grado</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="721"/>
        <source> degrees</source>
        <translation> grados</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="725"/>
        <source> unknown</source>
        <translation> desconocido</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="273"/>
        <source>Received %1 of %2 bytes</source>
        <translation>Recibidos %1 de %2 bytes</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="279"/>
        <source>Received %1 bytes (total unknown)</source>
        <translation>Recibidos %1 bytes (total desconocido)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="390"/>
        <source>Not connected</source>
        <translation>No conectado</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="396"/>
        <source>Looking up &apos;%1&apos;</source>
        <translation>Buscando &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="403"/>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>Conectando a &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="410"/>
        <source>Sending request &apos;%1&apos;</source>
        <translation>Enviando petición &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="417"/>
        <source>Receiving reply</source>
        <translation>Recibiendo respuesta</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="423"/>
        <source>Response is complete</source>
        <translation>La respuesta está completa</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="429"/>
        <source>Closing down connection</source>
        <translation>Cerrando la conexión</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="753"/>
        <source>Unable to open </source>
        <translation>No se puede abrir </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="253"/>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation>Las expresiones regulares sobre valores numéricos no tienen sentido. Use la comparación en su lugar.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="48"/>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Funciones de geoprocesamiento para trabajar con capas PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="137"/>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Localización: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="137"/>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Directorio de mapas: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="141"/>
        <source>Location: </source>
        <translation>Localización: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="141"/>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Directorio de mapas: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="146"/>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation>&lt;b&gt;Ráster&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="155"/>
        <source>Cannot open raster header</source>
        <translation>No se puede abrir la cabecera del ráster</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="159"/>
        <source>Rows</source>
        <translation>Filas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="160"/>
        <source>Columns</source>
        <translation>Columnas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="161"/>
        <source>N-S resolution</source>
        <translation>Resolución N-S</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="162"/>
        <source>E-W resolution</source>
        <translation>Resolución E-W</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="300"/>
        <source>North</source>
        <translation>Norte</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="302"/>
        <source>South</source>
        <translation>Sur</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="304"/>
        <source>East</source>
        <translation>Este</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="306"/>
        <source>West</source>
        <translation>Oeste</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="189"/>
        <source>Format</source>
        <translation>Formato</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="200"/>
        <source>Minimum value</source>
        <translation>Valor mínimo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="201"/>
        <source>Maximum value</source>
        <translation>Valor máximo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="212"/>
        <source>Data source</source>
        <translation>Fuente de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="217"/>
        <source>Data description</source>
        <translation>Descripción de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="226"/>
        <source>Comments</source>
        <translation>Comentarios</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="241"/>
        <source>Categories</source>
        <translation>Categorías</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="347"/>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;Vectorial&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="274"/>
        <source>Points</source>
        <translation>Puntos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="275"/>
        <source>Lines</source>
        <translation>Líneas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="276"/>
        <source>Boundaries</source>
        <translation>Contornos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="277"/>
        <source>Centroids</source>
        <translation>Centroides</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="280"/>
        <source>Faces</source>
        <translation>Caras</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="281"/>
        <source>Kernels</source>
        <translation>Kernels</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="284"/>
        <source>Areas</source>
        <translation>Áreas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="285"/>
        <source>Islands</source>
        <translation>Islas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="309"/>
        <source>Top</source>
        <translation>Arriba</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="310"/>
        <source>Bottom</source>
        <translation>Abajo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="313"/>
        <source>yes</source>
        <translation>sí</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="313"/>
        <source>no</source>
        <translation>no</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="320"/>
        <source>History&lt;br&gt;</source>
        <translation>Historia&lt;br&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="348"/>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Capa&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="367"/>
        <source>Features</source>
        <translation>Objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="376"/>
        <source>Driver</source>
        <translation>Controlador</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="377"/>
        <source>Database</source>
        <translation>Base de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="378"/>
        <source>Table</source>
        <translation>Tabla</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="379"/>
        <source>Key column</source>
        <translation>Columna clave</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="453"/>
        <source>GISBASE is not set.</source>
        <translation>GISBASE no establecida.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="458"/>
        <source> is not a GRASS mapset.</source>
        <translation> no es un directorio de mapas de GRASS.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="480"/>
        <source>Cannot start </source>
        <translation>No se puede iniciar </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="497"/>
        <source>Mapset is already in use.</source>
        <translation>El directorio de mapas ya está en uso.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="512"/>
        <source>Temporary directory </source>
        <translation>El directorio temporal </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="512"/>
        <source> exist but is not writable</source>
        <translation> existe pero no se puede escribir</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="518"/>
        <source>Cannot create temporary directory </source>
        <translation>No se puede crear el directorio temporal </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="534"/>
        <source>Cannot create </source>
        <translation>No se puede crear </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="611"/>
        <source>Cannot remove mapset lock: </source>
        <translation>No se puede eliminar el bloqueo del directorio de mapas: </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1051"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="999"/>
        <source>Cannot read raster map region</source>
        <translation>No se puede la región del mapa ráster</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1016"/>
        <source>Cannot read vector map region</source>
        <translation>No se puede leer la región del mapa vectorial</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1052"/>
        <source>Cannot read region</source>
        <translation>No se puede leer la región</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2422"/>
        <source>Where is &apos;</source>
        <translation>¿Dónde está &apos;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2422"/>
        <source>original location: </source>
        <translation>localización original: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="123"/>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Para identificar objetos espaciales, debe activar una capa haciendo clic en su nombre en la leyenda</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="47"/>
        <source>PostgreSQL Geoprocessing</source>
        <translation>Geoprocesamiento PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="38"/>
        <source>Quick Print</source>
        <translation>Impresión rápida</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="40"/>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation>Impresión rápida es un complemento para imprimir rápidamente un mapa con el mínimo esfuerzo.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="427"/>
        <source>Could not remove polygon intersection</source>
        <translation>No se pudo eliminar la intersección de los polígonos</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Currently only filebased datasets are supported</source>
        <translation type="obsolete">Actualmente sólo hay soporte para conjuntos de datos basados en archivo</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="521"/>
        <source>Loaded default style file from </source>
        <translation>Se ha cargado el archivo de estilo predeterminado desde </translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="552"/>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation>Se tiene que poder escribir en el directorio que contiene su conjunto de datos.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="565"/>
        <source>Created default style file as </source>
        <translation>Se ha creado el archivo de estilo predeterminado </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>ERROR: Failed to created default style file as </source>
        <translation type="obsolete">ERROR: no se pudo crear el archivo de estilo predeterminado </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>File could not been opened.</source>
        <translation type="obsolete">No se pudo abrir el archivo</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="946"/>
        <source> is not writeable.</source>
        <translation> no se puede editar.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="946"/>
        <source>Please adjust permissions (if possible) and try again.</source>
        <translation>Por favor, ajuste los permisos (si es posible) y vuelva a intentarlo.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="63"/>
        <source>Couldn&apos;t load SIP module.</source>
        <translation>No se pudo cargar el módulo SIP.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="78"/>
        <source>Python support will be disabled.</source>
        <translation>El soporte para Python estará deshabilitado.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="71"/>
        <source>Couldn&apos;t load PyQt4.</source>
        <translation>No se pudo cargar PyQt4.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="78"/>
        <source>Couldn&apos;t load PyQGIS.</source>
        <translation>No se pudo cargar PyQGIS.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="89"/>
        <source>An error has occured while executing Python code:</source>
        <translation>Ha ocurrido un error mientras se ejecutaba el código de Python:</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="188"/>
        <source>Python version:</source>
        <translation>Versión de Python:</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="189"/>
        <source>Python path:</source>
        <translation>Ruta de Python:</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="179"/>
        <source>An error occured during execution of following code:</source>
        <translation>Ocurrió un error durante la ejecución del siguiente código:</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="400"/>
        <source>Uncatched fatal GRASS error</source>
        <translation>Error fatal de GRASS no detectado</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="569"/>
        <source>ERROR: Failed to created default style file as %1 Check file permissions and retry.</source>
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
        <translation type="unfinished">Abortar</translation>
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
        <translation type="unfinished">Versión 0.001</translation>
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
</context>
<context>
    <name>QgisApp</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="345"/>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS - </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="312"/>
        <source>Checking database</source>
        <translation>Comprobando la base de datos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="318"/>
        <source>Reading settings</source>
        <translation>Leyendo configuraciones</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="322"/>
        <source>Setting up the GUI</source>
        <translation>Configurando la interfaz gráfica de usuario (GUI)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="405"/>
        <source>Restoring loaded plugins</source>
        <translation>Restableciendo complementos cargados</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="409"/>
        <source>Initializing file filters</source>
        <translation>Inicializando filtros de archivo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="434"/>
        <source>Restoring window state</source>
        <translation>Restableciendo el estado de la ventana</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="438"/>
        <source>QGIS Ready!</source>
        <translation>¡QGIS preparado!</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="548"/>
        <source>&amp;New Project</source>
        <translation>&amp;Nuevo proyecto</translation>
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
        <translation>Nuevo proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="553"/>
        <source>&amp;Open Project...</source>
        <translation>&amp;Abrir proyecto...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="554"/>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation>Ctrl+A</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="555"/>
        <source>Open a Project</source>
        <translation>Abrir un proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="558"/>
        <source>&amp;Save Project</source>
        <translation>&amp;Guardar proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="559"/>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation>Ctrl+G</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="560"/>
        <source>Save Project</source>
        <translation>Guardar proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="563"/>
        <source>Save Project &amp;As...</source>
        <translation>G&amp;uardar proyecto como...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="564"/>
        <source>Ctrl+A</source>
        <comment>Save Project under a new name</comment>
        <translation>Ctrl+U</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="565"/>
        <source>Save Project under a new name</source>
        <translation>Guardar proyecto con un nombre nuevo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="568"/>
        <source>&amp;Print...</source>
        <translation>Im&amp;primir...</translation>
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
        <translation>Imprimir</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="573"/>
        <source>Save as Image...</source>
        <translation>Guardar como &amp;imagen...</translation>
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
        <translation>Guardar mapa como imagen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="586"/>
        <source>Exit</source>
        <translation>Salir</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="587"/>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="588"/>
        <source>Exit QGIS</source>
        <translation>Salir de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="593"/>
        <source>Add a Vector Layer...</source>
        <translation>Añadir una capa vectorial...</translation>
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
        <translation>Añadir una capa vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="598"/>
        <source>Add a Raster Layer...</source>
        <translation>Añadir una capa ráster...</translation>
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
        <translation>Añadir una capa ráster</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="603"/>
        <source>Add a PostGIS Layer...</source>
        <translation>Añadir una capa de PostGIS...</translation>
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
        <translation>Añadir una capa de PostGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="615"/>
        <source>New Vector Layer...</source>
        <translation>Nueva capa vectorial...</translation>
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
        <translation>Crear una capa vectorial nueva</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="620"/>
        <source>Remove Layer</source>
        <translation>Eliminar capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="621"/>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation>Ctrl+E</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="622"/>
        <source>Remove a Layer</source>
        <translation>Eliminar una capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="626"/>
        <source>Add All To Overview</source>
        <translation>Añadir todo al localizador</translation>
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
        <translation>Mostrar todas las capas en el localizador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="631"/>
        <source>Remove All From Overview</source>
        <translation>Eliminar todo del localizador</translation>
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
        <translation>Eliminar todas las capas del localizador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="642"/>
        <source>Show All Layers</source>
        <translation>Mostrar todas las capas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="643"/>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation>M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="644"/>
        <source>Show all layers</source>
        <translation>Mostrar todas las capas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="647"/>
        <source>Hide All Layers</source>
        <translation>Ocultar todas las capas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="648"/>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="649"/>
        <source>Hide all layers</source>
        <translation>Ocultar todas las capas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="654"/>
        <source>Project Properties...</source>
        <translation>Propiedades del proyecto...</translation>
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
        <translation>Definir las propiedades del proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="659"/>
        <source>Options...</source>
        <translation>Opciones...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="661"/>
        <source>Change various QGIS options</source>
        <translation>Cambiar varias opciones de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="664"/>
        <source>Custom Projection...</source>
        <translation>Proyección personalizada...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="666"/>
        <source>Manage custom projections</source>
        <translation>Administrar proyecciones personalizadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="671"/>
        <source>Help Contents</source>
        <translation>Contenidos de la ayuda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="673"/>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation>Ctrl+?</translation>
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
        <translation>Documentación de ayuda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="680"/>
        <source>Qgis Home Page</source>
        <translation>Página web de Qgis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="682"/>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation>Ctrl+W</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="684"/>
        <source>QGIS Home Page</source>
        <translation>Página web de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="687"/>
        <source>About</source>
        <translation>Acerca de</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="688"/>
        <source>About QGIS</source>
        <translation>Acerca de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="691"/>
        <source>Check Qgis Version</source>
        <translation>Comprobar versión de Qgis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="692"/>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Comprobar si su versión de QGIS está actualizada (requiere acceso a internet)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="697"/>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="698"/>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation>Ctrl+A</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="699"/>
        <source>Refresh Map</source>
        <translation>Actualizar mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="704"/>
        <source>Zoom In</source>
        <translation>Acercar zum</translation>
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
        <translation>Alejar zum</translation>
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
        <translation>Zum general</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="713"/>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation>G</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="714"/>
        <source>Zoom to Full Extents</source>
        <translation>Zum a toda la extensión</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="717"/>
        <source>Zoom To Selection</source>
        <translation>Zum a la selección</translation>
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
        <translation>Zum a la selección</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="722"/>
        <source>Pan Map</source>
        <translation>Desplazar mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="723"/>
        <source>Pan the map</source>
        <translation>Desplazar el mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="726"/>
        <source>Zoom Last</source>
        <translation>Zum anterior</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="728"/>
        <source>Zoom to Last Extent</source>
        <translation>Zum a la extensión anterior</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="731"/>
        <source>Zoom To Layer</source>
        <translation>Zum a la capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="733"/>
        <source>Zoom to Layer</source>
        <translation>Zum a la capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="736"/>
        <source>Identify Features</source>
        <translation>Identificar objetos espaciales</translation>
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
        <translation>Pulsar sobre los objetos espaciales para identificarlos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="742"/>
        <source>Select Features</source>
        <translation>Seleccionar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="748"/>
        <source>Open Table</source>
        <translation>Abrir tabla</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="752"/>
        <source>Measure Line </source>
        <translation>Regla </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="753"/>
        <source>Ctrl+M</source>
        <comment>Measure a Line</comment>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="754"/>
        <source>Measure a Line</source>
        <translation>Regla</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="757"/>
        <source>Measure Area</source>
        <translation>Medir áreas</translation>
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
        <translation>Medir un área</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="764"/>
        <source>Show Bookmarks</source>
        <translation>Mostrar marcadores</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="763"/>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation>M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="769"/>
        <source>Show most toolbars</source>
        <translation>Mostrar todas las barras de herramientas posibles</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="775"/>
        <source>Hide most toolbars</source>
        <translation>Ocultar todas las barras de herramientas posibles</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="779"/>
        <source>New Bookmark...</source>
        <translation>Nuevo marcador...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="780"/>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation>Ctrl+M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5440"/>
        <source>New Bookmark</source>
        <translation>Nuevo marcador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="784"/>
        <source>Add WMS Layer...</source>
        <translation>Añadir capa WMS...</translation>
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
        <translation>Añadir capa de servidor web de mapas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="789"/>
        <source>In Overview</source>
        <translation>Llevar al localizador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="790"/>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation>L</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="791"/>
        <source>Add current layer to overview map</source>
        <translation>Añadir la capa actual al localizador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="797"/>
        <source>Plugin Manager...</source>
        <translation>Administrador de complementos...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="799"/>
        <source>Open the plugin manager</source>
        <translation>Abrir el administrador de complementos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="817"/>
        <source>Capture Point</source>
        <translation>Capturar punto</translation>
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
        <translation>Capturar puntos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="823"/>
        <source>Capture Line</source>
        <translation>Capturar línea</translation>
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
        <translation>Capturar líneas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="829"/>
        <source>Capture Polygon</source>
        <translation>Capturar polígono</translation>
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
        <translation>Capturar polígonos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="836"/>
        <source>Delete Selected</source>
        <translation>Borrar lo seleccionado</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="851"/>
        <source>Add Vertex</source>
        <translation>Añadir vértice</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="856"/>
        <source>Delete Vertex</source>
        <translation>Borrar vértice</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="861"/>
        <source>Move Vertex</source>
        <translation>Mover vértice</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="875"/>
        <source>Cut Features</source>
        <translation>Cortar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="876"/>
        <source>Cut selected features</source>
        <translation>Cortar los objetos espaciales seleccionados</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="880"/>
        <source>Copy Features</source>
        <translation>Copiar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="881"/>
        <source>Copy selected features</source>
        <translation>Copiar los objetos espaciales seleccionados</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="885"/>
        <source>Paste Features</source>
        <translation>Pegar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="886"/>
        <source>Paste selected features</source>
        <translation>Pegar los objetos espaciales seleccionados</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="954"/>
        <source>&amp;File</source>
        <translation>&amp;Archivo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="957"/>
        <source>&amp;Open Recent Projects</source>
        <translation>Abrir proyectos &amp;recientes</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="977"/>
        <source>&amp;View</source>
        <translation>&amp;Ver</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="996"/>
        <source>&amp;Layer</source>
        <translation>&amp;Capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1016"/>
        <source>&amp;Settings</source>
        <translation>C&amp;onfiguración</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1023"/>
        <source>&amp;Plugins</source>
        <translation>Co&amp;mplementos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1036"/>
        <source>&amp;Help</source>
        <translation>A&amp;yuda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1055"/>
        <source>File</source>
        <translation>Archivo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1072"/>
        <source>Manage Layers</source>
        <translation>Administrar capas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1133"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1082"/>
        <source>Digitizing</source>
        <translation>Digitalización</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1102"/>
        <source>Map Navigation</source>
        <translation>Navegación de mapas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1115"/>
        <source>Attributes</source>
        <translation>Atributos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1128"/>
        <source>Plugins</source>
        <translation>Complementos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1189"/>
        <source>Displays the current map scale</source>
        <translation>Muestra la escala del mapa actual</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1215"/>
        <source>Render</source>
        <translation>Representar</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1221"/>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Cuando está marcada, las capas se representan respondiendo a los comandos de navegación y otros eventos. Cuando no está marcada, la representación no se hace. Esto permite añadir un gran número de capas y simbolizarlas antes de su representación.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1222"/>
        <source>Toggle map rendering</source>
        <translation>Conmutar la representación del mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1245"/>
        <source>This icon shows whether on the fly projection is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation>Este icono muestra si la proyección &quot;al vuelo&quot; está activada o no. Pulse el icono para mostrar el cuadro de diálogo de propiedades del proyecto para variar esta característica.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1247"/>
        <source>Projection status - Click to open projection dialog</source>
        <translation>Estado de la proyección - Pulse para abrir el cuadro de diálogo de proyección</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1251"/>
        <source>Ready</source>
        <translation>Preparado</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1425"/>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Localizador del mapa. Esta vista puede usarse para visualizar un mapa de localización que muestra la extensión de la vista del mapa. La extensión actual se muestra como un rectángulo rojo. Cualquier capa del mapa se puede añadir al localizador.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1464"/>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>Leyenda del mapa que muestra todas la capas actualmente en la vista del mapa. Marcar la casilla para conmutar su vista. Pulsar dos veces sobre una capa en la leyenda para personalizar su apariencia y ajustar otras propiedades.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1625"/>
        <source>Version</source>
        <translation>Versión</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1627"/>
        <source>New features</source>
        <translation>Nuevos objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="768"/>
        <source>T</source>
        <comment>Show most toolbars</comment>
        <translation>T</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="358"/>
        <source>Checking provider plugins</source>
        <translation>Comprobando complementos del proveedor</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="362"/>
        <source>Starting Python</source>
        <translation>Iniciando Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="396"/>
        <source>Python console</source>
        <translation>Consola de Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1762"/>
        <source>Python error</source>
        <translation>Error de Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1762"/>
        <source>Error when reading metadata of plugin </source>
        <translation>Error al leer metadatos del complemento </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="810"/>
        <source>Toggle editing</source>
        <translation>Conmutar edición</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="811"/>
        <source>Toggles the editing state of the current layer</source>
        <translation>Conmuta el estado de edición de la capa activa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="866"/>
        <source>Add Ring</source>
        <translation>Añadir anillo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="870"/>
        <source>Add Island</source>
        <translation>Añadir isla</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="871"/>
        <source>Add Island to multipolygon</source>
        <translation>Añadir isla a multipolígono</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1145"/>
        <source>Toolbar Visibility...</source>
        <translation>Visibilidad de barras de herramientas...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1175"/>
        <source>Scale </source>
        <translation>Escala </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1190"/>
        <source>Current map scale (formatted as x:y)</source>
        <translation>Escala actual del mapa (en formato X:Y)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1204"/>
        <source>Map coordinates at mouse cursor position</source>
        <translation>Coordenadas del mapa en la posición del ratón</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="774"/>
        <source>Ctrl+T</source>
        <comment>Hide most toolbars</comment>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2027"/>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>Abrir una capa vectorial soportada por OGR</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2081"/>
        <source>is not a valid or recognized data source</source>
        <translation>no es una fuente de datos válida o reconocida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5184"/>
        <source>Invalid Data Source</source>
        <translation>Fuente de datos no válida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2173"/>
        <source>Invalid Layer</source>
        <translation>Capa no válida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2173"/>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>%1 es una capa no válida y no se puede cargar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2649"/>
        <source>Save As</source>
        <translation>Guardar como</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2749"/>
        <source>Choose a QGIS project file to open</source>
        <translation>Seleccionar un archivo de proyecto de QGIS para abrir</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2864"/>
        <source>QGIS Project Read Error</source>
        <translation>Error de lectura del proyecto de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2866"/>
        <source>Try to find missing layers?</source>
        <translation>¿Buscar las capas perdidas?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2882"/>
        <source>Unable to open project</source>
        <translation>No se puede abrir el proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2921"/>
        <source>Choose a QGIS project file</source>
        <translation>Seleccionar un archivo de proyecto de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3038"/>
        <source>Saved project to:</source>
        <translation>Proyecto guardado en:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3045"/>
        <source>Unable to save project</source>
        <translation>No se puede guardar el proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3046"/>
        <source>Unable to save project to </source>
        <translation>No se puede guardar el proyecto en </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3052"/>
        <source>Unable to save project </source>
        <translation>No se puede guardar el proyecto </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2994"/>
        <source>Choose a filename to save the QGIS project file as</source>
        <translation>Seleccionar un nombre de archivo para guardar como proyecto de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3110"/>
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: No se puede cargar el proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3111"/>
        <source>Unable to load project </source>
        <translation>No se puede cargar el proyecto </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3234"/>
        <source>Choose a filename to save the map image as</source>
        <translation>Seleccionar un nombre de archivo para guardar el mapa como imagen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3276"/>
        <source>Saved map image to</source>
        <translation>Imagen del mapa guardada en</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3487"/>
        <source>No Layer Selected</source>
        <translation>Ninguna capa seleccionada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3488"/>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation>Para borrar objetos espaciales, debe seleccionar una capa vectorial en la leyenda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3495"/>
        <source>No Vector Layer Selected</source>
        <translation>Ninguna capa vectorial seleccionada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3496"/>
        <source>Deleting features only works on vector layers</source>
        <translation>Borrar objetos espaciales solo funciona en capas vectoriales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3502"/>
        <source>Provider does not support deletion</source>
        <translation>El proveedor no soporta el borrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3503"/>
        <source>Data provider does not support deleting features</source>
        <translation>El proveedor de datos no soporta el borrado de objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3509"/>
        <source>Layer not editable</source>
        <translation>Capa no editable</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3510"/>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation>La capa activa no se puede editar. Elegir &apos;Conmutar edición&apos; en la barra de herramientas Digitalización.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3517"/>
        <source>Problem deleting features</source>
        <translation>Problema al borrar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3518"/>
        <source>A problem occured during deletion of features</source>
        <translation>Ha ocurrido un problema durante el borrado de objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3780"/>
        <source>Invalid scale</source>
        <translation>Escala no válida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4012"/>
        <source>Error Loading Plugin</source>
        <translation>Error al cargar el complemento</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4012"/>
        <source>There was an error loading %1.</source>
        <translation>Ha habido un error al cargar %1.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4048"/>
        <source>No MapLayer Plugins</source>
        <translation>No hay complementos de MapLayer</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4048"/>
        <source>No MapLayer plugins in ../plugins/maplayer</source>
        <translation>No hay complementos de MapLayer en ../plugins/maplayer</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4125"/>
        <source>No Plugins</source>
        <translation>No hay complementos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4126"/>
        <source>No plugins found in ../plugins. To test plugins, start qgis from the src directory</source>
        <translation>No se han encontrado complementos en ../plugins. Para probar complementos, inicie qgis desde el directorio src</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4160"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4160"/>
        <source>Plugin %1 is named %2</source>
        <translation>El complemento %1 se llama %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4177"/>
        <source>Plugin Information</source>
        <translation>Información del complemento</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>QGis loaded the following plugin:</source>
        <translation>QGis ha cargado el siguiente complemento:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>Name: %1</source>
        <translation>Nombre: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>Version: %1</source>
        <translation>Versión: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4179"/>
        <source>Description: %1</source>
        <translation>Descripción: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4197"/>
        <source>Unable to Load Plugin</source>
        <translation>No se puede cargar el complemento</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4198"/>
        <source>QGIS was unable to load the plugin from: %1</source>
        <translation>QGIS no ha podido cargar el complemento desde: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4254"/>
        <source>There is a new version of QGIS available</source>
        <translation>Hay una nueva versión de QGIS disponible</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4260"/>
        <source>You are running a development version of QGIS</source>
        <translation>Está utilizando una versión de desarrollo de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4264"/>
        <source>You are running the current version of QGIS</source>
        <translation>Está utilizando la versión actualizada de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4269"/>
        <source>Would you like more information?</source>
        <translation>¿Desea más información?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4316"/>
        <source>QGIS Version Information</source>
        <translation>Información de la versión de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4276"/>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS - Cambios en el SVN desde la última versión</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4288"/>
        <source>Unable to get current version information from server</source>
        <translation>No se puede obtener información de la versión actual del servidor</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4302"/>
        <source>Connection refused - server may be down</source>
        <translation>Conexión rehusada - el servidor puede estar fuera de servicio</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4305"/>
        <source>QGIS server was not found</source>
        <translation>No se ha encontrado el servidor QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4308"/>
        <source>Network error while communicating with server</source>
        <translation>Error de red mientras se comunicaba con el servidor</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4311"/>
        <source>Unknown network socket error</source>
        <translation>Error de socket de red desconocido</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4316"/>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>No se puede comunicar con el servidor de versión de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5264"/>
        <source>Layer is not valid</source>
        <translation>La capa no es válida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5265"/>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>La capa no es válida y no se puede añadir al mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4533"/>
        <source>Save?</source>
        <translation>¿Guardar?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4534"/>
        <source>Do you want to save the current project?</source>
        <translation>¿Quiere guardar el proyecto actual?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4684"/>
        <source>Extents: </source>
        <translation>Extensiones: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5047"/>
        <source>Clipboard contents set to: </source>
        <translation>Contenido del portapapeles ajustado a: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5092"/>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>Abrir una fuente de datos ráster soportada por GDAL</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5183"/>
        <source> is not a valid or recognized raster data source</source>
        <translation> no es una fuente de datos ráster válida o reconocida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5344"/>
        <source> is not a supported raster data source</source>
        <translation> no es una fuente de datos ráster soportada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5345"/>
        <source>Unsupported Data Source</source>
        <translation>Fuente de datos no soportada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5441"/>
        <source>Enter a name for the new bookmark:</source>
        <translation>Introducir un nombre para el nuevo marcador:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5458"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5458"/>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>No se puede crear el marcador. Puede que falte su base de datos de usuario o esté dañada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="841"/>
        <source>Move Feature</source>
        <translation>Mover objeto espacial</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="846"/>
        <source>Split Features</source>
        <translation>Dividir objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="891"/>
        <source>Map Tips</source>
        <translation>Avisos del mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="892"/>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation>Mostrar información sobre un objeto espacial cuando se desliza el ratón sobre él</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1176"/>
        <source>Current map scale</source>
        <translation>Escala actual del mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5493"/>
        <source>Project file is older</source>
        <translation>El archivo del proyecto es más antiguo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5495"/>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation>&lt;p&gt;Este archivo de proyecto se guardó con una versión más antigua de QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5497"/>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation> Cuando guarde este proyecto, QGIS lo actualizará a la última versión, dejándolo probablemente inservible para versiones anteriores de QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5500"/>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation>&lt;P&gt;Incluso aunque los desarrolladores de QGIS tratan de mantener retrocompatibilidad, parte de la información del antiguo proyecto puede perderse.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5502"/>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation> Para mejorar la calidad de QGIS, agradecemos si rellena un informe de error en %3.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5504"/>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation> Asegúrese de incluir el archivo del antiguo proyecto e indicar la versión de QGIS que usó para descubrir el error.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5506"/>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation>&lt;p&gt;Para eliminar este aviso al abrir un proyecto antiguo, desmarque la casilla &apos;%5&apos; en el menú %4.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5506"/>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation>&lt;p&gt;Versión del archivo de proyecto: %1 &lt;br&gt;Versión actual de QGIS: %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5510"/>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation>&lt;tt&gt;Configuración:Opciones:General&lt;/tt&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5511"/>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Avisarme al abrir archivos de proyecto guardados con una versión anterior de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="636"/>
        <source>Toggle full screen mode</source>
        <translation>Cambiar el modo de pantalla completa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="637"/>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation>C</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="638"/>
        <source>Toggle fullscreen mode</source>
        <translation>Cambiar el modo de pantalla completa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1633"/>
        <source>Imrovements to digitising capabilities.</source>
        <translation>Mejora de la capacidad de digitalización.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1639"/>
        <source>Supporting default and defined styles (.qml) files for file based vector layers. With styles you can save the symbolisation and other settings associated with a vector layer and they will be loaded whenever you load that layer.</source>
        <translation>Soporte para archivos (.qml) de estilos predeterminados y definidos, para capas vectoriales basadas en archivos. Con los estilos puede guardar la simbología y otros ajustes asociados a una capa vectorial y se cargarán cada vez que cargue la capa.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1645"/>
        <source>Improved support for transparency and contrast stretching in raster layers. Support for color ramps in raster layers. Support for non-north up rasters. Many other raster improvements &apos;under the hood&apos;.</source>
        <translation>Soporte mejorado para ajuste de la transparencia y el contraste en capas ráster. Soporte para rampas de color en capas ráster. Soporte para rásters no orientados al norte. Otras muchas mejoras ráster «en la trastienda».</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1630"/>
        <source>This release candidate includes over 120 bug fixes and enchancements over the QGIS 0.9.1 release. In addition we have added the following new features:</source>
        <translation>Este candidato de lanzamiento incluye más de 120 correcciones de errores y mejoras en relación a la versión 0.9.1. Además hemos añadido las siguientes funciones nuevas:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1648"/>
        <source>Updated icons for improved visual consistancy.</source>
        <translation>Iconos actualizados para una consistencia visual mejorada.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1651"/>
        <source>Support for migration of old projects to work in newer QGIS versions.</source>
        <translation>Soporte para la migración de proyectos antiguos, para que funcionen en versiones más recientes de QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1239"/>
        <source>Resource Location Error</source>
        <translation>Error en la localización de recursos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1240"/>
        <source>Error reading icon resources from: 
 %1
 Quitting...</source>
        <translation>Error al leer recursos de iconos de: 
 %1
 Saliendo...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1375"/>
        <source>Map canvas. This is where raster and vectorlayers are displayed when added to the map</source>
        <translation type="unfinished">Vista del mapa. Aquí es do</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1434"/>
        <source>Overview</source>
        <translation>Localizador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1468"/>
        <source>Legend</source>
        <translation>Leyenda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1605"/>
        <source>You are using QGIS version %1 built against code revision %2.</source>
        <translation>Está usando la versión %1 de QGIS compilada contra la revisión %2 del código.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1609"/>
        <source> This copy of QGIS has been built with PostgreSQL support.</source>
        <translation> Esta copia de QGIS se ha compilado con soporte para PostgreSQL.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1612"/>
        <source> This copy of QGIS has been built without PostgreSQL support.</source>
        <translation> Esta copia de QGIS se ha compilado sin soporte para PostgreSQL.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1616"/>
        <source>
This binary was compiled against Qt %1,and is currently running against Qt %2</source>
        <translation>
Este binario se compiló contra Qt %1 y se está ejecutando contra Qt %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1160"/>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1203"/>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation>Ventana Principal</translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="102"/>
        <source>Legend</source>
        <translation>Leyenda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="135"/>
        <source>Map View</source>
        <translation>Vista del mapa</translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="13"/>
        <source>About Quantum GIS</source>
        <translation>Acerca de Quantum GIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="26"/>
        <source>About</source>
        <translation>Acerca de</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="65"/>
        <source>Version</source>
        <translation>Versión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="129"/>
        <source>QGIS Home Page</source>
        <translation>Página web de QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="152"/>
        <source>What&apos;s New</source>
        <translation>Novedades</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="175"/>
        <source>Developers</source>
        <translation>Desarrolladores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="189"/>
        <source>Providers</source>
        <translation>Proveedores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="199"/>
        <source>Sponsors</source>
        <translation>Patrocinadores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="230"/>
        <source>Ok</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="50"/>
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
        <location filename="../src/ui/qgsabout.ui" line="91"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation>Quantum GIS se distribuye bajo la Licencia Pública General GNU</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="104"/>
        <source>http://www.gnu.org/licenses</source>
        <translation>http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="139"/>
        <source>Join our user mailing list</source>
        <translation>Únase a nuestra lista de correo de usuarios</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="112"/>
        <source>&lt;p&gt;The following have sponsored QGIS by contributing money to fund development and other project costs&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="114"/>
        <source>Name</source>
        <translation type="unfinished">Nombre</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="115"/>
        <source>Website</source>
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
</context>
<context>
    <name>QgsAddAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="13"/>
        <source>Add Attribute</source>
        <translation>Añadir atributo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="35"/>
        <source>Type:</source>
        <translation>Tipo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="22"/>
        <source>Name:</source>
        <translation>Nombre:</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="150"/>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation>Seleccionar una acción</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="247"/>
        <source>Remove the selected action</source>
        <translation>Eliminar la acción seleccionada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="250"/>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="221"/>
        <source>Move the selected action down</source>
        <translation>Mover la acción seleccionada hacia abajo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="224"/>
        <source>Move down</source>
        <translation>Bajar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="211"/>
        <source>Move the selected action up</source>
        <translation>Mover la acción seleccionada hacia arriba</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="214"/>
        <source>Move up</source>
        <translation>Subir</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="144"/>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>Esta lista contiene todas las acciones que han sido definidas para la capa actual. Añada acciones introduciendo los detalles en los controles inferiores y presionando el botón Insertar acción. Las acciones se pueden editar pulsando dos veces sobre ellas.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="105"/>
        <source>The valid attribute names for this layer</source>
        <translation>Los nombres de atributo válidos para esta capa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="112"/>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation>Inserta el campo seleccionado en la acción, precedido con un %</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="115"/>
        <source>Insert field</source>
        <translation>Insertar campo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="185"/>
        <source>Update the selected action</source>
        <translation>Actualizar la acción seleccionada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="188"/>
        <source>Update action</source>
        <translation>Actualizar acción</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="175"/>
        <source>Inserts the action into the list above</source>
        <translation>Inserta la acción en la lista de arriba</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="178"/>
        <source>Insert action</source>
        <translation>Insertar acción</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="122"/>
        <source>Captures any output from the action</source>
        <translation>Captura cualquier salida de la acción</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="125"/>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation>Captura la salida estándar o el error generado por la acción y la muestra en un cuadro de diálogo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="128"/>
        <source>Capture output</source>
        <translation>Capturar salida</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="79"/>
        <source>Enter the action command here</source>
        <translation>Introduzca aquí el comando de acción</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="56"/>
        <source>Enter the action name here</source>
        <translation>Introduzca el nombre de la acción aquí</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="59"/>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>Introduzca el nombre de la acción aquí. El nombre debe ser único (QGIS lo hará único si es necesario).</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="82"/>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation>Introduzca la acción aquí. Ésta puede ser cualquier programa, script o comando que esté disponible en su sistema. Cuando se invoca una acción cualquier conjunto de caracteres que empiece por % y luego tenga el nombre de un campo se reemplazará con el valor de ese campo. Los caracteres especiales %% se reemplazarán por el valor del campo que se seleccionó. Las comillas dobles agrupan texto en argumentos simples para el programa, script o comando. Las comillas dobles se ignorarán si se preceden por una barra invertida (\)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="157"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="162"/>
        <source>Action</source>
        <translation>Acción</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="167"/>
        <source>Capture</source>
        <translation>Capturar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="19"/>
        <source>Attribute Actions</source>
        <translation>Acciones de atributos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="37"/>
        <source>Action properties</source>
        <translation>Propiedades de acciones</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="89"/>
        <source>Browse for action</source>
        <translation>Explorar acciones</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="92"/>
        <source>Click to browse for an action</source>
        <translation>Pulse para explorar en busca de una acción</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="95"/>
        <source>Clicking the buttone will let you select an application to use as the action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="98"/>
        <source>...</source>
        <translation>...</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialog</name>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="76"/>
        <source> (int)</source>
        <translation> (int)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="81"/>
        <source> (dbl)</source>
        <translation> (dbl)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="86"/>
        <source> (txt)</source>
        <translation> (txt)</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="13"/>
        <source>Enter Attribute Values</source>
        <translation>Introducir valores de los atributos</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="340"/>
        <source>Run action</source>
        <translation>Ejecutar acción</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="13"/>
        <source>Attribute Table</source>
        <translation>Tabla de atributos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="211"/>
        <source>in</source>
        <translation>en</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="231"/>
        <source>Search</source>
        <translation>Búsqueda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="241"/>
        <source>Adva&amp;nced...</source>
        <translation>Ava&amp;nzado...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="244"/>
        <source>Alt+N</source>
        <translation>Alt+N</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="30"/>
        <source>Remove selection</source>
        <translation>Eliminar selección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="46"/>
        <source>Move selected to top</source>
        <translation>Mover la selección arriba del todo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="55"/>
        <source>Ctrl+T</source>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="62"/>
        <source>Invert selection</source>
        <translation>Invertir selección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="71"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="78"/>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Copiar las filas seleccionadas al portapapeles (Ctrl+C)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="81"/>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Copia las filas seleccionadas al portapapeles</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="90"/>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="119"/>
        <source>New column</source>
        <translation>Nueva columna</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="128"/>
        <source>Ctrl+N</source>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="135"/>
        <source>Delete column</source>
        <translation>Eliminar columna</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="144"/>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="167"/>
        <source>Start editing</source>
        <translation>Comenzar edición</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="174"/>
        <source>Stop editin&amp;g</source>
        <translation>Terminar &amp;edición</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="177"/>
        <source>Alt+G</source>
        <translation>Alt+E</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="97"/>
        <source>Zoom map to the selected rows (Ctrl-F)</source>
        <translation>Zum a las filas seleccionadas (Ctrl+F)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="100"/>
        <source>Zoom map to the selected rows</source>
        <translation>Zum a las filas seleccionadas </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="106"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="198"/>
        <source>Search for</source>
        <translation>Buscar</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="98"/>
        <source>select</source>
        <translation>seleccionar</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="99"/>
        <source>select and bring to top</source>
        <translation>seleccionar y llevar arriba</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="100"/>
        <source>show only matching</source>
        <translation>mostrar solo coincidentes</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="330"/>
        <source>Search string parsing error</source>
        <translation>Buscar error de análisis de la cadena</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="382"/>
        <source>Search results</source>
        <translation>Resultados de la búsqueda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="336"/>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>Ha suministrado una cadena de búsqueda vacía.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="370"/>
        <source>Error during search</source>
        <translation>Error durante la búsqueda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="381"/>
        <source>No matching features found.</source>
        <translation>No se han encontrado coincidencias.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="148"/>
        <source>Name conflict</source>
        <translation>Conflicto de nombre</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="197"/>
        <source>Stop editing</source>
        <translation>Terminar edición</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="198"/>
        <source>Do you want to save the changes?</source>
        <translation>¿Quiere guardar los cambios?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="204"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="148"/>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation>No se pudo insertar el atributo. El nombre ya existe en la tabla.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="204"/>
        <source>Could not commit changes - changes are still pending</source>
        <translation>No se pudieron enviar los cambios - los cambios están todavía pendientes</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="379"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="160"/>
        <source>Really Delete?</source>
        <translation>¿Borrar realmente?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="161"/>
        <source>Are you sure you want to delete the </source>
        <translation>¿Está seguro de que quiere borrar el marcador </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="161"/>
        <source> bookmark?</source>
        <translation>?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="177"/>
        <source>Error deleting bookmark</source>
        <translation>Error al borrar marcador</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="179"/>
        <source>Failed to delete the </source>
        <translation>Ha fallado el borrado del marcador </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="181"/>
        <source> bookmark from the database. The database said:
</source>
        <translation> de la base de datos. La base de datos ha dicho:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="60"/>
        <source>&amp;Delete</source>
        <translation>&amp;Borrar</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="61"/>
        <source>&amp;Zoom to</source>
        <translation>&amp;Zum a</translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="13"/>
        <source>Geospatial Bookmarks</source>
        <translation>Marcadores geoespaciales</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="29"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="34"/>
        <source>Project</source>
        <translation>Proyecto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="39"/>
        <source>Extent</source>
        <translation>Extensión</translation>
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
        <location filename="../src/app/composer/qgscomposer.cpp" line="56"/>
        <source>QGIS - print composer</source>
        <translation>QGIS - diseñador de mapas</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="81"/>
        <source>Map 1</source>
        <translation>Mapa 1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="472"/>
        <source>Couldn&apos;t open </source>
        <translation>No se puede abrir </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="472"/>
        <source> for read/write</source>
        <translation> para leer/escribir</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="551"/>
        <source>Error in Print</source>
        <translation>Error al imprimir</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="529"/>
        <source>Cannot seek</source>
        <translation>No se puede solicitar</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="454"/>
        <source>Cannot overwrite BoundingBox</source>
        <translation>No se puede sobrescribir el marco</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="462"/>
        <source>Cannot find BoundingBox</source>
        <translation>No se puede encontrar el marco</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="545"/>
        <source>Cannot overwrite translate</source>
        <translation>No se puede sobrescribir la traducción</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="551"/>
        <source>Cannot find translate</source>
        <translation>No se puede encontrar la traducción</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="559"/>
        <source>File IO Error</source>
        <translation>Error de ES del archivo</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="569"/>
        <source>Paper does not match</source>
        <translation>El papel no coincide</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="570"/>
        <source>The selected paper size does not match the composition size</source>
        <translation>El tamaño de papel seleccionado no se ajusta al tamaño del mapa</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="634"/>
        <source>Big image</source>
        <translation>Imagen grande</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="635"/>
        <source>To create image </source>
        <translation>Crear imagen </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="638"/>
        <source> requires circa </source>
        <translation> requiere cerca de </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="638"/>
        <source> MB of memory</source>
        <translation> MB de memoria</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="665"/>
        <source>format</source>
        <translation>formato</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="688"/>
        <source>Choose a filename to save the map image as</source>
        <translation>Seleccione un nombre de archivo para guardar el mapa como imagen</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="757"/>
        <source>SVG warning</source>
        <translation>Aviso de SVG</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="758"/>
        <source>Don&apos;t show this message again</source>
        <translation>No volver a mostrar este mensaje</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="783"/>
        <source>Choose a filename to save the map as</source>
        <translation>Seleccionar un nombre de archivo para guardar el mapa</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="784"/>
        <source>SVG Format</source>
        <translation>Formato SVG</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="764"/>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation>Ventana principal</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="62"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="107"/>
        <source>Composition</source>
        <translation>Diseño</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="115"/>
        <source>Item</source>
        <translation>Elemento</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="170"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="150"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="208"/>
        <source>&amp;Open Template ...</source>
        <translation>&amp;Abrir plantilla...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="216"/>
        <source>Save Template &amp;As...</source>
        <translation>Guardar plantilla &amp;como...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="224"/>
        <source>&amp;Print...</source>
        <translation>Im&amp;primir...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="232"/>
        <source>Zoom All</source>
        <translation>Zum general</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="240"/>
        <source>Zoom In</source>
        <translation>Acercar zum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="248"/>
        <source>Zoom Out</source>
        <translation>Alejar zum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="256"/>
        <source>Add new map</source>
        <translation>Añadir mapa nuevo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="264"/>
        <source>Add new label</source>
        <translation>Añadir etiqueta nueva</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="272"/>
        <source>Add new vect legend</source>
        <translation>Añadir nueva leyenda vectorizada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="280"/>
        <source>Select/Move item</source>
        <translation>Seleccionar/Mover elemento</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="288"/>
        <source>Export as image</source>
        <translation>Exportar como imagen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="296"/>
        <source>Export as SVG</source>
        <translation>Exportar como SVG</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="304"/>
        <source>Add new scalebar</source>
        <translation>Añadir nueva barra de escala</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="312"/>
        <source>Refresh view</source>
        <translation>Actualizar vista</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="320"/>
        <source>Add Image</source>
        <translation>Añadir imagen</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="21"/>
        <source>Label Options</source>
        <translation>Opciones de etiqueta</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="55"/>
        <source>Box</source>
        <translation>Cajetín</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="48"/>
        <source>Font</source>
        <translation>Fuente</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="77"/>
        <source>Map %1</source>
        <translation> Mapa %1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="99"/>
        <source>Extent (calculate scale)</source>
        <translation>Extensión (calcular escala)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="100"/>
        <source>Scale (calculate extent)</source>
        <translation>Escala (calcular extensión)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="107"/>
        <source>Cache</source>
        <translation>Caché</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="108"/>
        <source>Render</source>
        <translation>Representar</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="109"/>
        <source>Rectangle</source>
        <translation>Rectángulo</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapBase</name>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="21"/>
        <source>Map options</source>
        <translation>Opciones de mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="173"/>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation>&lt;b&gt;Mapa&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="147"/>
        <source>Set</source>
        <translation>Establecer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="196"/>
        <source>Width</source>
        <translation>Anchura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="180"/>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="69"/>
        <source>Set map extent to current extent in QGIS map canvas</source>
        <translation>Establecer la extensión del mapa a la extensión actual en la vista del mapa de QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="72"/>
        <source>Set Extent</source>
        <translation>Establecer extensión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="212"/>
        <source>Line width scale</source>
        <translation>Escala del ancho de línea</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="116"/>
        <source>Width of one unit in millimeters</source>
        <translation>Anchura de una unidad en milímetros</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="225"/>
        <source>Symbol scale</source>
        <translation>Escala del símbolo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="238"/>
        <source>Font size scale</source>
        <translation>Escala del tamaño de fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="251"/>
        <source>Frame</source>
        <translation>Marco</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="258"/>
        <source>Preview</source>
        <translation>Previsualizar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="79"/>
        <source>1:</source>
        <translation>1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="97"/>
        <source>Scale:</source>
        <translation>Escala:</translation>
    </message>
</context>
<context>
    <name>QgsComposerPicture</name>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="399"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="400"/>
        <source>Cannot load picture.</source>
        <translation>No se puede cargar el dibujo.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="466"/>
        <source>Pictures (</source>
        <translation>Dibujos (</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="483"/>
        <source>Choose a file</source>
        <translation>Seleccionar un archivo</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureBase</name>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="21"/>
        <source>Picture Options</source>
        <translation>Opciones de dibujo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="197"/>
        <source>Frame</source>
        <translation>Marco</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="161"/>
        <source>Angle</source>
        <translation>Ángulo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="119"/>
        <source>Width</source>
        <translation>Anchura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="140"/>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="58"/>
        <source>Browse</source>
        <translation>Explorar</translation>
    </message>
</context>
<context>
    <name>QgsComposerScalebarBase</name>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="21"/>
        <source>Barscale Options</source>
        <translation>Opciones de la barra de escala</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="140"/>
        <source>Segment size</source>
        <translation>Tamaño de segmento</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="172"/>
        <source>Number of segments</source>
        <translation>Número de segmentos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="159"/>
        <source>Map units per scalebar unit</source>
        <translation>Unidades de mapa por unidad de barra de escala</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="88"/>
        <source>Unit label</source>
        <translation>Etiqueta de unidad</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="127"/>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="195"/>
        <source>Font</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="41"/>
        <source>Line width</source>
        <translation>Ancho de línea</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegend</name>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="110"/>
        <source>Legend</source>
        <translation>Leyenda</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="718"/>
        <source>Combine selected layers</source>
        <translation>Combinar capas seleccionadas</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="134"/>
        <source>Cache</source>
        <translation>Caché</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="135"/>
        <source>Render</source>
        <translation>Representar</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="136"/>
        <source>Rectangle</source>
        <translation>Rectángulo</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="21"/>
        <source>Vector Legend Options</source>
        <translation>Opciones de leyenda vectorizada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="113"/>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="92"/>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="182"/>
        <source>Font</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="167"/>
        <source>Box</source>
        <translation>Cajetín</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="53"/>
        <source>Preview</source>
        <translation>Previsualización</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="149"/>
        <source>Layers</source>
        <translation>Capas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="154"/>
        <source>Group</source>
        <translation>Grupo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="159"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
</context>
<context>
    <name>QgsComposition</name>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="88"/>
        <source>Custom</source>
        <translation>Personalizado</translation>
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
        <translation>Carta (8.5x11 pulgadas)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="102"/>
        <source>Legal (8.5x14 inches)</source>
        <translation>Legal (8.5x14 pulgadas)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="115"/>
        <source>Portrait</source>
        <translation>Vertical</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="116"/>
        <source>Landscape</source>
        <translation>Horizontal</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="632"/>
        <source>Out of memory</source>
        <translation>Sin memoria</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="635"/>
        <source>Qgis is unable to resize the paper size due to insufficient memory.
 It is best that you avoid using the map composer until you restart qgis.
</source>
        <translation>Qgis no puede cambiar el tamaño del papel debido a una falta de memoria.
 Es mejor que no utilice el diseñador de mapas hasta que reinicie qgis.
</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="779"/>
        <source>Label</source>
        <translation>Etiqueta</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="829"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="830"/>
        <source>Cannot load picture.</source>
        <translation>No se puede cargar el dibujo.</translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="21"/>
        <source>Composition</source>
        <translation>Diseño</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="33"/>
        <source>Paper</source>
        <translation>Papel</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="176"/>
        <source>Size</source>
        <translation>Tamaño</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="158"/>
        <source>Units</source>
        <translation>Unidades</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="140"/>
        <source>Width</source>
        <translation>Anchura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="122"/>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="104"/>
        <source>Orientation</source>
        <translation>Orientación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="213"/>
        <source>Resolution (dpi)</source>
        <translation>Resolución (ppp)</translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="13"/>
        <source>Continuous color</source>
        <translation>Color graduado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="119"/>
        <source>Draw polygon outline</source>
        <translation>Dibujar contorno del polígono</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="109"/>
        <source>Classification Field:</source>
        <translation>Campo de clasificación:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="93"/>
        <source>Minimum Value:</source>
        <translation>Valor mínimo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="80"/>
        <source>Outline Width:</source>
        <translation>Anchura del contorno:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="54"/>
        <source>Maximum Value:</source>
        <translation>Valor máximo:</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="418"/>
        <source>The source spatial reference system (SRS) is not valid. </source>
        <translation>El sistema espacial de referencia (SRS) de origen no es válido. </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="426"/>
        <source>The coordinates can not be reprojected. The SRS is: </source>
        <translation>Las coordenadas no se pueden reproyectar. El SRS es: </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="425"/>
        <source>The destination spatial reference system (SRS) is not valid. </source>
        <translation>El sistema espacial de referencia (SRS) de destino no es válido. </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>Failed</source>
        <translation>Ha fallado</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>transform of</source>
        <translation>la transformación de</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="496"/>
        <source>with error: </source>
        <translation>con el error: </translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="66"/>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="79"/>
        <source>&amp;Copyright Label</source>
        <translation>Etiqueta de &amp;Copyright</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="80"/>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>Crea una etiqueta de copyright que se muestra en la vista del mapa.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="204"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Ilustraciones</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="13"/>
        <source>Copyright Label Plugin</source>
        <translation>Complemento etiqueta de copyright</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="145"/>
        <source>Placement</source>
        <translation>Ubicación</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="158"/>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="163"/>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="168"/>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="176"/>
        <source>Orientation</source>
        <translation>Orientación</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="184"/>
        <source>Horizontal</source>
        <translation>Horizontal</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="189"/>
        <source>Vertical</source>
        <translation>Vertical</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="118"/>
        <source>Enable Copyright Label</source>
        <translation>Activar etiqueta de copyright</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="36"/>
        <source>Color</source>
        <translation>Color</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Descripción&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Introduzca su etiqueta de copyright debajo. Este complemento soporta etiquetas básicas html para formatear la etiqueta. Por ejemplo:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Texto en negritas  &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Cursivas &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(nota: &amp;amp;copy; da un símbolo de copyright&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
        <translation>¿Borrar definición de proyección?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="176"/>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Borrar una definición de proyección no es reversible. ¿Desea continuar?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="882"/>
        <source>Abort</source>
        <translation>Abortar</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="884"/>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="939"/>
        <source>QGIS Custom Projection</source>
        <translation>Proyección personalizada de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="756"/>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, dele un nombre a la proyección antes de guardar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="762"/>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, rellene los parámetros antes de guardar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="777"/>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, añada una cláusula proj= antes de guardar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="784"/>
        <source>This proj4 ellipsoid definition is not valid. Please add a ellips= clause before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, añada una cláusula ellips= antes de guardar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="800"/>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, corregir antes de guardar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="913"/>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Esta definición de proyección proj4 no es válida.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="928"/>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>El Norte y el Este deben estar en formato decimal.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="940"/>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Error interno (¿la proyección original no es válida?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="13"/>
        <source>Custom Projection Definition</source>
        <translation>Definición de proyección personalizada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="22"/>
        <source>Define</source>
        <translation>Definición</translation>
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
        <translation>1 de 1</translation>
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
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="151"/>
        <source>Test</source>
        <translation>Probar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="180"/>
        <source>Geographic / WGS84</source>
        <translation>Geográficas / WGS84</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="253"/>
        <source>Calculate</source>
        <translation>Calcular</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="28"/>
        <source>You can define your own custom projection here. The definition must conform to the proj4 format for specifying a Spatial Reference System.</source>
        <translation>Puede definir su propia proyección personalizada aquí. La definición debe ajustarse al formato proj4 para especificar un sistema de referencia espacial.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="157"/>
        <source>Use the text boxes below to test the projection definition you are creating. Enter a coordinate where both the lat/long and the projected result are known (for example by reading off a map). Then press the calculate button to see if the projection definition you are creating is accurate.</source>
        <translation>Use los cuadros de texto de abajo para probar la definición de proyección que esté creando. Introduzca una coordenada en la que la tanto lat/long como el resultado proyectado sean conocidos (por ejemplo tomándolos de un mapa). A continuación pulse el botón calcular para ver su la definición de proyección que ha creado es exacta.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="187"/>
        <source>Projected Coordinate System</source>
        <translation>Sistema de coordenadas proyectado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="38"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="167"/>
        <source>Parameters</source>
        <translation>Parámetros</translation>
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
        <translation>Norte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="224"/>
        <source>East</source>
        <translation>Este</translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="147"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="155"/>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="230"/>
        <source>Are you sure you want to remove the </source>
        <translation>¿Está seguro de que quiere eliminar la conexión </translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="230"/>
        <source> connection and all associated settings?</source>
        <translation> y su configuración?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="231"/>
        <source>Confirm Delete</source>
        <translation>Confirmar la eliminación</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="338"/>
        <source>Select Table</source>
        <translation>Seleccionar tabla</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="338"/>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Debe seleccionar una tabla para poder añadir una capa.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="371"/>
        <source>Password for </source>
        <translation>Contraseña para </translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="372"/>
        <source>Please enter your password:</source>
        <translation>Por favor, introduzca su contraseña:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="437"/>
        <source>Connection failed</source>
        <translation>Conexión fallida</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="440"/>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>La conexión a %1 en %2 ha fallado. Puede ser que la base de datos esté inactiva o que su configuración sea incorrecta.%3Compruebe el nombre de usuario y la contraseña y pruebe de nuevo.%4La base de datos ha dicho:%5%6</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="123"/>
        <source>Wildcard</source>
        <translation>Comodín</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="127"/>
        <source>RegExp</source>
        <translation>Expresión regular</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="135"/>
        <source>All</source>
        <translation>Todos</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="139"/>
        <source>Schema</source>
        <translation>Esquema</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="143"/>
        <source>Table</source>
        <translation>Tabla</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="151"/>
        <source>Geometry column</source>
        <translation>Columna de geometría</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="547"/>
        <source>Accessible tables could not be determined</source>
        <translation>No se pudieron determinar las tablas accesibles</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="549"/>
        <source>Database connection was successful, but the accessible tables could not be determined.

The error message from the database was:
%1
</source>
        <translation>La conexión a la base de datos tuvo éxito, pero no se pudieron determinar las tablas accesibles.

El mensaje de error de la base de datos fue:
%1</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="554"/>
        <source>No accessible tables found</source>
        <translation>No se encontraron tablas accesibles</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="558"/>
        <source>Database connection was successful, but no accessible tables were found.

Please verify that you have SELECT privilege on a table carrying PostGIS
geometry.</source>
        <translation>La conexión a la base de datos tuvo éxito, pero no se encontraron tablas accesibles.

Por favor, verifique que tiene privilegios para SELECT sobre una tabla que lleve la geometría PostGIS.</translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelectBase</name>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="13"/>
        <source>Add PostGIS Table(s)</source>
        <translation>Añadir tabla(s) PostGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="111"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="114"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="140"/>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="156"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation>Conexiones de PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="66"/>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="73"/>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="80"/>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="87"/>
        <source>Connect</source>
        <translation>Conectar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="183"/>
        <source>Search:</source>
        <translation>Buscar:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="190"/>
        <source>Search mode:</source>
        <translation>Modo de búsqueda:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="200"/>
        <source>Search in columns:</source>
        <translation>Buscar en columnas:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="216"/>
        <source>Search options...</source>
        <translation>Opciones de búsqueda...</translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="24"/>
        <source>Schema</source>
        <translation>Esquema</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="25"/>
        <source>Table</source>
        <translation>Tabla</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="26"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="27"/>
        <source>Geometry column</source>
        <translation>Columna de geometría</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="28"/>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="229"/>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="233"/>
        <source>Multipoint</source>
        <translation>Múltiples puntos</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="237"/>
        <source>Line</source>
        <translation>Línea</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="241"/>
        <source>Multiline</source>
        <translation>Múltiples línea</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="245"/>
        <source>Polygon</source>
        <translation>Polígono</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="249"/>
        <source>Multipolygon</source>
        <translation>Múltiples polígono</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="13"/>
        <source>Delete Attributes</source>
        <translation>Borrar atributos</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="101"/>
        <source>&amp;Add Delimited Text Layer</source>
        <translation>&amp;Añadir capa de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="104"/>
        <source>Add a delimited text file as a map layer. </source>
        <translation>Añade una capa de texto delimitado como una capa del mapa. </translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>The file must have a header row containing the field names. </source>
        <translation>El archivo debe tener una fila de encabezado con los nombres de los campos. </translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>Los campos X e Y son necesarios y deben contener las coordenadas en unidades decimales.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="142"/>
        <source>&amp;Delimited text</source>
        <translation>&amp;Texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="56"/>
        <source>DelimitedTextLayer</source>
        <translation>CapaDeTextoDelimitado</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>No layer name</source>
        <translation>Ningún nombre de capa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Por favor, introduzca un nombre para la capa antes de añadir ésta al mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="211"/>
        <source>No delimiter</source>
        <translation>No hay delimitador</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="211"/>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Por favor, especifique un delimitador antes de analizar el archivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="245"/>
        <source>Choose a delimited text file to open</source>
        <translation>Seleccione un archivo de texto delimitado para abrir</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="35"/>
        <source>Parse</source>
        <translation>Analizar</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="60"/>
        <source>Description</source>
        <translation>Descripción</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="63"/>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation>Seleccione un archivo de texto delimitado que contenga una fila de encabezado y una o más filas de coordenadas X e Y que quiera usar como capa de puntos y este complemento hará el trabajo por usted.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="67"/>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation>Use el recuadro Nombre de la capa para especificar el nombre de la nueva capa en la leyenda. Use el recuadro delimitador para especificar qué delimitador se usa en su archivo ej.: espacio, coma, tabulador o una expresión regular en estilo Perl). Después de elegir un delimitador, pulse el botón Analizar y seleccione las columnas que contienen los valores X e Y para la capa.</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="13"/>
        <source>Create a Layer from a Delimited Text File</source>
        <translation>Crear una capa a partir de un archivo de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="67"/>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Coordenada X&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="88"/>
        <source>Name of the field containing x values</source>
        <translation>Nombre del campo que contiene los valores de la X</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="91"/>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nombre del campo que contiene los valores de la X. Seleccione un campo de la lista. La lista se genera al analizar la fila de encabezado del archivo de texto delimitado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="101"/>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Coordenada Y&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="122"/>
        <source>Name of the field containing y values</source>
        <translation>Nombre del campo que contiene los valores de la Y</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="125"/>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nombre del campo que contiene los valores de la Y. Seleccione un campo de la lista. La lista se genera al analizar la fila de encabezado del archivo de texto delimitado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="246"/>
        <source>Sample text</source>
        <translation>Texto de muestra</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="39"/>
        <source>Delimited Text Layer</source>
        <translation>Capa de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="145"/>
        <source>Delimited text file</source>
        <translation>Archivo de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="152"/>
        <source>Full path to the delimited text file</source>
        <translation>Ruta completa al archivo de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="155"/>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Ruta completa al archivo de texto delimitado. Para poder analizar los campos del archivo correctamente, el delimitador debe definirse antes de introducir el nombre del archivo. Utilice el botón Explorar situado a la derecha de este campo para seleccionar el archivo de entrada.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="168"/>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Explorar para buscar el archivo de texto delimitado a procesar</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="171"/>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Use este botón para buscar el archivo de texto. Este botón no se activará hasta que se haya introducido un delimitador en el recuadro &lt;i&gt; Delimitador&lt;/i&gt;. Una vez que se ha seleccionado un archivo, los cuadros combinados de las coordenadas X e Y se rellenarán con los campos del archivo de texto delimitado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="191"/>
        <source>Layer name</source>
        <translation>Nombre de la capa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="198"/>
        <source>Name to display in the map legend</source>
        <translation>Nombre para mostrar en la leyenda del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="201"/>
        <source>Name displayed in the map legend</source>
        <translation>Nombre mostrado en la leyenda del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="297"/>
        <source>Delimiter</source>
        <translation>Delimitador</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="318"/>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation>Delimitador a usar para dividir campos en el archivo de texto. El delimitador puede ser más de un carácter.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="321"/>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Delimitador a usar para dividir campos en el archivo de texto. El delimitador puede tener uno o más caracteres.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="174"/>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="331"/>
        <source>The delimiter is taken as is</source>
        <translation>El delimitador se toma tal como es</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="334"/>
        <source>Plain characters</source>
        <translation>Caracteres sencillos</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="344"/>
        <source>The delimiter is a regular expression</source>
        <translation>El delimitador es una expresión regular</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="347"/>
        <source>Regular expression</source>
        <translation>Expresión regular</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextProvider</name>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="402"/>
        <source>Note: the following lines were not loaded because Qgis was unable to determine values for the x and y coordinates:
</source>
        <translation>Nota: las líneas siguientes no se han cargado porque Qgis no pudo determinar los valores de las coordenadas X e Y:
</translation>
    </message>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="400"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
</context>
<context>
    <name>QgsDetailedItemWidgetBase</name>
    <message>
        <location filename="../src/ui/qgsdetaileditemwidgetbase.ui" line="13"/>
        <source>Form</source>
        <translation>Formulario</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdetaileditemwidgetbase.ui" line="96"/>
        <source>Heading Label</source>
        <translation>Etiqueta de cabecera</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdetaileditemwidgetbase.ui" line="117"/>
        <source>Detail label</source>
        <translation>Etiqueta de detalle</translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="13"/>
        <source>Buffer features</source>
        <translation>Crear buffer de objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="28"/>
        <source>Parameters</source>
        <translation>Parámetros</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="59"/>
        <source>Geometry column:</source>
        <translation>Columna de la geometría:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="66"/>
        <source>Add the buffered layer to the map?</source>
        <translation>¿Añadir la capa de buffer al mapa?</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="92"/>
        <source>Spatial reference ID:</source>
        <translation>ID de la referencia espacial:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="118"/>
        <source>Schema:</source>
        <translation>Esquema:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="125"/>
        <source>Unique field to use as feature id:</source>
        <translation>Campo único a utilizar como identificador:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="132"/>
        <source>Table name for the buffered layer:</source>
        <translation>Nombre de la tabla de la capa del buffer:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="172"/>
        <source>Create unique object id</source>
        <translation>Crear id de objetos único</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="216"/>
        <source>public</source>
        <translation>público</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="224"/>
        <source>Buffer distance in map units:</source>
        <translation>Distancia de buffer en unidades del mapa:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="234"/>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Crear buffer de los objetos espaciales de la capa: &lt;/h2&gt;</translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <location filename="../src/gui/qgsencodingfiledialog.cpp" line="29"/>
        <source>Encoding:</source>
        <translation>Codificación:</translation>
    </message>
</context>
<context>
    <name>QgsFillStyleWidgetBase</name>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="83"/>
        <source>Form1</source>
        <translation type="unfinished">Form1</translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="84"/>
        <source>Fill Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="85"/>
        <source>col</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="86"/>
        <source>Colour:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="90"/>
        <source>PolyStyleWidget</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="44"/>
        <source>New device %1</source>
        <translation>Nuevo receptor %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="57"/>
        <source>Are you sure?</source>
        <translation>¿Está seguro?</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="58"/>
        <source>Are you sure that you want to delete this device?</source>
        <translation>¿Está seguro de que quiere borrar este receptor?</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="21"/>
        <source>GPS Device Editor</source>
        <translation>Editor de receptores GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="69"/>
        <source>New device</source>
        <translation>Nuevo receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="84"/>
        <source>Delete device</source>
        <translation>Borrar receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="99"/>
        <source>Update device</source>
        <translation>Actualizar receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="132"/>
        <source>Device name:</source>
        <translation>Nombre del receptor:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="147"/>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Así es como aparecerá el nombre del receptor en la lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="156"/>
        <source>Commands</source>
        <translation>Comandos</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="171"/>
        <source>Track download:</source>
        <translation>Descargar track:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="178"/>
        <source>Route upload:</source>
        <translation>Cargar ruta:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="185"/>
        <source>Waypoint download:</source>
        <translation>Descargar waypoint:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="192"/>
        <source>The command that is used to download routes from the device</source>
        <translation>El comando que se usa para descargar rutas del receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="199"/>
        <source>Route download:</source>
        <translation>Descargar ruta:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="206"/>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>El comando que se usa para cargar waypoints al receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="213"/>
        <source>Track upload:</source>
        <translation>Cargar track:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="220"/>
        <source>The command that is used to download tracks from the device</source>
        <translation>El comando que se usa para descargar tracks del receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="227"/>
        <source>The command that is used to upload routes to the device</source>
        <translation>El comando que se usa para cargar rutas al receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="234"/>
        <source>The command that is used to download waypoints from the device</source>
        <translation>El comando que se usa para descargar waypoints del receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="241"/>
        <source>The command that is used to upload tracks to the device</source>
        <translation>El comando que se usa para cargar tracks al receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="248"/>
        <source>Waypoint upload:</source>
        <translation>Cargar waypoint:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="266"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;En los comandos para descargar y cargar puede haber palabras especiales que serán reemplazadas por QGIS cuando se utilicen estos comandos. Estas palabras son:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - ruta de GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%dentro&lt;/span&gt; - el nombre del archivo GPX cuando se está cargando o el puerto cuando se está descargando&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%fuera&lt;/span&gt; - el puerto cuando se está cargando o el nombre del archivo GPX cuando se está descargando&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="298"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="93"/>
        <source>&amp;Gps Tools</source>
        <translation>Herramientas &amp;GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="94"/>
        <source>&amp;Create new GPX layer</source>
        <translation>&amp;Crear nueva capa GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="97"/>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Crea una capa GPX nueva y la muestra en la vista del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="196"/>
        <source>&amp;Gps</source>
        <translation>&amp;GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="160"/>
        <source>Save new GPX file as...</source>
        <translation>Guardar nuevo archivo GPX como...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="160"/>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>Archivo GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="165"/>
        <source>Could not create file</source>
        <translation>No se pudo crear el archivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="167"/>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>No se puede crear un archivo GPX con este nombre. </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="168"/>
        <source>Try again with another name or in another </source>
        <translation>Inténtelo de nuevo con otro nombre o en otro </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="168"/>
        <source>directory.</source>
        <translation>directorio.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="207"/>
        <source>GPX Loader</source>
        <translation>Cargador de GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="209"/>
        <source>Unable to read the selected file.
</source>
        <translation>No se puede leer el archivo seleccionado.
</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="209"/>
        <source>Please reselect a valid file.</source>
        <translation>Por favor, vuelva a seleccionar un archivo válido.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="487"/>
        <source>Could not start process</source>
        <translation>No se pudo iniciar el proceso</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="488"/>
        <source>Could not start GPSBabel!</source>
        <translation>No se pudo iniciar GPSBabel</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="330"/>
        <source>Importing data...</source>
        <translation>Importando datos...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="493"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="274"/>
        <source>Could not import data from %1!

</source>
        <translation>No se pudieron importar datos de %1

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="276"/>
        <source>Error importing data</source>
        <translation>Error al importar datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="476"/>
        <source>Not supported</source>
        <translation>No soportado</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="394"/>
        <source>This device does not support downloading </source>
        <translation>Este receptor no soporta la descarga </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="394"/>
        <source>of </source>
        <translation>de </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="410"/>
        <source>Downloading data...</source>
        <translation>Descargando datos...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="421"/>
        <source>Could not download data from GPS!

</source>
        <translation>No se pudieron descargar datos del GPS

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="423"/>
        <source>Error downloading data</source>
        <translation>Error al descargar datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="477"/>
        <source>This device does not support uploading of </source>
        <translation>Este receptor no soporta el cargar datos de </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="493"/>
        <source>Uploading data...</source>
        <translation>Cargando datos...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="504"/>
        <source>Error while uploading data to GPS!

</source>
        <translation>Error al cargar datos al GPS

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="506"/>
        <source>Error uploading data</source>
        <translation>Error al cargar datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="342"/>
        <source>Could not convert data from %1!

</source>
        <translation>¡No se pudieron convertir los datos desde %1!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="344"/>
        <source>Error converting data</source>
        <translation>Error al convertir datos</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="522"/>
        <source>Choose a filename to save under</source>
        <translation>Seleccione un nombre de archivo para guardar en</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="524"/>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>formato GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="511"/>
        <source>Select GPX file</source>
        <translation>Seleccionar archivo GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="250"/>
        <source>Select file and format to import</source>
        <translation>Seleccionar archivo y formato a importar</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="486"/>
        <source>Waypoints</source>
        <translation>Waypoints</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="486"/>
        <source>Routes</source>
        <translation>Rutas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="272"/>
        <source>Tracks</source>
        <translation>Tracks</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="491"/>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation>QGIS puede realizar conversiones de archivos GPX, usando GPSBabel (%1) para hacer las conversiones.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="492"/>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation>Esto requiere que tenga instalado GPSBabel donde QGIS lo pueda encontrar</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="493"/>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX filename that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation>Seleccione un nombre de archivo GPX de entrada, el tipo de conversión que quiere realizar, un nombre de archivo GPX con el que quiera guardar el archivo convertido y un nombre para la nueva capa creada a partir del resultado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="422"/>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation>GPX es el %1, que se usa para guardar información sobre waypoints, rutas y tracks.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="422"/>
        <source>GPS eXchange file format</source>
        <translation>Formato de archivo GPS eXchange</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="423"/>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation>Seleccione un archivo GPX y luego elija el tipo de objetos espaciales que quiere cargar.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="435"/>
        <source>This tool will help you download data from a GPS device.</source>
        <translation>Esta herramienta le ayudará a descargar datos de un receptor GPS.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="436"/>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation>Seleccione su receptor GPS, el puerto al que está conectado, el tipo de objeto espacial que quiere descargar, un nombre para su nueva capa y el archivo GPX donde quiera guardar los datos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="455"/>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation>Si su receptor no está en la lista o si quiere cambiar alguna configuración, también puede editar los receptores.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="457"/>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation>Esta herramienta usa el programa GPSBabel (%1) para transferir los datos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="453"/>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation>Esta herramienta le ayudará a cargar datos de una capa GPX a un receptor GPS.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="454"/>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation>Seleccione la capa que quiere cargar , el receptor en el que la quiere cargar y el puerto al que está conectado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="472"/>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation>QGIS sólo puede cargar archivos GPX por si mismo, pero otros muchos formatos se pueden convertir a GPX usando GPSBabel (%1).</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="474"/>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer.</source>
        <translation>Seleccione un formato de archivo de GPS y el archivo que quiere importar, el tipo de objeto espacial que quiere usar, un nombre de archivo con el que quiera guardar el archivo convertido a GPX y un nombre para la nueva capa.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="475"/>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation>No todos los formatos de archivo pueden guardar waypoints, rutas y tracks, por lo que algunos tipos de objeto espacial pueden estar deshabilitados para algunos formatos de archivo.</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="13"/>
        <source>GPS Tools</source>
        <translation>Herramientas GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="58"/>
        <source>Load GPX file</source>
        <translation>Cargar archivo GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="94"/>
        <source>File:</source>
        <translation>Archivo:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="111"/>
        <source>Feature types:</source>
        <translation>Tipo de objetos espaciales:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="314"/>
        <source>Waypoints</source>
        <translation>Waypoints</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="319"/>
        <source>Routes</source>
        <translation>Rutas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="324"/>
        <source>Tracks</source>
        <translation>Tracks</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="158"/>
        <source>Import other file</source>
        <translation>Importar otro archivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="558"/>
        <source>Layer name:</source>
        <translation>Nombre de la capa:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="582"/>
        <source>GPX output file:</source>
        <translation>Archivo de salida GPX:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="339"/>
        <source>Feature type:</source>
        <translation>Tipo de objeto espacial:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="264"/>
        <source>File to import:</source>
        <translation>Archivo a importar:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="272"/>
        <source>Download from GPS</source>
        <translation>Descargar desde GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="481"/>
        <source>Port:</source>
        <translation>Puerto:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="356"/>
        <source>Output file:</source>
        <translation>Archivo de salida:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="515"/>
        <source>GPS device:</source>
        <translation>Receptor GPS:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="508"/>
        <source>Edit devices</source>
        <translation>Editar receptores</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="430"/>
        <source>Upload to GPS</source>
        <translation>Cargar al GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="522"/>
        <source>Data layer:</source>
        <translation>Capa de datos:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="572"/>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="565"/>
        <source>Save As...</source>
        <translation>Guardar como...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="250"/>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation>(Nota: ¡Seleccionar el tipo de archivo correcto en el diálogo de exploración es importante!)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="530"/>
        <source>GPX Conversions</source>
        <translation>Conversiones GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="615"/>
        <source>Conversion:</source>
        <translation>Conversión:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="629"/>
        <source>GPX input file:</source>
        <translation>Archivo GPX de entrada:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="545"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="70"/>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>URI errónea - necesita especificar el tipo de objeto espacial.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="114"/>
        <source>GPS eXchange file</source>
        <translation>Archivo GPS eXchange</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="731"/>
        <source>Digitized in QGIS</source>
        <translation>Digitalizado en QGIS</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="30"/>
        <source>Real</source>
        <translation>Real</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="31"/>
        <source>Integer</source>
        <translation>Entero</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="32"/>
        <source>String</source>
        <translation>Cadena</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="13"/>
        <source>New Vector Layer</source>
        <translation>Nueva capa vectorial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="155"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="41"/>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="48"/>
        <source>Line</source>
        <translation>Línea</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="55"/>
        <source>Polygon</source>
        <translation>Polígono</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="150"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="22"/>
        <source>File format</source>
        <translation>Formato de archivo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="65"/>
        <source>Attributes</source>
        <translation>Atributos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="111"/>
        <source>Remove selected row</source>
        <translation>Eliminar la fila seleccionada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="127"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="124"/>
        <source>Add values manually</source>
        <translation>Añadir valores manualmente</translation>
    </message>
</context>
<context>
    <name>QgsGeorefDescriptionDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefdescriptiondialogbase.ui" line="13"/>
        <source>Description georeferencer</source>
        <translation>Georreferenciador de descripción</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Descripciónn&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;&quot;&gt;Este complemento puede generar archivos de referenciación (world files) para imágenes ráster. Seleccione puntos en el ráster e introduzca sus coordenadas y el complemento procesará los parámetros del archivo de referenciación. Cuantas más coordenadas pueda proporcionar, mejor será el resultado&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPlugin</name>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="119"/>
        <source>&amp;Georeferencer</source>
        <translation>Geo&amp;rreferenciador</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="85"/>
        <source>Choose a raster file</source>
        <translation>Seleccionar un archivo ráster</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="87"/>
        <source>Raster files (*.*)</source>
        <translation>Archivos ráster (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="97"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="98"/>
        <source>The selected file is not a valid raster file.</source>
        <translation>El archivo seleccionado no es un archivo ráster válido.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="122"/>
        <source>World file exists</source>
        <translation>El archivo de referenciación (world file) ya existe</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="124"/>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation>&lt;p&gt;Parece que el archivo seleccionado ya tiene un </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>world file! Do you want to replace it with the </source>
        <translation>archivo de referenciación (world file). ¿Quiere reemplazarlo con el </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>new world file?&lt;/p&gt;</source>
        <translation>nuevo archivo de referenciación (world file)?&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="13"/>
        <source>Georeferencer</source>
        <translation>Georreferenciador</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="62"/>
        <source>Raster file:</source>
        <translation>Archivo ráster:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="100"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="28"/>
        <source>Arrange plugin windows</source>
        <translation>Ajustar ventanas de complementos</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="43"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="77"/>
        <source>Description...</source>
        <translation>Descripción...</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialog.cpp" line="27"/>
        <source>unstable</source>
        <translation>inestable</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="13"/>
        <source>Warp options</source>
        <translation>Opciones de combado</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="35"/>
        <source>Resampling method:</source>
        <translation>Método de remuestreo:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="46"/>
        <source>Nearest neighbour</source>
        <translation>Vecino más próximo</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="51"/>
        <source>Linear</source>
        <translation>Lineal</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="56"/>
        <source>Cubic</source>
        <translation>Cúbica</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="74"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="64"/>
        <source>Use 0 for transparency when needed</source>
        <translation>Usar 0 para transparencia cuando sea necesario</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="28"/>
        <source>Compression:</source>
        <translation>Compresión:</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="324"/>
        <source>Equal Interval</source>
        <translation>Intervalo igual</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="301"/>
        <source>Quantiles</source>
        <translation>Cuantiles</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="350"/>
        <source>Empty</source>
        <translation>Vacío</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="25"/>
        <source>graduated Symbol</source>
        <translation>Símbolo graduado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="188"/>
        <source>Delete class</source>
        <translation>Borrar clase</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="181"/>
        <source>Classify</source>
        <translation>Clasificar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="55"/>
        <source>Classification field</source>
        <translation>Falló la clasificación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="93"/>
        <source>Mode</source>
        <translation>Modo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="131"/>
        <source>Number of classes</source>
        <translation>Número de clases</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="300"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="152"/>
        <source>Column</source>
        <translation>Columna</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="153"/>
        <source>Value</source>
        <translation>Valor</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="154"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="301"/>
        <source>ERROR</source>
        <translation>ERROR</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="303"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="158"/>
        <source>Layer</source>
        <translation>Capa</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="48"/>
        <source>GRASS Attributes</source>
        <translation>Atributos de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="78"/>
        <source>Tab 1</source>
        <translation>Tab 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="112"/>
        <source>result</source>
        <translation>resultado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="177"/>
        <source>Update database record</source>
        <translation>Actualizar registro de la base de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="180"/>
        <source>Update</source>
        <translation>Actualizar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="207"/>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Añadir nueva categoría utilizando las especificaciones de la caja de herramientas de edición de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="210"/>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="237"/>
        <source>Delete selected category</source>
        <translation>Borrar la categoría seleccionada</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="240"/>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="66"/>
        <source>Tools</source>
        <translation>Herramientas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="71"/>
        <source>Add selected map to canvas</source>
        <translation>Añadir el mapa seleccionado a la vista del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="79"/>
        <source>Copy selected map</source>
        <translation>Copiar el mapa seleccionado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="87"/>
        <source>Rename selected map</source>
        <translation>Cambiar nombre del mapa seleccionado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="95"/>
        <source>Delete selected map</source>
        <translation>Borrar el mapa seleccionado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="103"/>
        <source>Set current region to selected map</source>
        <translation>Establecer la región actual al mapa seleccionado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="111"/>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="453"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="289"/>
        <source>Cannot copy map </source>
        <translation>No se puede copiar el mapa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="411"/>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt;comando: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="355"/>
        <source>Cannot rename map </source>
        <translation>No se puede cambiar el nombre del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="393"/>
        <source>Delete map &lt;b&gt;</source>
        <translation>Borrar mapa &lt;b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="410"/>
        <source>Cannot delete map </source>
        <translation>No se puede borrar el mapa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="454"/>
        <source>Cannot write new region</source>
        <translation>No se puede guardar una nueva región</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="339"/>
        <source>New name</source>
        <translation>Nuevo nombre</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1458"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="201"/>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>No es propietario del directorio de mapas, no puede abrir el vectorial para editarlo.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="206"/>
        <source>Cannot open vector for update.</source>
        <translation>No se puede abrir el vectorial para actualizar.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="214"/>
        <source>Edit tools</source>
        <translation>Herramientas de edición</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="217"/>
        <source>New point</source>
        <translation>Nuevo punto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="223"/>
        <source>New line</source>
        <translation>Nueva línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="229"/>
        <source>New boundary</source>
        <translation>Nuevo contorno</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="235"/>
        <source>New centroid</source>
        <translation>Nuevo centroide</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="241"/>
        <source>Move vertex</source>
        <translation>Mover vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="247"/>
        <source>Add vertex</source>
        <translation>Añadir vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="253"/>
        <source>Delete vertex</source>
        <translation>Borrar vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="259"/>
        <source>Move element</source>
        <translation>Mover elemento</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="265"/>
        <source>Split line</source>
        <translation>Dividir línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="271"/>
        <source>Delete element</source>
        <translation>Borrar elemento</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="277"/>
        <source>Edit attributes</source>
        <translation>Editar atributos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="282"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="665"/>
        <source>Info</source>
        <translation>Info</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="665"/>
        <source>The table was created</source>
        <translation>La tabla ha sido creada</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1333"/>
        <source>Tool not yet implemented.</source>
        <translation>Herramienta no implementada todavía.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1357"/>
        <source>Cannot check orphan record: </source>
        <translation>No se puede comprobar registro huérfano: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1364"/>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation>El registro huérfano se dejó en la tabla de atributos. &lt;br&gt;¿Borrar el registro?</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1373"/>
        <source>Cannot delete orphan record: </source>
        <translation>No se puede borrar el registro huérfano: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1401"/>
        <source>Cannot describe table for field </source>
        <translation>No se puede describir la tabla por el campo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1821"/>
        <source>Left: </source>
        <translation>Izquierda: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1822"/>
        <source>Middle: </source>
        <translation>Medio: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="363"/>
        <source>Background</source>
        <translation>Fondo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="364"/>
        <source>Highlight</source>
        <translation>Resaltado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="365"/>
        <source>Dynamic</source>
        <translation>Dinámico</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="366"/>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="367"/>
        <source>Line</source>
        <translation>Línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="368"/>
        <source>Boundary (no area)</source>
        <translation>Contorno (sin área)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="369"/>
        <source>Boundary (1 area)</source>
        <translation>Contorno (1 área)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="370"/>
        <source>Boundary (2 areas)</source>
        <translation>Contorno (2 áreas)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="371"/>
        <source>Centroid (in area)</source>
        <translation>Centroide (en el área)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="372"/>
        <source>Centroid (outside area)</source>
        <translation>Centroide (fuera del área)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="373"/>
        <source>Centroid (duplicate in area)</source>
        <translation>Centroide (duplicado en el área)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="374"/>
        <source>Node (1 line)</source>
        <translation>Nodo (1 línea)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="375"/>
        <source>Node (2 lines)</source>
        <translation>Nodo (2 líneas)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="408"/>
        <source>Disp</source>
        <comment>Column title</comment>
        <translation>Título</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="444"/>
        <source>Column</source>
        <translation>Columna</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="445"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="446"/>
        <source>Length</source>
        <translation>Longitud</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="501"/>
        <source>Next not used</source>
        <translation>La siguiente sin usar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="502"/>
        <source>Manual entry</source>
        <translation>Entrada manual</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="503"/>
        <source>No category</source>
        <translation>Ninguna categoría</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1823"/>
        <source>Right: </source>
        <translation>Derecha: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="410"/>
        <source>Color</source>
        <comment>Column title</comment>
        <translation>Color</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="412"/>
        <source>Type</source>
        <comment>Column title</comment>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="414"/>
        <source>Index</source>
        <comment>Column title</comment>
        <translation>Índice</translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="16"/>
        <source>GRASS Edit</source>
        <translation>Edición de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="106"/>
        <source>Category</source>
        <translation>Categoría</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="66"/>
        <source>Mode</source>
        <translation>Modo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="170"/>
        <source>Settings</source>
        <translation>Configuración</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="190"/>
        <source>Snapping in screen pixels</source>
        <translation>Autoensamblado en píxeles de la pantalla</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="241"/>
        <source>Symbology</source>
        <translation>Simbología</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="274"/>
        <source>Column 1</source>
        <translation>Columna 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="298"/>
        <source>Line width</source>
        <translation>Ancho de línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="325"/>
        <source>Marker size</source>
        <translation>Tamaño de marcador</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="360"/>
        <source>Table</source>
        <translation>Tabla</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="496"/>
        <source>Add Column</source>
        <translation>Añadir columna</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="511"/>
        <source>Create / Alter Table</source>
        <translation>Crear / modificar tabla</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="410"/>
        <source>Layer</source>
        <translation>Capa</translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="132"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="163"/>
        <source>Ok</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="168"/>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;¡Introduzca un nombre!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="179"/>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;¡Este es el nombre del origen!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="185"/>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;¡Existe!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="186"/>
        <source>Overwrite</source>
        <translation>Sobrescribir</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="111"/>
        <source>Mapcalc tools</source>
        <translation>Herramientas Mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="114"/>
        <source>Add map</source>
        <translation>Añadir mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="121"/>
        <source>Add constant value</source>
        <translation>Añadir constante</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="128"/>
        <source>Add operator or function</source>
        <translation>Añadir operador o función</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="135"/>
        <source>Add connection</source>
        <translation>Añadir conexión</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="142"/>
        <source>Select item</source>
        <translation>Seleccionar elemento</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="149"/>
        <source>Delete selected item</source>
        <translation>Borrar elemento seleccionado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="159"/>
        <source>Open</source>
        <translation>Abrir</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="164"/>
        <source>Save</source>
        <translation>Guardar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="170"/>
        <source>Save as</source>
        <translation>Guardar como</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="178"/>
        <source>Addition</source>
        <translation>Suma</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="179"/>
        <source>Subtraction</source>
        <translation>Resta</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="180"/>
        <source>Multiplication</source>
        <translation>Multiplicación</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="181"/>
        <source>Division</source>
        <translation>División</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="182"/>
        <source>Modulus</source>
        <translation>Módulo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="183"/>
        <source>Exponentiation</source>
        <translation>Exponencial</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="186"/>
        <source>Equal</source>
        <translation>Igual</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="187"/>
        <source>Not equal</source>
        <translation>Distinto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="188"/>
        <source>Greater than</source>
        <translation>Mayor que</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="189"/>
        <source>Greater than or equal</source>
        <translation>Mayor o igual que</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="190"/>
        <source>Less than</source>
        <translation>Menor que</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="191"/>
        <source>Less than or equal</source>
        <translation>Menor o igual que</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="192"/>
        <source>And</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="193"/>
        <source>Or</source>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="196"/>
        <source>Absolute value of x</source>
        <translation>Valor absoluto de x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="197"/>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>Inverso de la tangente de x (resultado en grados)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="198"/>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>Inverso de la tangente de y/x (resultado en grados)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="199"/>
        <source>Current column of moving window (starts with 1)</source>
        <translation>Columna actual de la ventana movible (empieza con 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="200"/>
        <source>Cosine of x (x is in degrees)</source>
        <translation>Coseno de x (x en grados)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="201"/>
        <source>Convert x to double-precision floating point</source>
        <translation>Convertir x a coma flotante (precisión doble)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="202"/>
        <source>Current east-west resolution</source>
        <translation>Resolución actual Este-Oeste</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="203"/>
        <source>Exponential function of x</source>
        <translation>Elevado a x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="204"/>
        <source>x to the power y</source>
        <translation>x elevado a y</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="205"/>
        <source>Convert x to single-precision floating point</source>
        <translation>Convertir x a coma flotante (precisión simple)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="206"/>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>Decisión: 1 si x no es cero, 0 para el resto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="207"/>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>Decisión: a si x no es cero, 0 para el resto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="208"/>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>Decisión: a si x no es cero, b para el resto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="209"/>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>Decisión: a si x &gt; 0, b si x es cero, c si x &lt; 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="210"/>
        <source>Convert x to integer [ truncates ]</source>
        <translation>Convertir x a entero [ truncar ]</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="211"/>
        <source>Check if x = NULL</source>
        <translation>Comprobar si x = NULO</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="212"/>
        <source>Natural log of x</source>
        <translation>Logaritmo neperiano de x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="213"/>
        <source>Log of x base b</source>
        <translation>Logaritmo de x en base b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="215"/>
        <source>Largest value</source>
        <translation>Máximo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="217"/>
        <source>Median value</source>
        <translation>Mediana</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="219"/>
        <source>Smallest value</source>
        <translation>Mínimo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="221"/>
        <source>Mode value</source>
        <translation>Moda</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="222"/>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1 si x = 0, 0 para el resto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="223"/>
        <source>Current north-south resolution</source>
        <translation>Resolución Norte-Sur actual</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="224"/>
        <source>NULL value</source>
        <translation>Valor NULO</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="225"/>
        <source>Random value between a and b</source>
        <translation>Valor aleatorio entre a y b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="226"/>
        <source>Round x to nearest integer</source>
        <translation>Redondear x al número entero más próximo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="227"/>
        <source>Current row of moving window (Starts with 1)</source>
        <translation>Fila actual de la ventana movible (empieza con 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="228"/>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation>Seno de x (x en grados)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="229"/>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>Raíz cuadrada de x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="230"/>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation>Tangente de x (x en grados)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="231"/>
        <source>Current x-coordinate of moving window</source>
        <translation>Coordenada X de la ventana movible</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="232"/>
        <source>Current y-coordinate of moving window</source>
        <translation>Coordenada Y de la ventana movible</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1317"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="584"/>
        <source>Cannot get current region</source>
        <translation>No se puede estimar la región actual</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="561"/>
        <source>Cannot check region of map </source>
        <translation>No se puede comprobar la región del mapa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="617"/>
        <source>Cannot get region of map </source>
        <translation>No se puede obtener la región del mapa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="813"/>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>Actualmente no hay mapas ráster de GRASS en QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1102"/>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>No se puede crear el directorio &quot;mapcalc&quot; en el directorio de mapas actual.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1112"/>
        <source>New mapcalc</source>
        <translation>Nuevo mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1113"/>
        <source>Enter new mapcalc name:</source>
        <translation>Introduzca el nombre del nuevo mapcalc:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1118"/>
        <source>Enter vector name</source>
        <translation>Introducir nombre del vectorial</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1126"/>
        <source>The file already exists. Overwrite? </source>
        <translation>El archivo ya existe. ¿Desea sobrescribirlo?</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1164"/>
        <source>Save mapcalc</source>
        <translation>Guardar mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1146"/>
        <source>File name empty</source>
        <translation>Nombre de archivo vacío</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1165"/>
        <source>Cannot open mapcalc file</source>
        <translation>No se puede abrir el archivo mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>The mapcalc schema (</source>
        <translation>El esquema del mapcalc (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>) not found.</source>
        <translation>) no se ha encontrado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1302"/>
        <source>Cannot open mapcalc schema (</source>
        <translation>No se puede abrir el esquema del mapcalc (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1313"/>
        <source>Cannot read mapcalc schema (</source>
        <translation>No se puede leer el esquema del mapcalc (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1314"/>
        <source>
at line </source>
        <translation>
en la línea </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1315"/>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1388"/>
        <source>Output</source>
        <translation>Salida</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="16"/>
        <source>MainWindow</source>
        <translation>Ventana principal</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="37"/>
        <source>Output</source>
        <translation>Salida</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1380"/>
        <source>Run</source>
        <translation>Ejecutar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1358"/>
        <source>Stop</source>
        <translation>Detener</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="205"/>
        <source>Module</source>
        <translation>Módulo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1352"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="220"/>
        <source>The module file (</source>
        <translation>El archivo del módulo (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="220"/>
        <source>) not found.</source>
        <translation>) no se ha encontrado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="224"/>
        <source>Cannot open module file (</source>
        <translation>No se puede abrir el archivo del módulo (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="992"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="987"/>
        <source>Cannot read module file (</source>
        <translation>No se puede leer el archivo del módulo (</translation>
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
en la línea </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="260"/>
        <source>Module </source>
        <translation>Módulo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="260"/>
        <source> not found</source>
        <translation> no encontrado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="301"/>
        <source>Cannot find man page </source>
        <translation>No se puede encontrar la página del manual </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="981"/>
        <source>Not available, cannot open description (</source>
        <translation>No disponible, no se puede abrir la descripción (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="988"/>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="992"/>
        <source>Not available, incorrect description (</source>
        <translation>No disponible, descripción incorrecta (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1179"/>
        <source>Cannot get input region</source>
        <translation>No se puede obtener la región de entrada</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1167"/>
        <source>Use Input Region</source>
        <translation>Usar región de entrada</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1281"/>
        <source>Cannot find module </source>
        <translation>No se puede encontrar el módulo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1353"/>
        <source>Cannot start module: </source>
        <translation>No se puede iniciar el módulo: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1369"/>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation>&lt;B&gt;Finalizado correctamente&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1375"/>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation>&lt;B&gt;Finalizado con error&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1378"/>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation>&lt;B&gt;Módulo finalizado o matado&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="978"/>
        <source>Not available, description not found (</source>
        <translation>No disponible, no se encuentra la descripción (</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="13"/>
        <source>GRASS Module</source>
        <translation>Módulo de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="26"/>
        <source>Options</source>
        <translation>Opciones</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="31"/>
        <source>Output</source>
        <translation>Salida</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="47"/>
        <source>Manual</source>
        <translation>Manual</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="118"/>
        <source>Run</source>
        <translation>Ejecutar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="141"/>
        <source>View output</source>
        <translation>Ver salida</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="161"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="74"/>
        <source>TextLabel</source>
        <translation>EtiquetaDeTexto</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2714"/>
        <source>Attribute field</source>
        <translation>Campo de atributos</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2916"/>
        <source>File</source>
        <translation>Archivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3029"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;falta el valor</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3036"/>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation>:&amp;nbsp;el directorio no existe</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2665"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2527"/>
        <source>Cannot find layeroption </source>
        <translation>No se puede encontrar la opción capa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2669"/>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation>El controlador PostGIS de OGR no soporta esquemas.&lt;br&gt;Sólo se usará el nombre de la tabla.&lt;br&gt;Esto puede dar como resultado entradas incorrectas si hay más tablas con el mismo nombre&lt;br&gt;en la base de datos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2692"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;ninguna entrada</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2540"/>
        <source>Cannot find whereoption </source>
        <translation>No se puede encontrar la opción dónde </translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1984"/>
        <source>Cannot find typeoption </source>
        <translation>No se puede encontrar la opción tipo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1993"/>
        <source>Cannot find values for typeoption </source>
        <translation>No se pueden encontrar valores para la opción tipo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2054"/>
        <source>Cannot find layeroption </source>
        <translation>No se puede encontrar la opción capa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source>GRASS element </source>
        <translation>Elemento de GRASS </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source> not supported</source>
        <translation> no soportado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2095"/>
        <source>Use region of this map</source>
        <translation>Usar la región de este mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2431"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;ninguna entrada</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1900"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;falta el valor</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2803"/>
        <source>Attribute field</source>
        <translation>Campo de atributo</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="886"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="369"/>
        <source>Cannot find module </source>
        <translation>No se puede encontrar el módulo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="386"/>
        <source>Cannot start module </source>
        <translation>No se puede iniciar el módulo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="399"/>
        <source>Cannot read module description (</source>
        <translation>No se puede leer la descripción del módulo (</translation>
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
en la línea </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="400"/>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="424"/>
        <source>Cannot find key </source>
        <translation>No se puede encontrar la clave </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="560"/>
        <source>Item with id </source>
        <translation>Ítem con ID </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="560"/>
        <source> not found</source>
        <translation> no encontrado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="848"/>
        <source>Cannot get current region</source>
        <translation>No se puede obtener la región actual</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="825"/>
        <source>Cannot check region of map </source>
        <translation>No se puede comprobar la región del mapa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="887"/>
        <source>Cannot set region of map </source>
        <translation>No se puede establecer la región del mapa </translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="124"/>
        <source>GRASS database</source>
        <translation>Base de datos de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="125"/>
        <source>GRASS location</source>
        <translation>Localización de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="126"/>
        <source>Projection</source>
        <translation>Proyección</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="127"/>
        <source>Default GRASS Region</source>
        <translation>Región predeterminada de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="184"/>
        <source>Mapset</source>
        <translation>Directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="129"/>
        <source>Create New Mapset</source>
        <translation>Crear nuevo directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="158"/>
        <source>Tree</source>
        <translation>Árbol</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="159"/>
        <source>Comment</source>
        <translation>Comentario</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="160"/>
        <source>Database</source>
        <translation>Base de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="164"/>
        <source>Location 2</source>
        <translation>Localización 2</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="175"/>
        <source>User&apos;s mapset</source>
        <translation>Directorio de mapas del usuario</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="177"/>
        <source>System mapset</source>
        <translation>Directorio de mapas del sistema</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="171"/>
        <source>Location 1</source>
        <translation>Localización 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="185"/>
        <source>Owner</source>
        <translation>Propietario</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="233"/>
        <source>Enter path to GRASS database</source>
        <translation>Introduzca la ruta a la base de datos de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="241"/>
        <source>The directory doesn&apos;t exist!</source>
        <translation>¡El directorio no existe!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="271"/>
        <source>No writable locations, the database not writable!</source>
        <translation>¡Ninguna localización que se pueda escribir, la base de datos no se puede escribir!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="369"/>
        <source>Enter location name!</source>
        <translation>¡Introduzca el nombre de la localización!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="382"/>
        <source>The location exists!</source>
        <translation>¡La localización existe!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="533"/>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>¡La proyección seleccionada no está soportada por GRASS!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1160"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="580"/>
        <source>Cannot create projection.</source>
        <translation>No se puede crear la proyección.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="629"/>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation>No se puede reproyectar la región establecida previamente, se establece la región predeterminada.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="764"/>
        <source>North must be greater than south</source>
        <translation>El Norte debe ser mayor que el Sur</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="769"/>
        <source>East must be greater than west</source>
        <translation>El Este debe ser mayor que el Oeste</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="816"/>
        <source>Regions file (</source>
        <translation>Archivo de regiones (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="816"/>
        <source>) not found.</source>
        <translation>) no encontrado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="821"/>
        <source>Cannot open locations file (</source>
        <translation>No se puede abrir el archivo de localizaciones (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="821"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="830"/>
        <source>Cannot read locations file (</source>
        <translation>No se puede leer el archivo de localizaciones (</translation>
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
en la línea </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="832"/>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1161"/>
        <source>Cannot create QgsSpatialRefSys</source>
        <translation>No se puede crear QgsSpatialRefSys</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="968"/>
        <source>Cannot reproject selected region.</source>
        <translation>No se puede reproyectar la región seleccionada.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1057"/>
        <source>Cannot reproject region</source>
        <translation>No se puede reproyectar la región</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1289"/>
        <source>Enter mapset name.</source>
        <translation>Introduzca el nombre del directorio de mapas.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1306"/>
        <source>The mapset already exists</source>
        <translation>El directorio de mapas ya existe</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1330"/>
        <source>Database: </source>
        <translation>Base de datos: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1341"/>
        <source>Location: </source>
        <translation>Localización: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1343"/>
        <source>Mapset: </source>
        <translation>Directorio de mapas: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1378"/>
        <source>Create location</source>
        <translation>Crear localización</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1380"/>
        <source>Cannot create new location: </source>
        <translation>No se puede crear la nueva localización: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1427"/>
        <source>Create mapset</source>
        <translation>Crear directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1420"/>
        <source>Cannot open DEFAULT_WIND</source>
        <translation>No se puede abrir DEFAULT_WIND</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1427"/>
        <source>Cannot open WIND</source>
        <translation>No se puede abrir WIND</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1454"/>
        <source>New mapset</source>
        <translation>Nuevo directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1450"/>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>El nuevo directorio de mapas de creó correctamente, pero no se puede abrir: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1456"/>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>El nuevo directorio de mapas se creó correctamente y se estableció como el directorio de mapas de trabajo actual.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1410"/>
        <source>Cannot create new mapset directory</source>
        <translation>No se puede crear el nuevo directorio de mapas</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2068"/>
        <source>Column 1</source>
        <translation>Columna 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="88"/>
        <source>Example directory tree:</source>
        <translation>Ejemplo de árbol de directorios:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="95"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS data are stored in tree directory structure.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Los datos de GRASS se guardan en un directorio con estructura de árbol.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;La base de datos de GRASS se encuentra en el directorio más elevado de esta estructura.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="399"/>
        <source>Database Error</source>
        <translation>Error de la base de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2153"/>
        <source>Database:</source>
        <translation>Base de datos:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="440"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="457"/>
        <source>Select existing directory or create a new one:</source>
        <translation>Seleccionar un directorio existente o crear uno nuevo:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="508"/>
        <source>Location</source>
        <translation>Localización</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="535"/>
        <source>Select location</source>
        <translation>Seleccionar localización</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="552"/>
        <source>Create new location</source>
        <translation>Crear nueva localización</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="832"/>
        <source>Location Error</source>
        <translation>Error en la localización</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="848"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;La localización de GRASS es una colección de mapas de un territorio o proyecto particular.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1159"/>
        <source>Projection Error</source>
        <translation>Error de la proyección</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1174"/>
        <source>Coordinate system</source>
        <translation>Sistema de coordenadas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1186"/>
        <source>Projection</source>
        <translation>Proyección</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1193"/>
        <source>Not defined</source>
        <translation>Sin definir</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1273"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;La región de GRASS define un entorno de trabajo para módulos ráster. La región predeterminada es válida para una localización. Es posible establecer una región diferente en cada directorio de mapas. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;La región de la localización predeterminada se puede cambiar después.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1334"/>
        <source>Set current QGIS extent</source>
        <translation>Establecer la extensión actual de QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1376"/>
        <source>Set</source>
        <translation>Establecer</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1396"/>
        <source>Region Error</source>
        <translation>Error de la región</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1441"/>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1500"/>
        <source>W</source>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1555"/>
        <source>E</source>
        <translation>E</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1614"/>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1699"/>
        <source>New mapset:</source>
        <translation>Nuevo directorio de mapas:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1988"/>
        <source>Mapset Error</source>
        <translation>Error del directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2045"/>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;center&quot;&gt;Directorios de mapas existentes&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2101"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS mapset is a collection of maps used by one user. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A user can read maps from all mapsets in the location but &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;El directorio de mapas de GRASS es una colección de mapas utilizada por un usuario. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Un usuario puede leer mapas de todos los directorio de mapas de la localización pero &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;sólo puede escribir en su directorio de mapas (el que le pertenezca).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2174"/>
        <source>Location:</source>
        <translation>Localización:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2195"/>
        <source>Mapset:</source>
        <translation>Directorio de mapas:</translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="145"/>
        <source>Open mapset</source>
        <translation>Abrir directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="146"/>
        <source>New mapset</source>
        <translation>Nuevo directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="147"/>
        <source>Close mapset</source>
        <translation>Cerrar directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="150"/>
        <source>Add GRASS vector layer</source>
        <translation>Añadir capa vectorial de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="152"/>
        <source>Add GRASS raster layer</source>
        <translation>Añadir capa ráster de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="168"/>
        <source>Open GRASS tools</source>
        <translation>Abrir herramientas de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="157"/>
        <source>Display Current Grass Region</source>
        <translation>Mostrar región actual de Grass</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="161"/>
        <source>Edit Current Grass Region</source>
        <translation>Editar la región actual de Grass</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="163"/>
        <source>Edit Grass Vector layer</source>
        <translation>Editar capa vectorial de Grass</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="166"/>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>Añade una capa vectorial de GRASS a la vista del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="167"/>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>Añade una capa ráster de GRASS a la vista del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="169"/>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Muestra la región actual de GRASS como un rectángulo en la vista del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="170"/>
        <source>Edit the current GRASS region</source>
        <translation>Editar la región actual de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="171"/>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Editar la capa vectorial de GRASS seleccionada actualmente.</translation>
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
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="82"/>
        <source>GrassVector</source>
        <translation>VectorialDeGrass</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="83"/>
        <source>0.1</source>
        <translation>0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="84"/>
        <source>GRASS layer</source>
        <translation>Capa de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="164"/>
        <source>Create new Grass Vector</source>
        <translation>Crear nuevo vectorial de Grass</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="758"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="470"/>
        <source>GRASS Edit is already running.</source>
        <translation>Le edición de GRASS ya se está ejecutando.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="479"/>
        <source>New vector name</source>
        <translation>Nombre del nuevo vectorial</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="495"/>
        <source>Cannot create new vector: </source>
        <translation>No se puede crear el nuevo vectorial: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="515"/>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation>Se creó el nuevo vectorial pero el proveedor de datos no lo puede abrir.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="526"/>
        <source>Cannot start editing.</source>
        <translation>No se puede iniciar la edición.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="561"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>BASE DE DATOS, NOMBRE_LOCALIZACIÓN o DIRECTORIO DE MAPAS no están establecidos, no se puede mostrar la región actual.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="571"/>
        <source>Cannot read current region: </source>
        <translation>No se puede leer la región actual: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="675"/>
        <source>Cannot open the mapset. </source>
        <translation>No se abrir el directorio de mapas. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="693"/>
        <source>Cannot close mapset. </source>
        <translation>No se puede cerrar el directorio de mapas. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="749"/>
        <source>Cannot close current mapset. </source>
        <translation>No se puede cerrar el directorio de mapas actual. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="758"/>
        <source>Cannot open GRASS mapset. </source>
        <translation>No se puede abrir el directorio de mapas de GRASS. </translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="459"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="195"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>BASE DE DATOS, NOMBRE_LOCALIZACIÓN o DIRECTORIO DE MAPAS no están establecidos, no se puede mostrar la región actual.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="202"/>
        <source>Cannot read current region: </source>
        <translation>No se puede leer la región actual: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="459"/>
        <source>Cannot write region</source>
        <translation>No se puede escribir la región</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="13"/>
        <source>GRASS Region Settings</source>
        <translation>Configuración de la región de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="76"/>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="146"/>
        <source>W</source>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="172"/>
        <source>E</source>
        <translation>E</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="236"/>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="280"/>
        <source>N-S Res</source>
        <translation>Resolución N-S</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="293"/>
        <source>Rows</source>
        <translation>Filas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="303"/>
        <source>Cols</source>
        <translation>Columnas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="316"/>
        <source>E-W Res</source>
        <translation>Resolución E-O</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="364"/>
        <source>Color</source>
        <translation>Color</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="384"/>
        <source>Width</source>
        <translation>Anchura</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="464"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="487"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="68"/>
        <source>Select GRASS Vector Layer</source>
        <translation>Seleccionar capa vectorial de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="75"/>
        <source>Select GRASS Raster Layer</source>
        <translation>Seleccionar capa ráster de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="82"/>
        <source>Select GRASS mapcalc schema</source>
        <translation>Seleccionar esquema mapcalc de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="90"/>
        <source>Select GRASS Mapset</source>
        <translation>Seleccionar directorio de mapas de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="409"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="409"/>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation>No se puede abrir el vectorial en el nivel 2 (topología no disponible).</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="466"/>
        <source>Choose existing GISDBASE</source>
        <translation>Elija una base de datos (GISBASE) existente</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="482"/>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>Base de datos (GISBASE) incorrecta, ninguna localización disponible.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="483"/>
        <source>Wrong GISDBASE</source>
        <translation>Base de datos (GISBASE) incorrecta</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="500"/>
        <source>Select a map.</source>
        <translation>Seleccionar un mapa.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="501"/>
        <source>No map</source>
        <translation>Ningún mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="509"/>
        <source>No layer</source>
        <translation>Ninguna capa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="510"/>
        <source>No layers available in this map</source>
        <translation>No hay ninguna capa disponible en este mapa</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="21"/>
        <source>Add GRASS Layer</source>
        <translation>Añadir capa de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="65"/>
        <source>Gisdbase</source>
        <translation>Base de datos (Gisdbase)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="78"/>
        <source>Location</source>
        <translation>Localización</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="85"/>
        <source>Mapset</source>
        <translation>Directorio de mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="102"/>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Seleccionar o escribir el nombre del mapa (se aceptan los comodines &apos;*&apos; y &apos;?&apos; para los ráster)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="118"/>
        <source>Map name</source>
        <translation>Nombre del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="125"/>
        <source>Layer</source>
        <translation>Capa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="161"/>
        <source>Browse</source>
        <translation>Explorar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="168"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="175"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="19"/>
        <source>GRASS Shell</source>
        <translation>Consola de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="49"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="146"/>
        <source>Modules</source>
        <translation>Módulos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="205"/>
        <source>Browser</source>
        <translation>Explorador</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="123"/>
        <source>GRASS Tools</source>
        <translation>Herramientas de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="470"/>
        <source>GRASS Tools: </source>
        <translation>Herramientas de GRASS: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="358"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="271"/>
        <source>Cannot find MSYS (</source>
        <translation>No se puede encontrar MSYS (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="293"/>
        <source>GRASS Shell is not compiled.</source>
        <translation>La consola de GRASS no está compilada.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="343"/>
        <source>The config file (</source>
        <translation>El archivo de configuración (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="343"/>
        <source>) not found.</source>
        <translation>) no se ha encontrado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="347"/>
        <source>Cannot open config file (</source>
        <translation>No se puede abrir el archivo de configuración (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="347"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="355"/>
        <source>Cannot read config file (</source>
        <translation>No se puede leer el archivo de configuración (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="356"/>
        <source>
at line </source>
        <translation>
en la línea </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="356"/>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="143"/>
        <source>Modules Tree</source>
        <translation>Árbol de módulos</translation>
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
        <translation>&amp;Generador de cuadrícula</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="94"/>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation>Crea una cuadricula (grid) y guarda el resultado como un archivo shape</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="136"/>
        <source>&amp;Graticules</source>
        <translation>&amp;Cuadrículas</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="51"/>
        <source>QGIS - Grid Maker</source>
        <translation>QGIS - Creador de cuadrículas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="108"/>
        <source>Choose a filename to save under</source>
        <translation>Seleccione un nombre para guardar el archivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="110"/>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>Archivo Shape de ESRI (*.shp)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="52"/>
        <source>Please enter the file name before pressing OK!</source>
        <translation>¡Por favor, introduzca el nombre de archivo antes de pulsar Aceptar!</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="256"/>
        <source>Graticule Builder</source>
        <translation>Generador de cuadrículas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="186"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="198"/>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="208"/>
        <source>Polygon</source>
        <translation>Polígono</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="110"/>
        <source>Origin (lower left)</source>
        <translation>Origen (esquina inferior izquierda)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="69"/>
        <source>End point (upper right)</source>
        <translation>Punto final (esquina superior derecha)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="161"/>
        <source>Output (shape) file</source>
        <translation>Archivo de salida (shape)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="176"/>
        <source>Save As...</source>
        <translation>Guardar como...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="13"/>
        <source>QGIS Graticule Creator</source>
        <translation>Generador de cuadrículas de QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="28"/>
        <source>Graticle size</source>
        <translation>Tamaño de cuadrícula</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="46"/>
        <source>Y Interval:</source>
        <translation>Intervalo Y</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="56"/>
        <source>X Interval:</source>
        <translation>Intervalo X</translation>
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
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Esta complemento le ayudará a construir un archivo shape de cuadrícula que puede usar superpuesto en la vista del mapa de qgis.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Por favor, introduzca todas las unidades en grados decimales.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="139"/>
        <source>This help file does not exist for your language</source>
        <translation>Este archivo de ayuda no existe en su idioma</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="142"/>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation>Si desea crearlo, contacte con el equipo de desarrollo de QGIS</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="157"/>
        <source>Quantum GIS Help</source>
        <translation>Ayuda de Quantum GIS</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="185"/>
        <source>Quantum GIS Help - </source>
        <translation>Ayuda de Quantum GIS - </translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="214"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="191"/>
        <source>Failed to get the help text from the database</source>
        <translation>No se pudo obtener el texto de ayuda de la base de datos</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="215"/>
        <source>The QGIS help database is not installed</source>
        <translation>La base de datos de ayuda de QGIS no está instalada</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="13"/>
        <source>QGIS Help</source>
        <translation>Ayuda de QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="78"/>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="81"/>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="39"/>
        <source>&amp;Home</source>
        <translation>&amp;Inicio</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="42"/>
        <source>Alt+H</source>
        <translation>Alt+I</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="52"/>
        <source>&amp;Forward</source>
        <translation>A&amp;delante</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="55"/>
        <source>Alt+F</source>
        <translation>Alt+D</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="65"/>
        <source>&amp;Back</source>
        <translation>A&amp;trás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="68"/>
        <source>Alt+B</source>
        <translation>Alt+T</translation>
    </message>
</context>
<context>
    <name>QgsHttpTransaction</name>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="234"/>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation>El servidor WMS ha respondido de forma inesperada con el código de estado HTTP %1 (%2)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="313"/>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>Respuesta HTTP completada, sin embargo ha habido un error: %1</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="362"/>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>Transacción HTTP completada, sin embargo ha habido un error: %1</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/core/qgshttptransaction.cpp" line="441"/>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
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
        <location filename="../src/app/qgsidentifyresults.cpp" line="44"/>
        <source>Feature</source>
        <translation>Objeto espacial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="45"/>
        <source>Value</source>
        <translation>Valor</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="106"/>
        <source>Run action</source>
        <translation>Ejecutar acción</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="196"/>
        <source>(Derived)</source>
        <translation>(Derivado)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="225"/>
        <source>Identify Results - </source>
        <translation>Resultados de la identificación - </translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="13"/>
        <source>Identify Results</source>
        <translation>Resultados de la identificación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="43"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="46"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="72"/>
        <source>Close</source>
        <translation>Cerrar</translation>
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
        <translation type="unfinished">Salida</translation>
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
        <translation>Introducir contornos de la clase</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="40"/>
        <source>Lower value</source>
        <translation>Valor más bajo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="79"/>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="66"/>
        <source>Upper value</source>
        <translation>Valor más alto</translation>
    </message>
</context>
<context>
    <name>QgsLabelDialogBase</name>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="19"/>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="323"/>
        <source>Font size units</source>
        <translation>Unidades del tamaño de fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="512"/>
        <source>Map units</source>
        <translation>Unidades del mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="505"/>
        <source>Points</source>
        <translation>Puntos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="776"/>
        <source>Transparency:</source>
        <translation>Transparencia:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="153"/>
        <source>Font</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="411"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="731"/>
        <source>Placement</source>
        <translation>Ubicación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="254"/>
        <source>Below Right</source>
        <translation>Abajo derecha</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="261"/>
        <source>Right</source>
        <translation>Derecha</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="268"/>
        <source>Below</source>
        <translation>Abajo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="275"/>
        <source>Over</source>
        <translation>Sobre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="285"/>
        <source>Above</source>
        <translation>Encima de</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="292"/>
        <source>Left</source>
        <translation>Izquierda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="299"/>
        <source>Below Left</source>
        <translation>Abajo izquierda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="306"/>
        <source>Above Right</source>
        <translation>Encima derecha</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="313"/>
        <source>Above Left</source>
        <translation>Abajo izquierda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="99"/>
        <source>Buffer</source>
        <translation>Margen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="431"/>
        <source>Buffer size units</source>
        <translation>Unidades de tamaño del margen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="444"/>
        <source>Size is in map units</source>
        <translation>Tamaño en unidades del mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="437"/>
        <source>Size is in points</source>
        <translation>El tamaño está en puntos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="799"/>
        <source>Size:</source>
        <translation>Tamaño:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="104"/>
        <source>Position</source>
        <translation>Posición</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="499"/>
        <source>Offset units</source>
        <translation>Unidades de desplazamiento</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="890"/>
        <source>Preview:</source>
        <translation>Previsualizar:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="908"/>
        <source>QGIS Rocks!</source>
        <translation>¡QGIS Avanza!</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="39"/>
        <source>Field containing label</source>
        <translation>Campo que contiene la etiqueta</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="62"/>
        <source>Default label</source>
        <translation>Etiqueta predeterminada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="109"/>
        <source>Data defined style</source>
        <translation>Estilo definido por datos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="114"/>
        <source>Data defined alignment</source>
        <translation>Alineación definida por datos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="119"/>
        <source>Data defined buffer</source>
        <translation>Buffer definido por datos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="124"/>
        <source>Data defined position</source>
        <translation>Posición definida por datos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="163"/>
        <source>Font transparency</source>
        <translation>Transparencia de tipo de letra</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="424"/>
        <source>Color</source>
        <translation>Color</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="747"/>
        <source>Angle (deg)</source>
        <translation>Ángulo (grados)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&#xb0;</source>
        <translation type="obsolete">°</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="363"/>
        <source>Buffer labels?</source>
        <translation>¿Hacer buffer de etiquetas?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="373"/>
        <source>Buffer size</source>
        <translation>Tamaño de buffer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="701"/>
        <source>Transparency</source>
        <translation>Transparencia</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="854"/>
        <source>X Offset (pts)</source>
        <translation>Desplazamiento X (pts)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="870"/>
        <source>Y Offset (pts)</source>
        <translation>Desplazamiento Y (pts)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="545"/>
        <source>&amp;Font family</source>
        <translation>&amp;Familia del tipo de letra</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="571"/>
        <source>&amp;Bold</source>
        <translation>&amp;Negrita</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="597"/>
        <source>&amp;Italic</source>
        <translation>&amp;Cursiva</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="623"/>
        <source>&amp;Underline</source>
        <translation>&amp;Subrayado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="649"/>
        <source>&amp;Size</source>
        <translation>&amp;Tamaño</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="675"/>
        <source>Size units</source>
        <translation>Unidades de tamaño</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="822"/>
        <source>X Coordinate</source>
        <translation>Coordenada X</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="838"/>
        <source>Y Coordinate</source>
        <translation>Coordenada Y</translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/ui/qgslabeldialogbase.ui" line="221"/>
        <source>°</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelector</name>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="34"/>
        <source>Define this layer&apos;s projection:</source>
        <translation>Definir la proyección de esta capa:</translation>
    </message>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="35"/>
        <source>This layer appears to have no projection specification.</source>
        <translation>Parece que esta capa no tiene no tiene especificada la proyección.</translation>
    </message>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="37"/>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation>Por omisión, se utilizará la misma que para el proyecto, pero puede ignorarla seleccionado una proyección diferente a continuación.</translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelectorBase</name>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="13"/>
        <source>Layer Projection Selector</source>
        <translation>Selector de proyección de la capa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="80"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="113"/>
        <source>group</source>
        <translation>grupo</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="426"/>
        <source>&amp;Remove</source>
        <translation>E&amp;liminar</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="419"/>
        <source>&amp;Make to toplevel item</source>
        <translation>&amp;Subir el elemento al nivel superior</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="431"/>
        <source>Re&amp;name</source>
        <translation>Cambiar &amp;nombre</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="436"/>
        <source>&amp;Add group</source>
        <translation>&amp;Añadir grupo</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="437"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Expandir todo</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="438"/>
        <source>&amp;Collapse all</source>
        <translation>&amp;Comprimir todo</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="440"/>
        <source>Show file groups</source>
        <translation>Mostrar grupos de archivos</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1827"/>
        <source>No Layer Selected</source>
        <translation>Ninguna capa no seleccionada</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1828"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Para abrir una tabla de atributos, debe seleccionar una capa vectorial en la leyenda</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="490"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Zum a la extensión de la capa</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="493"/>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation>&amp;Zum a la mejor escala (100%)</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="497"/>
        <source>&amp;Show in overview</source>
        <translation>Mo&amp;strar en el localizador</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="503"/>
        <source>&amp;Remove</source>
        <translation>E&amp;liminar</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="510"/>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Abrir tabla de atributos</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="534"/>
        <source>Save as shapefile...</source>
        <translation>Guardar como archivo shape...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="541"/>
        <source>Save selection as shapefile...</source>
        <translation>Guardar selección como archivo shape...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="551"/>
        <source>&amp;Properties</source>
        <translation>&amp;Propiedades</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="600"/>
        <source>More layers</source>
        <translation>Más capas</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="601"/>
        <source>This item contains more layer files. Displaying more layers in table is not supported.</source>
        <translation>Este elemento contiene más archivos de capas. Mostrar más capas en la tabla no está soportado.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="276"/>
        <source>Attribute table - </source>
        <translation>Tabla de atributos - </translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="346"/>
        <source>Save layer as...</source>
        <translation>Guardar capa como...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="426"/>
        <source>Start editing failed</source>
        <translation>Ha fallado el comienzo de la edición</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="427"/>
        <source>Provider cannot be opened for editing</source>
        <translation>El proveedor no se puede abrir para editar</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="440"/>
        <source>Stop editing</source>
        <translation>Terminar edición</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="440"/>
        <source>Do you want to save the changes?</source>
        <translation>¿Quiere guardar los cambios?</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="457"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="446"/>
        <source>Could not commit changes</source>
        <translation>No se pudieron aplicar los cambios</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="458"/>
        <source>Problems during roll back</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="227"/>
        <source>Not a vector layer</source>
        <translation>No es una capa vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="228"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Para abrir una tabla de atributos, debe seleccionar una capa vectorial en la leyenda</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="393"/>
        <source>Saving done</source>
        <translation>Guardado terminado</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="393"/>
        <source>Export to Shapefile has been completed</source>
        <translation>La exportación a archivo shape se ha completado</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="397"/>
        <source>Driver not found</source>
        <translation>No se ha encontrado el controlador</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="397"/>
        <source>ESRI Shapefile driver is not available</source>
        <translation>El controlador de archivos shape de ESRI no está disponible</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="401"/>
        <source>Error creating shapefile</source>
        <translation>Error al crear el archivo shape</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="402"/>
        <source>The shapefile could not be created (</source>
        <translation>El archivo shape no se ha podido crear (</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="406"/>
        <source>Layer creation failed</source>
        <translation>Ha fallado la creación de la capa</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="501"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Zum a la extensión de la capa</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="504"/>
        <source>&amp;Show in overview</source>
        <translation>Mo&amp;strar en el localizador</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="512"/>
        <source>&amp;Remove</source>
        <translation>E&amp;liminar</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="521"/>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Abrir tabla de atributos</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="535"/>
        <source>Save as shapefile...</source>
        <translation>Guardar como archivo shape...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="537"/>
        <source>Save selection as shapefile...</source>
        <translation>Guardar selección como archivo shape...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="554"/>
        <source>&amp;Properties</source>
        <translation>&amp;Propiedades</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="271"/>
        <source>bad_alloc exception</source>
        <translation>excepción bad_alloc</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="271"/>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation>El relleno de la tabla de atributos se ha detenido porque no quedaba más memoria virtual</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="410"/>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation>La tabla de atributos de la capa contiene tipos de datos no soportados</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1224"/>
        <source>Could not draw</source>
        <translation>No se pudo dibujar</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1224"/>
        <source>because</source>
        <translation>porque</translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <location filename="" line="135533324"/>
        <source> Check file permissions and retry.</source>
        <translation type="obsolete"> Comprobar permisos de archivo y volver a intentar.</translation>
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
        <translation>No se han encontrado objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="568"/>
        <source>&lt;p&gt;No features were found within the search radius. Note that it is currently not possible to use the identify tool on unsaved features.&lt;/p&gt;</source>
        <translation>&lt;p&gt;No se han encontrado objetos espaciales dentro del radio de búsqueda. Tenga en cuenta que actualmente no es posible utilizar la herramienta de identificación en objetos espaciales sin guardar.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="424"/>
        <source>(clicked coordinate)</source>
        <translation>(coordenada pinchada)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="223"/>
        <source>WMS identify result for %1
%2</source>
        <translation>Resultado de identificación WMS para %1
%2</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="491"/>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="86"/>
        <source>Split error</source>
        <translation>Error en la división</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="86"/>
        <source>An error occured during feature splitting</source>
        <translation>Ocurrió un error durante la división de los objetos espaciales</translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="51"/>
        <source>Snap tolerance</source>
        <translation>Tolerancia de autoensamblado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="52"/>
        <source>Don&apos;t show this message again</source>
        <translation>No volver a mostrar este mensaje</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="57"/>
        <source>Could not snap segment.</source>
        <translation>No se pudo autoensamblar el segmento.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="60"/>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>¿Ha establecido la tolerancia de autoensamblado en Configuración &gt; Propiedades del proyecto &gt; General?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="76"/>
        <source>Name for the map file</source>
        <translation>Nombre para el archivo del mapa</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="84"/>
        <source>Choose the QGIS project file</source>
        <translation>Seleccione el archivo de proyecto de QGIS</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="85"/>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>Archivos de proyectos de QGIS (*.qgs);;Todos los archivos (*.*)</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="197"/>
        <source>Overwrite File?</source>
        <translation>¿Sobrescribir archivo?</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="199"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a filename is prepended to this text, and appears in a dialog box</comment>
        <translation> existe. 
¿Quiere sobrescribirlo?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmapserverexport.cpp" line="74"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation> existe. 
¿Quiere sobrescribirlo?</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="77"/>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>Archivos de mapa de MapServer (*.map);;Todos los archivos (*.*)</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="14"/>
        <source>Export to Mapserver</source>
        <translation>Exportar a MapServer</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="352"/>
        <source>Map file</source>
        <translation>Archivo de mapa</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="403"/>
        <source>Export LAYER information only</source>
        <translation>Exportar sólo información de la CAPA</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="56"/>
        <source>&amp;Help</source>
        <translation>&amp;Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="59"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="85"/>
        <source>&amp;OK</source>
        <translation>&amp;Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="101"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="116"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="159"/>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="330"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="304"/>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="182"/>
        <source>Units</source>
        <translation>Unidades</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="229"/>
        <source>Image type</source>
        <translation>Tipo de imagen</translation>
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
        <translation>definida por el usuario</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="273"/>
        <source>wbmp</source>
        <translation>wbmp</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="226"/>
        <source>MinScale</source>
        <translation>Escala mínima</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="236"/>
        <source>MaxScale</source>
        <translation>Escala máxima</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="252"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation>Prefijo añadido al nombre de los archivos GIF del mapa, la barra de escala y la leyenda creados usando este archivo de mapa. Debe ser corto.</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="291"/>
        <source>Width</source>
        <translation>Anchura</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="29"/>
        <source>Web Interface Definition</source>
        <translation>Definición del interfaz web</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="98"/>
        <source>Header</source>
        <translation>Encabezado</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="134"/>
        <source>Footer</source>
        <translation>Pie de página</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="58"/>
        <source>Template</source>
        <translation>Plantilla</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="68"/>
        <source>Path to the MapServer template file</source>
        <translation>Ruta al archivo de plantilla de MapServer</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="196"/>
        <source>dd</source>
        <translation>dd</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="201"/>
        <source>feet</source>
        <translation>pies</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="206"/>
        <source>meters</source>
        <translation>metros</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="211"/>
        <source>miles</source>
        <translation>millas</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="216"/>
        <source>inches</source>
        <translation>pulgadas</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="221"/>
        <source>kilometers</source>
        <translation>kilómetros</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="340"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation>Prefijo añadido al nombre de los archivos GIF del mapa, la barra de escala y la leyenda creados usando este archivo de mapa</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="362"/>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation>Nombre para el archivo de mapa que se va a crear a partir del proyecto de QGIS</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="376"/>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation>Ruta completa al proyecto de QGIS para exportar al formato de mapa de MapServer</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="383"/>
        <source>QGIS project file</source>
        <translation>Archivo de proyecto de QGIS</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="400"/>
        <source>If checked, only the layer information will be processed</source>
        <translation>Si se marca, sólo será procesada la información de la capa</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="369"/>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="393"/>
        <source>Save As...</source>
        <translation>Guardar como...</translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="19"/>
        <source>Measure</source>
        <translation>Medida</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="66"/>
        <source>Total:</source>
        <translation>Total:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="102"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="125"/>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="132"/>
        <source>Cl&amp;ose</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="86"/>
        <source>Segments</source>
        <translation>Segmentos</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="198"/>
        <source>Segments (in meters)</source>
        <translation>Segmentos (en metros)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="201"/>
        <source>Segments (in feet)</source>
        <translation>Segmentos (en pies)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="204"/>
        <source>Segments (in degrees)</source>
        <translation>Segmentos (en grados)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="207"/>
        <source>Segments</source>
        <translation>Segmentos</translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="74"/>
        <source>Incorrect measure results</source>
        <translation>Resultados de medida incorrectos</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="82"/>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation>&lt;p&gt;Este mapa está definido con un sistema de coordenadas geográficas (latitud/longitud), pero la extensión del mapa sugiere que en realidad es un sistema de coordenadas proyectado (ej.: Mercator). Si es así, las medidas de líneas o áreas serán incorrectas.&lt;/p&gt;&lt;p&gt;Para corregir esto, establezca de forma explicita un sistema de coordenadas del mapa adecuado usando el menú &lt;tt&gt;Configuración&gt;Propiedades de proyecto&lt;/tt&gt;.</translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="13"/>
        <source>QGIS Message</source>
        <translation>Mensaje de QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="28"/>
        <source>Don&apos;t show this message again</source>
        <translation>No mostrar este mensaje de nuevo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="48"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
</context>
<context>
    <name>QgsMySQLProvider</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Unable to access relation</source>
        <translation type="obsolete">No se puede acceder a la relación</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Unable to access the </source>
        <translation type="obsolete">No se puede acceder a la relación </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> relation.
The error message from the database was:
</source>
        <translation type="obsolete">.
El mensaje de error de la base de datos fue:
</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>No GEOS Support!</source>
        <translation type="obsolete">No hay soporte para GEOS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="obsolete">Su instalación de PostGIS no tiene soporte para GEOS.
La selección e identificación de objetos espaciales no funcionará correctamente.
Por favor, instale PostGIS con soporte para GEOS (http://geos.refractions.net)</translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="121"/>
        <source>Test connection</source>
        <translation>Probar conexión</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="121"/>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>La conexión ha fallado - Compruebe la configuración y vuelva a intentarlo.

Información extensa sobre el error:
</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="118"/>
        <source>Connection to %1 was successful</source>
        <translation>La conexión a %1 tuvo éxito</translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="21"/>
        <source>Create a New PostGIS connection</source>
        <translation>Crear una nueva conexión a PostGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="39"/>
        <source>Connection Information</source>
        <translation>Información sobre la conexión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="67"/>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation>Buscar sólo en el esquema &quot;público&quot;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="90"/>
        <source>Only look in the geometry_columns table</source>
        <translation>Buscar sólo en la tabla de columnas de la geometría</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="105"/>
        <source>Save Password</source>
        <translation>Guardar contraseña</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="112"/>
        <source>Test Connect</source>
        <translation>Probar conexión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="137"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="147"/>
        <source>Host</source>
        <translation>Servidor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="157"/>
        <source>Database</source>
        <translation>Base de datos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="167"/>
        <source>Port</source>
        <translation>Puerto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="177"/>
        <source>Username</source>
        <translation>Nombre de usuario</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="187"/>
        <source>Password</source>
        <translation>Contraseña</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="207"/>
        <source>Name of the new connection</source>
        <translation>Nombre de la nueva conexión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="220"/>
        <source>5432</source>
        <translation>5432</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="252"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="268"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="284"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="287"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="61"/>
        <source>Restrict the search to the public schema for spatial tables not in the geometry_columns table</source>
        <translation>Restringir la búsqueda al esquema público de las tablas espaciales que no están en la tabla de columnas de la geometría</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="64"/>
        <source>When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)</source>
        <translation>Cuando se busca en tablas espaciales que no están en las tablas de columnas de la geometría, restringir la búsqueda a tablas que están en el esquema público (en algunas bases de datos esto puede ahorrar mucho tiempo)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="84"/>
        <source>Restrict the displayed tables to those that are in the geometry_columns table</source>
        <translation>Restringir las tablas mostradas a aquellas que están en la tabla de columnas de la geometría</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="87"/>
        <source>Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.</source>
        <translation>Restringe las tablas mostradas a aquellas que están en la tabla de columnas de la geometría. Esto puede acelerar la visualización inicial de las tablas espaciales.</translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Create a New WMS connection</source>
        <translation type="obsolete">Crear una nueva conexión WMS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Connection Information</source>
        <translation type="obsolete">Información sobre la conexión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="50"/>
        <source>Name of the new connection</source>
        <translation>Nombre de la nueva conexión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="31"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="60"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Proxy Host</source>
        <translation type="obsolete">Servidor proxy</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Proxy Port</source>
        <translation type="obsolete">Puerto proxy</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Proxy User</source>
        <translation type="obsolete">Usuario proxy</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Proxy Password</source>
        <translation type="obsolete">Contraseña proxy</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Your user name for the HTTP proxy (optional)</source>
        <translation type="obsolete">Nombre de usuario para el proxy HTTP (opcional)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Password for your HTTP proxy (optional)</source>
        <translation type="obsolete">Contraseña para su proxy HTTP (opcional)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="73"/>
        <source>HTTP address of the Web Map Server</source>
        <translation>Dirección HTTP del servidor de mapas web</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name of your HTTP proxy (optional)</source>
        <translation type="obsolete">Nombre de su proxy HTTP (opcional)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Port number of your HTTP proxy (optional)</source>
        <translation type="obsolete">Puerto de su proxy HTTP (opcional)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>OK</source>
        <translation type="obsolete">Aceptar</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">Cancelar</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Help</source>
        <translation type="obsolete">Ayuda</translation>
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
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="96"/>
        <source>&amp;North Arrow</source>
        <translation>Flecha de &amp;Norte</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="97"/>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>Crea una flecha de norte que se muestra en la vista del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="255"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Ilustraciones</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="246"/>
        <source>North arrow pixmap not found</source>
        <translation>No se ha encontrado la imagen de flecha de Norte</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugingui.cpp" line="157"/>
        <source>Pixmap not found</source>
        <translation>No se ha encontrado la imagen</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="235"/>
        <source>North Arrow Plugin</source>
        <translation>Complemento de flecha de Norte</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="35"/>
        <source>Properties</source>
        <translation>Propiedades</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="55"/>
        <source>Angle</source>
        <translation>Ángulo</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="62"/>
        <source>Placement</source>
        <translation>Ubicación</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="75"/>
        <source>Set direction automatically</source>
        <translation>Establecer la dirección automáticamente</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="85"/>
        <source>Enable North Arrow</source>
        <translation>Activar flecha de Norte</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="137"/>
        <source>Placement on screen</source>
        <translation>Ubicación en la pantalla</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="141"/>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="146"/>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="151"/>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="156"/>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="164"/>
        <source>Preview of north arrow</source>
        <translation>Previsualización de la flecha de Norte</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="183"/>
        <source>Icon</source>
        <translation>Icono</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="198"/>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="154"/>
        <source>Detected active locale on your system: </source>
        <translation>Idioma activo detectado en su sistema: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="332"/>
        <source>to vertex</source>
        <translation>a vértice</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="336"/>
        <source>to segment</source>
        <translation>a segmento</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="340"/>
        <source>to vertex and segment</source>
        <translation>a vértice y segmento</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="349"/>
        <source>Semi transparent circle</source>
        <translation>Círculo semitransparente</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="353"/>
        <source>Cross</source>
        <translation>Cruz</translation>
    </message>
</context>
<context>
    <name>QgsOptionsBase</name>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="13"/>
        <source>QGIS Options</source>
        <translation>Opciones de QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="212"/>
        <source>Hide splash screen at startup</source>
        <translation>Ocultar la pantalla de bienvenida al iniciar la aplicación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="188"/>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Nota: &lt;/b&gt;Los cambios de tema tendrán efecto la próxima vez que inicie QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="236"/>
        <source>&amp;Rendering</source>
        <translation>&amp;Representación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="265"/>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>La vista del mapa se actualizará después de que todos estos objetos espaciales sean leídos de la fuente de datos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="248"/>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Por omisión, las nuevas &amp;capas añadidas al mapa se deben visualizar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="306"/>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>Hacer que las líneas se muestren menos quebradas a expensas de la calidad del dibujo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="316"/>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Solucionar problemas con polígonos rellenados incorrectamente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="323"/>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Redibujar el mapa continuamente cuando se desplaza el separador entre mapa y leyenda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="347"/>
        <source>&amp;Map tools</source>
        <translation>Herramientas de &amp;mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="507"/>
        <source>Search radius</source>
        <translation>Radio de búsqueda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="545"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="437"/>
        <source>Measure tool</source>
        <translation>Herramienta de medida</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="365"/>
        <source>Panning and zooming</source>
        <translation>Desplazar y zum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="384"/>
        <source>Zoom</source>
        <translation>Zum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="389"/>
        <source>Zoom and recenter</source>
        <translation>Zum y centrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="399"/>
        <source>Nothing</source>
        <translation>Nada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="788"/>
        <source>Pro&amp;jection</source>
        <translation>Pro&amp;yección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="819"/>
        <source>Select Global Default ...</source>
        <translation>Seleccionar proyección global predeterminada...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="829"/>
        <source>When layer is loaded that has no projection information</source>
        <translation>Cuando se carga una capa que no tiene información sobre la proyección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="32"/>
        <source>&amp;General</source>
        <translation>&amp;General</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="872"/>
        <source>Locale</source>
        <translation>Idioma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="887"/>
        <source>Locale to use instead</source>
        <translation>Idioma a usar en su lugar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="926"/>
        <source>Additional Info</source>
        <translation>Información adicional</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="932"/>
        <source>Detected active locale on your system:</source>
        <translation>Idioma activo detectado en su sistema:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="313"/>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation>Con esta opción desactivará el &quot;hacer las líneas menos quebradas&quot;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="578"/>
        <source>Digitizing</source>
        <translation>Digitalización</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="584"/>
        <source>Rubberband</source>
        <translation>Banda elástica</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="600"/>
        <source>Line width in pixels</source>
        <translation>Anchura de la línea en píxeles</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="636"/>
        <source>Snapping</source>
        <translation>Autoensamblado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="394"/>
        <source>Zoom to mouse cursor</source>
        <translation>Zoom al cursor del ratón</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="735"/>
        <source>Vertex markers</source>
        <translation>Marcadores de vértices</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="38"/>
        <source>Project files</source>
        <translation>Archivos de proyecto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="56"/>
        <source>Prompt to save project changes when required</source>
        <translation>Preguntar si guardar cambios en el proyecto cuando sea necesario</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="63"/>
        <source>Warn when opening a project file saved with an older version of QGIS</source>
        <translation>Avisar cuando se abra un proyecto guardado con una versión anterior de QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="73"/>
        <source>Default Map Appearance (overridden by project properties)</source>
        <translation>Apariencia predeterminada del mapa (anulada por las propiedades del proyecto)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="79"/>
        <source>Selection color</source>
        <translation>Color de selección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="115"/>
        <source>Background color</source>
        <translation>Color de fondo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="154"/>
        <source>&amp;Application</source>
        <translation>&amp;Aplicación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="166"/>
        <source>Icon theme</source>
        <translation>Tema de iconos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="198"/>
        <source>Capitalise layer names in legend</source>
        <translation>Comenzar el nombre de las capas con mayúsculas en la leyenda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="205"/>
        <source>Display classification attribute names in legend</source>
        <translation>Mostrar nombre de atributos de clasificación en la leyenda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="242"/>
        <source>Rendering behavior</source>
        <translation>Comportamiento de la representación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="255"/>
        <source>Number of features to draw before updating the display</source>
        <translation>Número de objetos espaciales a dibujar antes de actualizar la visualización</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="278"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Use zero to prevent display updates until all features have been rendered</source>
        <translation>&lt;b&gt;Nota:&lt;/b&gt; Usar cero para evitar actualizaciones de la visualización hasta que todos los objetos espaciales se hayan representado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="288"/>
        <source>Rendering quality</source>
        <translation>Calidad de representación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="407"/>
        <source>Zoom factor</source>
        <translation>Factor de zum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="414"/>
        <source>Mouse wheel action</source>
        <translation>Acción de la rueda del ratón</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="484"/>
        <source>Rubberband color</source>
        <translation>Color de la banda de medida</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="494"/>
        <source>Ellipsoid for distance calculations</source>
        <translation>Elipsoide para el cálculo de distancias</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="525"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Specify the search radius as a percentage of the map width</source>
        <translation>&lt;b&gt;Nota:&lt;/b&gt; Especifique el radio de búsqueda como porcentaje de la anchura del mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="535"/>
        <source>Search radius for identifying features and displaying map tips</source>
        <translation>Radio de búsqueda para identificar objetos espaciales y mostrar avisos del mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="590"/>
        <source>Line width</source>
        <translation>Ancho de línea</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="610"/>
        <source>Line colour</source>
        <translation>Color de línea</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="642"/>
        <source>Default snap mode</source>
        <translation>Modo de autoensamblado por omisión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="672"/>
        <source>Default snapping tolerance in layer units</source>
        <translation>Tolerancia de autoensamblado predeterminada en unidades de la capa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="702"/>
        <source>Search radius for vertex edits in layer units</source>
        <translation>Radio de búsqueda para edición de vértices en unidades de la capa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="741"/>
        <source>Marker style</source>
        <translation>Estilo de marcadores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="847"/>
        <source>Prompt for projection</source>
        <translation>Preguntar por la proyección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="854"/>
        <source>Project wide default projection will be used</source>
        <translation>Usar la proyección amplia predeterminada del proyecto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="861"/>
        <source>Global default projection displa&amp;yed below will be used</source>
        <translation>Usar la proyección global predeterminada &amp;mostrada a continuación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="878"/>
        <source>Override system locale</source>
        <translation>Ignorar el idioma del sistema</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="900"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Enabling / changing overide on local requires an application restart</source>
        <translation>&lt;b&gt;Nota:&lt;/b&gt; Activar / cambiar ignorar el idioma requiere reiniciar la aplicación</translation>
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
        <translation type="unfinished">Servidor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="974"/>
        <source>Port</source>
        <translation type="unfinished">Puerto</translation>
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
        <translation type="unfinished">Contraseña</translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="16"/>
        <source>Paste Transformations</source>
        <translation>Pegar transformaciones</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="39"/>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation>&lt;b&gt;Nota: Esta función todavía no es útil&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="62"/>
        <source>Source</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="83"/>
        <source>Destination</source>
        <translation>Destino</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="122"/>
        <source>&amp;Help</source>
        <translation>&amp;Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="125"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="151"/>
        <source>Add New Transfer</source>
        <translation>Añadir nueva transformación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="158"/>
        <source>&amp;OK</source>
        <translation>&amp;Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="174"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="78"/>
        <source>&amp;Buffer features</source>
        <translation>&amp;Crear buffer de objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation>Se crea una nueva capa en la base de datos con el buffer de los objetos espaciales.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="410"/>
        <source>&amp;Geoprocessing</source>
        <translation>Geo&amp;procesamiento</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="117"/>
        <source>Buffer features in layer %1</source>
        <translation>Crear buffer de objetos espaciales de la capa %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="325"/>
        <source>Unable to add geometry column</source>
        <translation>No se puede añadir la columna de la geometría</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="327"/>
        <source>Unable to add geometry column to the output table </source>
        <translation>No se ha podido añadir la columna de la geomtría a la tabla de salida </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="331"/>
        <source>Unable to create table</source>
        <translation>No se puede crear la tabla</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="333"/>
        <source>Failed to create the output table </source>
        <translation>No se ha podido crear la tabla de salida </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="340"/>
        <source>Error connecting to the database</source>
        <translation>Error al conectarse a la base de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="346"/>
        <source>No GEOS support</source>
        <translation>No hay soporte para GEOS</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="347"/>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation>La función buffer requiere soporte GEOS en PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="356"/>
        <source>No Active Layer</source>
        <translation>Ninguna capa activa</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="357"/>
        <source>You must select a layer in the legend to buffer</source>
        <translation>Debe seleccionar una capa en la leyenda para crear el buffer</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="350"/>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation>No es una capa PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation>Crear un área de influencia de una capa PostgreSQL. </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation> no es una capa PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation>Las funciones de geoprocesamiento sólo están disponibles para capas PostgreSQL/PostGIS</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="83"/>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Tabla &lt;b&gt;%1&lt;/b&gt; en la base de datos &lt;b&gt;%2&lt;/b&gt; en el servidor &lt;b&gt;%3&lt;/b&gt;, usuario &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="67"/>
        <source>Connection Failed</source>
        <translation>Ha fallado la conexión</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="67"/>
        <source>Connection to the database failed:</source>
        <translation>Ha fallado la conexión a la base de datos:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="204"/>
        <source>Database error</source>
        <translation>Error de la base de datos</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="204"/>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;No se pudieron obtener muestras de los valores de los campos utilizando SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="279"/>
        <source>Query Result</source>
        <translation>Resultados de la consulta</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="281"/>
        <source>The where clause returned </source>
        <translation>La cláusula &quot;donde&quot; (WHERE) devolvió </translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="281"/>
        <source> rows.</source>
        <translation> filas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="285"/>
        <source>Query Failed</source>
        <translation>Ha fallado la consulta</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="287"/>
        <source>An error occurred when executing the query:</source>
        <translation>Se ha producido un error mientras se ejecutaba la consulta:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="340"/>
        <source>No Records</source>
        <translation>Sin registros</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="340"/>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>La consulta que ha especificado devuelve cero registros. Las capas válidas de PostgreSQL deben tener al menos un objeto espacial.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="268"/>
        <source>No Query</source>
        <translation>Ninguna consulta</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="268"/>
        <source>You must create a query before you can test it</source>
        <translation>Debe crear una consulta antes de probarlo</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="334"/>
        <source>Error in Query</source>
        <translation>Error en la consulta</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="21"/>
        <source>PostgreSQL Query Builder</source>
        <translation>Constructor de consultas de PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="328"/>
        <source>Clear</source>
        <translation>Limpiar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="338"/>
        <source>Test</source>
        <translation>Probar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="348"/>
        <source>Ok</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="358"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="83"/>
        <source>Values</source>
        <translation>Valores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="139"/>
        <source>All</source>
        <translation>Todos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="126"/>
        <source>Sample</source>
        <translation>Muestra</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="46"/>
        <source>Fields</source>
        <translation>Campos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="149"/>
        <source>Operators</source>
        <translation>Operadores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="167"/>
        <source>=</source>
        <translation>=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="209"/>
        <source>IN</source>
        <translation>EN</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="216"/>
        <source>NOT IN</source>
        <translation>NO EN</translation>
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
        <translation>COMO</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="195"/>
        <source>AND</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="244"/>
        <source>ILIKE</source>
        <translation>DISTINTO DE</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="188"/>
        <source>OR</source>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="181"/>
        <source>NOT</source>
        <translation>NO</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="274"/>
        <source>SQL where clause</source>
        <translation>Cláusula &quot;donde&quot; (WHERE) de SQL</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Recuperar &lt;span style=&quot; font-weight:600;&quot;&gt;todos&lt;/span&gt; los registros del archivo vectorial (&lt;span style=&quot; font-style:italic;&quot;&gt;si la tabla es grande, la operación puede llevar un poco de tiempo&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Tomar una &lt;span style=&quot; font-weight:600;&quot;&gt;muestra&lt;/span&gt; de los registros del archivo vectorial&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Lista de valores para el campo actual.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Lista de campos en este archivo vectorial&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="33"/>
        <source>Datasource</source>
        <translation>Origen de datos</translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="203"/>
        <source>No Plugins</source>
        <translation>No hay complementos</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="203"/>
        <source>No QGIS plugins found in </source>
        <translation>No se han econtrado complementos de QGIS en </translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="84"/>
        <source>&amp;Select All</source>
        <translation>&amp;Seleccionar todos</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="85"/>
        <source>&amp;Clear All</source>
        <translation>&amp;Limpiar todos</translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="16"/>
        <source>QGIS Plugin Manager</source>
        <translation>Administrador de complementos de QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="25"/>
        <source>To enable / disable a plugin, click its checkbox or description</source>
        <translation>Para activar /desactivar un complemento, pulse en su casilla o descripción</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="45"/>
        <source>&amp;Filter</source>
        <translation>&amp;Filtrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="58"/>
        <source>Plugin Directory:</source>
        <translation>Directorio de complementos:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="71"/>
        <source>Directory</source>
        <translation>Directorio</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="488"/>
        <source>Zoom In</source>
        <translation>Acercar zum</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="487"/>
        <source>z</source>
        <translation>z</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="493"/>
        <source>Zoom Out</source>
        <translation>Alejar zum</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="492"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="496"/>
        <source>Zoom To Layer</source>
        <translation>Zum a la capa</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="498"/>
        <source>Zoom to Layer</source>
        <translation>Hace zum a la capa</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="501"/>
        <source>Pan Map</source>
        <translation>Mover mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="502"/>
        <source>Pan the map</source>
        <translation>Mueve el mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="505"/>
        <source>Add Point</source>
        <translation>Añadir punto</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="506"/>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="507"/>
        <source>Capture Points</source>
        <translation>Capturar puntos</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="510"/>
        <source>Delete Point</source>
        <translation>Borrar punto</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="511"/>
        <source>Delete Selected</source>
        <translation>Borrar lo seleccionado</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="559"/>
        <source>Linear</source>
        <translation>Lineal</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="560"/>
        <source>Helmert</source>
        <translation>Helmert</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="198"/>
        <source>Choose a name for the world file</source>
        <translation>Seleccionar un nombre para el archivo de referenciación (world file)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="216"/>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given filename</comment>
        <translation>-modificado</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="265"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="280"/>
        <source>Affine</source>
        <translation>Afinidad</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="290"/>
        <source>Not implemented!</source>
        <translation>Sin implementar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="285"/>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Una transformación por afinidad requiere cambiar el archivo ráster original. Esto todavía no está soportado.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="292"/>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt; La transformación </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="293"/>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation> aún no está soportada.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="324"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="325"/>
        <source>Could not write to </source>
        <translation>No se pudo escribir </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="272"/>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>Actualmente todos los archivos modificados se escribirán en formato TIFF.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="271"/>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Una transformacioń Helmert requiere modificaciones en la capa ráster.&lt;/p&gt;&lt;p&gt;El ráster modificado se guardará en un nuevo archivo y se generará un archivo de referenciación para dicho archivo.&lt;/p&gt;&lt;p&gt;¿Está seguro de que es eso lo que quiere?&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="105"/>
        <source>Add points</source>
        <translation>Añadir puntos</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="130"/>
        <source>Delete points</source>
        <translation>Borrar puntos</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="178"/>
        <source>Zoom in</source>
        <translation>Acercar zum</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="200"/>
        <source>Zoom out</source>
        <translation>Alejar zum</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="222"/>
        <source>Zoom to the raster extents</source>
        <translation>Zum a la extensión del ráster</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="244"/>
        <source>Pan</source>
        <translation>Mover</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="45"/>
        <source>Modified raster:</source>
        <translation>Raster modificado:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="52"/>
        <source>World file:</source>
        <translation>Archivo de referenciación (world file):</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="65"/>
        <source>Transform type:</source>
        <translation>Tipo de transformación:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="13"/>
        <source>Reference points</source>
        <translation>Puntos de referencia</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="38"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="75"/>
        <source>Create</source>
        <translation>Crear</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="282"/>
        <source>Create and load layer</source>
        <translation>Crear y cargar capa</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="141"/>
        <source>Unable to access relation</source>
        <translation>No se puede acceder a la relación</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="121"/>
        <source>Unable to access the </source>
        <translation>No se puede acceder a la relación </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="144"/>
        <source> relation.
The error message from the database was:
</source>
        <translation>
El mensaje de error de la base de datos fue:
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="105"/>
        <source>No GEOS Support!</source>
        <translation>No hay soporte para GEOS</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="109"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>Su instalación de PostGIS no tiene soporte para GEOS.
La selección e identificación de objetos espaciales no funcionarán correctamente.
Instale PostGIS con soporte para GEOS (http://geos.refractions.net)
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="907"/>
        <source>No suitable key column in table</source>
        <translation>No hay una columna de clave adecuada</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="911"/>
        <source>The table has no column suitable for use as a key.

Qgis requires that the table either has a column of type
int4 with a unique constraint on it (which includes the
primary key) or has a PostgreSQL oid column.
</source>
        <translation>La tabla no tiene una columna adecuada para utilizar como clave.

Qgis necesita que la tabla tenga una columna de tipo
int4 con reserva única en ella (lo que incluye la
clave primaria) o que tenga una columna oid de PostgreSQL.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="952"/>
        <source>The unique index on column</source>
        <translation>El índice único en la columna</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="954"/>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation>no es adecuado porque actualmente Qgis no soporta columnas de tipo no-int4 como clave de la tabla.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="977"/>
        <source>and </source>
        <translation>y </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="983"/>
        <source>The unique index based on columns </source>
        <translation>El índice único basado en columnas </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="985"/>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation> no es adecuado porque actualmente Qgis no soporta columnas múltiples como clave de la tabla.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1028"/>
        <source>Unable to find a key column</source>
        <translation>No se puede encontrar una columna de clave</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1108"/>
        <source> derives from </source>
        <translation> deriva de </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1112"/>
        <source>and is suitable.</source>
        <translation>y es adecuada.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1116"/>
        <source>and is not suitable </source>
        <translation>y no es adecuada </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1117"/>
        <source>type is </source>
        <translation>es de tipo </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1119"/>
        <source> and has a suitable constraint)</source>
        <translation> y tiene la reserva adecuada)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1121"/>
        <source> and does not have a suitable constraint)</source>
        <translation> y no tiene la reserva adecuada)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1207"/>
        <source>Note: </source>
        <translation>Nota: </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1209"/>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation>inicialmente parecía adecuada pero no contiene datos únicos, por lo que no es aceptable.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1221"/>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation>La vista que ha seleccionado contiene las siguientes columnas, ninguna de las cuales satisface las condiciones anteriores:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1227"/>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation>Qgis necesita que la vista tenga una columna que pueda ser utilizada como clave única. Esta columna debería proceder de una columna de una tabla de tipo int4 y ser una clave primaria, tener una reserva única o ser una columna oid de PostgreSQL. Para un mejor funcionamiento la columna también debería estar indexada.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1228"/>
        <source>The view </source>
        <translation>La vista </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1229"/>
        <source>has no column suitable for use as a unique key.
</source>
        <translation>no tiene columnas adecuadas para usar como clave única.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1230"/>
        <source>No suitable key column in view</source>
        <translation>No hay una columna de clave adecuada en la vista</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2530"/>
        <source>Unknown geometry type</source>
        <translation>Tipo de geometría desconocido</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2531"/>
        <source>Column </source>
        <translation>Columna </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2541"/>
        <source> in </source>
        <translation> en </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2533"/>
        <source> has a geometry type of </source>
        <translation> tiene una geometría de tipo </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2533"/>
        <source>, which Qgis does not currently support.</source>
        <translation>, que Qgis no soporta actualmente.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2542"/>
        <source>. The database communication log was:
</source>
        <translation>. La registro de comunicación de la base de datos fue:
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2543"/>
        <source>Unable to get feature type and srid</source>
        <translation>No se ha podido obtener el tipo ni el srid del objeto espacial</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2540"/>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation>QGIS no pude determinar el tipo y srid de la columna </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="142"/>
        <source>Unable to determine table access privileges for the </source>
        <translation>No se pueden determinar los privilegios de acceso a tabla para el </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1869"/>
        <source>Error while adding features</source>
        <translation>Error al añadir objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1903"/>
        <source>Error while deleting features</source>
        <translation>Error al borrar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1935"/>
        <source>Error while adding attributes</source>
        <translation>Error al añadir atributos</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1974"/>
        <source>Error while deleting attributes</source>
        <translation>Error al borrar atributos</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2038"/>
        <source>Error while changing attributes</source>
        <translation>Error al cambiar atributos</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.h" line="495"/>
        <source>unexpected PostgreSQL error</source>
        <translation>error inesperado de PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2117"/>
        <source>Error while changing geometry values</source>
        <translation>Error al cambiar los valores de la geometría</translation>
    </message>
</context>
<context>
    <name>QgsProjectPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="13"/>
        <source>Project Properties</source>
        <translation>Propiedades del proyecto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="32"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="54"/>
        <source>Default project title</source>
        <translation>Título del proyecto por omisión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="51"/>
        <source>Descriptive project name</source>
        <translation>Nombre descriptivo del proyecto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="157"/>
        <source>Meters</source>
        <translation>Metros</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="167"/>
        <source>Feet</source>
        <translation>Pies</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="174"/>
        <source>Decimal degrees</source>
        <translation>Grados decimales</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="184"/>
        <source>Precision</source>
        <translation>Precisión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="190"/>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>Establece automáticamente el número de decimales en la visualización en la posición del ratón</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="193"/>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>El número de decimales usado cuando se visualiza en la posición del ratón se establece automaticamente de manera que un movimiento del ratón de un solo píxel cambia la posición de la visualización</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="196"/>
        <source>Automatic</source>
        <translation>Automática</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="209"/>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Estable el número de decimales a usar para la visualización en la posición del ratón</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="212"/>
        <source>Manual</source>
        <translation>Manual</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="222"/>
        <source>The number of decimal places for the manual option</source>
        <translation>Número de decimales para la opción manual</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="235"/>
        <source>decimal places</source>
        <translation>lugares decimales</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="251"/>
        <source>Digitizing</source>
        <translation>Digitalización</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="282"/>
        <source>Projection</source>
        <translation>Proyección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="300"/>
        <source>Enable on the fly projection</source>
        <translation>Activar proyección al vuelo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="257"/>
        <source>Enable topological editing</source>
        <translation>Activar edición topológica</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="271"/>
        <source>Snapping options...</source>
        <translation>Opciones de autoensamblado...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="264"/>
        <source>Avoid intersections of new polygons</source>
        <translation>Evitar intersecciones de nuevos polígonos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="38"/>
        <source>Title and colors</source>
        <translation>Título y colores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="44"/>
        <source>Project title</source>
        <translation>Título del proyecto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="61"/>
        <source>Selection color</source>
        <translation>Color de selección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="100"/>
        <source>Background color</source>
        <translation>Color de fondo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="139"/>
        <source>Map units</source>
        <translation>Unidades del mapa</translation>
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
        <translation>Selector de proyección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="52"/>
        <source>Projection</source>
        <translation>Proyección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="82"/>
        <source>Search</source>
        <translation>Buscar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="112"/>
        <source>Find</source>
        <translation>Encontrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="125"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="138"/>
        <source>QGIS SRSID</source>
        <translation>QGIS SRSID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="151"/>
        <source>EPSG ID</source>
        <translation>EPSG ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="164"/>
        <source>Postgis SRID</source>
        <translation>Postgis SRID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="221"/>
        <source>Spatial Reference System</source>
        <translation>Sistema de referencia espacial</translation>
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
        <translation>Consola de Python</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="58"/>
        <source>&gt;&gt;&gt;</source>
        <translation>&gt;&gt;&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="33"/>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation>Para acceder al entorno de Quantum GIS desde esta consola de python use objetos de ámbito global, lo que es una instancia de la clase QgisInterface.&lt;br&gt;Ejemplo de uso: iface.zoomFull()</translation>
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
        <translation> millas</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="869"/>
        <source> mile</source>
        <translation> milla</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="874"/>
        <source> inches</source>
        <translation> pulgadas</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="879"/>
        <source> foot</source>
        <translation> pie</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="883"/>
        <source> feet</source>
        <translation> pies</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="888"/>
        <source> degree</source>
        <translation> grado</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="890"/>
        <source> degrees</source>
        <translation> grados</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="893"/>
        <source> unknown</source>
        <translation> desconocido</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="242"/>
        <source>and all other files</source>
        <translation>y todos los demás archivos</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.h" line="513"/>
        <source>Not Set</source>
        <translation>No establecido</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2202"/>
        <source>Raster Extent: </source>
        <translation>Extensión del ráster: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2205"/>
        <source>Clipped area: </source>
        <translation>Área rectortada: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3425"/>
        <source>Driver:</source>
        <translation>Controlador:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3450"/>
        <source>Dataset Description</source>
        <translation>Descripción del conjunto de datos</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3502"/>
        <source>Dimensions:</source>
        <translation>Dimensiones:</translation>
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
        <translation> Bandas: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3513"/>
        <source>No Data Value</source>
        <translation>Valor sin datos</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3528"/>
        <source>Data Type:</source>
        <translation>Tipo de datos:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3534"/>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte - Entero natural de 8 bits</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3537"/>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 - Entero natural de 16 bits </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3540"/>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 - Número entero de 16 bits </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3543"/>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 - Entero natural de 32 bits </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3546"/>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 - Número entero de 32 bits </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3549"/>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 - Número de coma flotante de 32 bits </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3552"/>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 - Número de coma flotante de 64 bits </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3555"/>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 - Número complejo Int16 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3558"/>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 - Número complejo Int32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3561"/>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 - Número conplejo Float32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3564"/>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 - Número complejo Float64 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3567"/>
        <source>Could not determine raster data type.</source>
        <translation>No se pudo determinar el tipo de datos ráster.</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3572"/>
        <source>Pyramid overviews:</source>
        <translation>Pirámides:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3593"/>
        <source>Layer Spatial Reference System: </source>
        <translation>Sistema de referencia espacial de la capa: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3621"/>
        <source>Origin:</source>
        <translation>Origen:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3630"/>
        <source>Pixel Size:</source>
        <translation>Tamaño de píxel:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4793"/>
        <source>Band</source>
        <translation>Banda</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3655"/>
        <source>Band No</source>
        <translation>Número de banda</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3667"/>
        <source>No Stats</source>
        <translation>No hay estadísticas</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3670"/>
        <source>No stats collected yet</source>
        <translation>Todavía no se han recogido estadísticas</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3680"/>
        <source>Min Val</source>
        <translation>Valor mínimo</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3688"/>
        <source>Max Val</source>
        <translation>Valor máximo</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3696"/>
        <source>Range</source>
        <translation>Intervalo</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3704"/>
        <source>Mean</source>
        <translation>Media</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3712"/>
        <source>Sum of squares</source>
        <translation>Suma de cuadrados</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3720"/>
        <source>Standard Deviation</source>
        <translation>Desviación estándar</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3728"/>
        <source>Sum of all cells</source>
        <translation>Suma de todas las celdas</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3736"/>
        <source>Cell Count</source>
        <translation>Número de celdas</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3822"/>
        <source>Average Magphase</source>
        <translation>Fase magnética (Magphase) media</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3827"/>
        <source>Average</source>
        <translation>Media</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4749"/>
        <source>out of extent</source>
        <translation>fuera de la extensión</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4787"/>
        <source>null (no data)</source>
        <translation>nulo (sin datos)</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3522"/>
        <source>NoDataValue not set</source>
        <translation>No se ha establecido el valor sin datos (NoDataValue)</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3471"/>
        <source>Band %1</source>
        <translation>Banda %1</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1032"/>
        <source>Grayscale</source>
        <translation>Escala de grises</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2412"/>
        <source>Pseudocolor</source>
        <translation>Pseudocolor</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2412"/>
        <source>Freak Out</source>
        <translation>Alucinante</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="177"/>
        <source>Palette</source>
        <translation>Paleta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="825"/>
        <source>Columns: </source>
        <translation>Columnas: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="826"/>
        <source>Rows: </source>
        <translation>Filas: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="827"/>
        <source>No-Data Value: </source>
        <translation>Valor sin datos: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="827"/>
        <source>n/a</source>
        <translation>n/d</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1849"/>
        <source>Write access denied</source>
        <translation>Acceso de escritura denegado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1849"/>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation>Acceso de escritura denegado. Ajuste los permisos del archivo e inténtelo de nuevo.

</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1594"/>
        <source>Building pyramids failed.</source>
        <translation>Ha fallado la construcción de pirámides.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1590"/>
        <source>The file was not writeable. Some formats can not be written to, only read. You can also try to check the permissions and then try again.</source>
        <translation>El archivo no se puede escribir. Algunos formatos no se pueden escribir, sólo se pueden leer. También puede comprobar los permisos e intentarlo de nuevo.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1595"/>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation>La creación de pirámides no es soportada en este tipo de ráster.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1714"/>
        <source>Custom Colormap</source>
        <translation>Mapa de colores personalizado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2777"/>
        <source>No Stretch</source>
        <translation>Sin estiramiento</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2782"/>
        <source>Stretch To MinMax</source>
        <translation>Estirar a MinMax</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2787"/>
        <source>Stretch And Clip To MinMax</source>
        <translation>Estirar y unir a MinMax</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2792"/>
        <source>Clip To MinMax</source>
        <translation>Unir a MinMax</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1427"/>
        <source>Discrete</source>
        <translation>Discreto</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="907"/>
        <source>Linearly</source>
        <translation>Linealmente</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2622"/>
        <source>Equal interval</source>
        <translation>Intervalo igual</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2644"/>
        <source>Quantiles</source>
        <translation>Cuantiles</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="332"/>
        <source>Description</source>
        <translation>Descripción</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="333"/>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation>Las capas ráster de elevada resolución pueden ralentizar la navegación en QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="334"/>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation>Creando copias de menor resolución de los datos (pirámides) se puede mejorar el rendimiento de forma considerable, ya que QGIS selecciona la resolución más adecuada dependiendo del nivel de zum.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="335"/>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation>Debe tener permiso de escritura en el directorio donde están almacenados los datos originales para construir las pirámides.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="336"/>
        <source>Please note that building pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation>Nota: la construcción de pirámides puede alterar el archivo original de los datos y una vez creadas no podrán ser eliminadas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="337"/>
        <source>Please note that building pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation>Nota: la construcción de pirámides puede desvirtuar la imagen - ¡realice siempre una copia de seguridad antes!</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1821"/>
        <source>Red</source>
        <translation>Rojo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1821"/>
        <source>Green</source>
        <translation>Verde</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1821"/>
        <source>Blue</source>
        <translation>Azul</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1838"/>
        <source>Percent Transparent</source>
        <translation>Porcentaje transparente</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1834"/>
        <source>Gray</source>
        <translation>Gris</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1838"/>
        <source>Indexed Value</source>
        <translation>Valor indexado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2774"/>
        <source>User Defined</source>
        <translation>Definido por el usuario</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>No Scaling</source>
        <translation type="obsolete">Sin escalado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="819"/>
        <source>No-Data Value: Not Set</source>
        <translation>Valor sin datos: no establecido</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1806"/>
        <source>Save file</source>
        <translation>Guardar archivo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2308"/>
        <source>Textfile (*.txt)</source>
        <translation>Archivo de texto (*.txt)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1818"/>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation>Archivo de exportación de valores de píxel transparente generado por QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2308"/>
        <source>Open file</source>
        <translation>Abrir archivo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2381"/>
        <source>Import Error</source>
        <translation>Error de importación</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2381"/>
        <source>The following lines contained errors

</source>
        <translation>Las siguientes líneas contenían errores</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2386"/>
        <source>Read access denied</source>
        <translation>Acceso de lectura denegado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2386"/>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation>Acceso de lectura denegado. Ajuste los permisos de archivo y pruebe de nuevo.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2412"/>
        <source>Color Ramp</source>
        <translation>Rampa de colores</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="56"/>
        <source>Not Set</source>
        <translation>No establecido</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2999"/>
        <source>Default Style</source>
        <translation>Estilo predeterminado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2984"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Archivo de estilo de capa de QGIS (*.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3005"/>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3006"/>
        <source>Unknown style format: </source>
        <translation>Formato de estilo desconocido: </translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="13"/>
        <source>Raster Layer Properties</source>
        <translation>Propiedades de la capa ráster</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="32"/>
        <source>Symbology</source>
        <translation>Simbología</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="881"/>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Total&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="835"/>
        <source>None</source>
        <translation>Nada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1306"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1517"/>
        <source>Thumbnail</source>
        <translation>Miniatura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1570"/>
        <source>Legend:</source>
        <translation>Leyenda:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1623"/>
        <source>Palette:</source>
        <translation>Paleta:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1450"/>
        <source>Columns:</source>
        <translation>Columnas:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1457"/>
        <source>Rows:</source>
        <translation>Filas:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1464"/>
        <source>No Data:</source>
        <translation>Sin datos:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1380"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Escala máxima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1403"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Escala mínima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1342"/>
        <source>Change</source>
        <translation>Cambiar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1679"/>
        <source>Metadata</source>
        <translation>Metadatos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1701"/>
        <source>Pyramids</source>
        <translation>Pirámides</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1790"/>
        <source>Average</source>
        <translation>Media</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1795"/>
        <source>Nearest Neighbour</source>
        <translation>Vecino más próximo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1813"/>
        <source>Histogram</source>
        <translation>Histograma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1907"/>
        <source>Options</source>
        <translation>Opciones</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1862"/>
        <source>Chart Type</source>
        <translation>Tipo de gráfico</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1900"/>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="584"/>
        <source>Max</source>
        <translation>Máx</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="546"/>
        <source>Min</source>
        <translation>Mín</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="861"/>
        <source> 00%</source>
        <translation> 00%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="38"/>
        <source>Render as</source>
        <translation>Representar como</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1098"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1127"/>
        <source>Colormap</source>
        <translation>Mapa de color</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1217"/>
        <source>Delete entry</source>
        <translation>Borrar entrada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1224"/>
        <source>Classify</source>
        <translation>Clasificar</translation>
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
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="676"/>
        <source>Estimate (faster)</source>
        <translation>Estimar (más rápido)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="716"/>
        <source>Load</source>
        <translation>Cargar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="696"/>
        <source>Actual (slower)</source>
        <translation>Real (más lento)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="747"/>
        <source>Current</source>
        <translation>Actual</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="770"/>
        <source>Save current contrast enhancement algorithm as default. This setting will be persistent between QGIS sessions.</source>
        <translation>Guardar el algoritmo actual de mejora del contraste como predeterminado. Esta configuración se mantendrá entre sesiones de QGIS.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="773"/>
        <source>Saves current contrast enhancement algorithm as a default. This setting will be persistent between QGIS sessions.</source>
        <translation>Guarda la configuración actual del algoritmo de mejora del contraste como predeterminado. Esta configuración se mantendrá entre sesiones de QGIS.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="786"/>
        <source>Default</source>
        <translation>Predeterminado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="793"/>
        <source>TextLabel</source>
        <translation>EtiquetaDeTexto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="804"/>
        <source>Transparency</source>
        <translation>Transparencia</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="44"/>
        <source>Single band gray</source>
        <translation>Gris de una banda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="51"/>
        <source>Three band color</source>
        <translation>Color de tres bandas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="86"/>
        <source>RGB mode band selection and scaling</source>
        <translation>Selección de banda del modo RGB y escalado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="92"/>
        <source>Red band</source>
        <translation>Banda roja</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="124"/>
        <source>Green band</source>
        <translation>Banda verde</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="156"/>
        <source>Blue band</source>
        <translation>Banda azul</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="533"/>
        <source>Custom min / max values</source>
        <translation>Valores mínimo / máximo personalizados</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="201"/>
        <source>Red min</source>
        <translation>Mínimo de rojo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="239"/>
        <source>Red max</source>
        <translation>Máximo de rojo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="277"/>
        <source>Green min</source>
        <translation>Mínimo de verde</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="315"/>
        <source>Green max</source>
        <translation>Máximo de verde</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="353"/>
        <source>Blue min</source>
        <translation>Mínimo de azul</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="391"/>
        <source>Blue max</source>
        <translation>Máximo de azul</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="423"/>
        <source>Std. deviation</source>
        <translation>Desviación estándar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="462"/>
        <source>Single band properties</source>
        <translation>Propiedades de banda única</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="474"/>
        <source>Gray band</source>
        <translation>Banda gris</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="497"/>
        <source>Color map</source>
        <translation>Mapa de color</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="526"/>
        <source>Invert color map</source>
        <translation>Invertir mapa de color</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="616"/>
        <source>Use standard deviation</source>
        <translation>Usar desviación estándar</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Estimate note:</source>
        <translation type="obsolete">Estimar nota:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="670"/>
        <source>Load min / max values from band</source>
        <translation>Cargar valores min / max de la banda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="726"/>
        <source>Contrast enhancement</source>
        <translation>Mejora de contraste</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="810"/>
        <source>Global transparency</source>
        <translation>Transparencia global</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="891"/>
        <source>No data value</source>
        <translation>Valor de sin datos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="900"/>
        <source>Reset no data value</source>
        <translation>Restablecer valor de sin datos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="910"/>
        <source>Custom transparency options</source>
        <translation>Opciones de transparencia personalizada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="916"/>
        <source>Transparency band</source>
        <translation>Banda de transparencia</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Transparency layer;</source>
        <translation type="obsolete">Capa de transparencia:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="968"/>
        <source>Transparent pixel list</source>
        <translation>Lista de píxeles de transparentes</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1014"/>
        <source>Add values manually</source>
        <translation>Añadir valores manualmente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1030"/>
        <source>Add Values from display</source>
        <translation>Añadir valores desde la visualización</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1043"/>
        <source>Remove selected row</source>
        <translation>Eliminar la fila seleccionada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1056"/>
        <source>Default values</source>
        <translation>Valores predeterminados</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1082"/>
        <source>Import from file</source>
        <translation>Importar de archivo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1095"/>
        <source>Export to file</source>
        <translation>Exporta a archivo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1185"/>
        <source>Number of entries</source>
        <translation>Número de entradas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1267"/>
        <source>Color interpolation</source>
        <translation>Interpolación de color</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1293"/>
        <source>Classification mode</source>
        <translation>Modo de clasificación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1324"/>
        <source>Spatial reference system</source>
        <translation>Sistema de referencia espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1359"/>
        <source>Scale dependent visibility</source>
        <translation>Visibilidad dependiente de la escala</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1393"/>
        <source>Maximum</source>
        <translation>Máximo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1416"/>
        <source>Minimum</source>
        <translation>Mínimo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1429"/>
        <source>Show debug info</source>
        <translation>Mostrar información de depuración</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1473"/>
        <source>Layer source</source>
        <translation>Fuente de la capa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1493"/>
        <source>Display name</source>
        <translation>Nombre a visualizar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1723"/>
        <source>Pyramid resolutions</source>
        <translation>Resoluciones de pirámides</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1779"/>
        <source>Resampling method</source>
        <translation>Método de remuestreo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1803"/>
        <source>Build pyramids</source>
        <translation>Construir pirámides</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1880"/>
        <source>Line graph</source>
        <translation>Gráfico de líneas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1890"/>
        <source>Bar chart</source>
        <translation>Gráfico de barras</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1925"/>
        <source>Column count</source>
        <translation>Cuenta de columnas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1932"/>
        <source>Out of range OK?</source>
        <translation>¿Aceptar fuera de intervalo?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1939"/>
        <source>Allow approximation</source>
        <translation>Permitir aproximación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2002"/>
        <source>Restore Default Style</source>
        <translation>Restaurar estilo predeterminado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2009"/>
        <source>Save As Default</source>
        <translation>Guardar como predeterminado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2016"/>
        <source>Load Style ...</source>
        <translation>Cargar estilo...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2023"/>
        <source>Save Style ...</source>
        <translation>Guardar estilo...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="643"/>
        <source>Note:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1713"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRunProcess</name>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="146"/>
        <source>Unable to run command</source>
        <translation>No se puede ejecutar el comando</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="59"/>
        <source>Starting</source>
        <translation>Empezando</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="115"/>
        <source>Done</source>
        <translation>Hecho</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselect.cpp" line="219"/>
        <source> connection and all associated settings?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselect.cpp" line="220"/>
        <source>Confirm Delete</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished">Conexiones de servidor</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="49"/>
        <source>&amp;New</source>
        <translation type="unfinished">&amp;Nuevo</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="59"/>
        <source>Delete</source>
        <translation type="unfinished">Borrar</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="69"/>
        <source>Edit</source>
        <translation type="unfinished">Editar</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="95"/>
        <source>C&amp;onnect</source>
        <translation type="unfinished">Co&amp;nectar</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="108"/>
        <source>Offerings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="118"/>
        <source>Name</source>
        <translation type="unfinished">Nombre</translation>
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
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="78"/>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="81"/>
        <source>Tick Down</source>
        <translation>Marcas abajo</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Tick Up</source>
        <translation>Marcas arriba</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Bar</source>
        <translation>Barra</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Box</source>
        <translation>Cajetín</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="102"/>
        <source>&amp;Scale Bar</source>
        <translation>Barra de &amp;escala</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="103"/>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation>Crea una barra de escala que se muestra en la Vista del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="543"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Ilustraciones</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="164"/>
        <source> metres/km</source>
        <translation> metros/km</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="281"/>
        <source> feet</source>
        <translation> pies</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="288"/>
        <source> degrees</source>
        <translation> grados</translation>
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
        <translation> pie</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="286"/>
        <source> degree</source>
        <translation> grado</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="291"/>
        <source> unknown</source>
        <translation> desconocido</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="165"/>
        <source> feet/miles</source>
        <translation> pies/millas</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="262"/>
        <source> miles</source>
        <translation> millas</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="267"/>
        <source> mile</source>
        <translation> milla</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="272"/>
        <source> inches</source>
        <translation> pulgadas</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="300"/>
        <source>Scale Bar Plugin</source>
        <translation>Complemento de Barra de escala</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="76"/>
        <source>Click to select the colour</source>
        <translation>Pulsar para seleccionar el color</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="94"/>
        <source>Size of bar:</source>
        <translation>Tamaño de la barra:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="109"/>
        <source>Automatically snap to round number on resize</source>
        <translation>Redondear números automáticamente al cambiar de tamaño</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="127"/>
        <source>Colour of bar:</source>
        <translation>Color de la barra:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="143"/>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="148"/>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="158"/>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="174"/>
        <source>Enable scale bar</source>
        <translation>Activar barra de escala</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="192"/>
        <source>Scale bar style:</source>
        <translation>Estilo de la barra de escala:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="207"/>
        <source>Select the style of the scale bar</source>
        <translation>Seleccionar el estilo de la barra de escala</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="211"/>
        <source>Tick Down</source>
        <translation>Marcas abajo</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="216"/>
        <source>Tick Up</source>
        <translation>Marcas arriba</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="221"/>
        <source>Box</source>
        <translation>Cajetín</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="226"/>
        <source>Bar</source>
        <translation>Barra</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="254"/>
        <source>Placement:</source>
        <translation>Ubicación:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="274"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Este complemento dibuja una barra de escala sobre el mapa. Tenga en cuenta que la opción Tamaño indicada abajo es un tamaño &apos;preferido&apos; y puede que tenga que ser alterada por QGIS dependiendo del nivel de zum. El tamaño se establece de acuerdo con las unidades del mapa especificadas en las propiedades del proyecto.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="173"/>
        <source>No matching features found.</source>
        <translation>No se han encontrado objetos espaciales coincidentes.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="174"/>
        <source>Search results</source>
        <translation>Resultados de la búsqueda</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="183"/>
        <source>Search string parsing error</source>
        <translation>Error al analizar la cadena de búsqueda</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="239"/>
        <source>No Records</source>
        <translation>Ningún registro</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="239"/>
        <source>The query you specified results in zero records being returned.</source>
        <translation>La consulta que especificó no ha devuelto ningún registro.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="41"/>
        <source>Search query builder</source>
        <translation>Constructor de consultas de búsqueda</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="171"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="173"/>
        <source>Are you sure you want to remove the </source>
        <translation>¿Está seguro de que quiere eliminar la conexión </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="173"/>
        <source> connection and all associated settings?</source>
        <translation> y toda su configuración?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="174"/>
        <source>Confirm Delete</source>
        <translation>Confirmar borrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="399"/>
        <source>WMS Provider</source>
        <translation>Proveedor WMS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="401"/>
        <source>Could not open the WMS Provider</source>
        <translation>No se pudo conectar al proveedor WMS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="410"/>
        <source>Select Layer</source>
        <translation>Seleccionar capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="410"/>
        <source>You must select at least one layer first.</source>
        <translation>Primero debe seleccionar al menos una capa.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="414"/>
        <source>Coordinate Reference System</source>
        <translation>Sistema de coordenadas de referencia</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="414"/>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation>No hay un sistema de coordenadas de referencia para las capas seleccionadas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="679"/>
        <source>Could not understand the response.  The</source>
        <translation>La respuesta es ininteligible. El proveedor </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="680"/>
        <source>provider said</source>
        <translation> ha dicho</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="731"/>
        <source>WMS proxies</source>
        <translation>Proxy del servidor WMS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;p&gt;Several WMS servers have been added to the server list. Note that the proxy fields have been left blank and if you access the internet via a web proxy, you will need to individually set the proxy fields with appropriate values.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;Se han añadido varios servidores WMS la lista de servidores. Los campos proxy se han dejado en blanco y si accede a internet a través de un servidor proxy necesitará rellenar los campos del proxy de manera individual con valores apropiados.&lt;/p&gt;</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsserversourceselect.cpp" line="520"/>
        <source>Coordinate Reference System (%1 available)</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
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
        <translation>Añadir capa(s) de un servidor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="34"/>
        <source>Server Connections</source>
        <translation>Conexiones de servidor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="46"/>
        <source>Adds a few example WMS servers</source>
        <translation>Añadir unos cuantos servidores WMS de ejemplo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="52"/>
        <source>Add default servers</source>
        <translation>Añadir servidores por omisión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="81"/>
        <source>C&amp;onnect</source>
        <translation>Co&amp;nectar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="91"/>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="101"/>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="108"/>
        <source>&amp;New</source>
        <translation>&amp;Nuevo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="118"/>
        <source>Coordinate Reference System</source>
        <translation>Sistema de coordenadas de referencia</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="156"/>
        <source>Change ...</source>
        <translation>Cambiar...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="174"/>
        <source>Ready</source>
        <translation>Preparado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="187"/>
        <source>&amp;Add</source>
        <translation>&amp;Añadir</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="190"/>
        <source>Alt+A</source>
        <translation>Alt+A</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="203"/>
        <source>Layers</source>
        <translation>Capas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="230"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="235"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="240"/>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="245"/>
        <source>Abstract</source>
        <translation>Resumen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="270"/>
        <source>Image encoding</source>
        <translation>Codificación de la imagen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="296"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="299"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="309"/>
        <source>C&amp;lose</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="312"/>
        <source>Alt+L</source>
        <translation>Alt+C</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="419"/>
        <source>The database gave an error while executing this SQL:</source>
        <translation>La base de datos dio un error mientras ejecutaba esta SQL:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="427"/>
        <source>The error was:</source>
        <translation>El error fue:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="424"/>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation>... (cortado el resto de la SQL)</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="121"/>
        <source>Solid Line</source>
        <translation>Línea continua</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="122"/>
        <source>Dash Line</source>
        <translation>Línea de rayas</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="123"/>
        <source>Dot Line</source>
        <translation>Línea de puntos</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="124"/>
        <source>Dash Dot Line</source>
        <translation>Línea de rayas y puntos</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="125"/>
        <source>Dash Dot Dot Line</source>
        <translation>Línea de raya punto punto</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="126"/>
        <source>No Pen</source>
        <translation>Sin dibujo</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="145"/>
        <source>No Brush</source>
        <translation>Ningún pincel</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="131"/>
        <source>Solid</source>
        <translation>Sólido</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="132"/>
        <source>Horizontal</source>
        <translation>Horizontal</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="133"/>
        <source>Vertical</source>
        <translation>Vertical</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="134"/>
        <source>Cross</source>
        <translation>Cruz</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="135"/>
        <source>BDiagonal</source>
        <translation>Diagonal atrás</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="136"/>
        <source>FDiagonal</source>
        <translation>Diagonal adelante</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="137"/>
        <source>Diagonal X</source>
        <translation>Diagonal X</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="138"/>
        <source>Dense1</source>
        <translation>Denso 1</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="139"/>
        <source>Dense2</source>
        <translation>Denso 2</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="140"/>
        <source>Dense3</source>
        <translation>Denso 3</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="141"/>
        <source>Dense4</source>
        <translation>Denso 4</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="142"/>
        <source>Dense5</source>
        <translation>Denso 5</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="143"/>
        <source>Dense6</source>
        <translation>Denso 6</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="144"/>
        <source>Dense7</source>
        <translation>Denso 7</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="146"/>
        <source>Texture</source>
        <translation>Textura</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="19"/>
        <source>Single Symbol</source>
        <translation>Símbolo único</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="95"/>
        <source>Size</source>
        <translation>Tamaño</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="73"/>
        <source>Point Symbol</source>
        <translation>Símbolo de punto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="105"/>
        <source>Area scale field</source>
        <translation>Campo de escala de área</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="115"/>
        <source>Rotation field</source>
        <translation>Campo de rotación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="182"/>
        <source>Style Options</source>
        <translation>Opciones de estilo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="341"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="188"/>
        <source>Outline style</source>
        <translation>Estilo de línea exterior</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="220"/>
        <source>Outline color</source>
        <translation>Color de línea exterior</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="255"/>
        <source>Outline width</source>
        <translation>Anchura de línea exterior</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="278"/>
        <source>Fill color</source>
        <translation>Color de relleno</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="313"/>
        <source>Fill style</source>
        <translation>Estilo de relleno</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="45"/>
        <source>Label</source>
        <translation>Etiqueta</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="147"/>
        <source>to vertex</source>
        <translation>a vértice</translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="151"/>
        <source>to segment</source>
        <translation>a segmento</translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="89"/>
        <source>to vertex and segment</source>
        <translation>a vértice y segmento</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="13"/>
        <source>Snapping options</source>
        <translation>Opciones de autoensamblado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="26"/>
        <source>Layer</source>
        <translation>Capa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="31"/>
        <source>Mode</source>
        <translation>Modo</translation>
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
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>File Name</source>
        <translation>Nombre de archivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>Feature Class</source>
        <translation>Clase de objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Features</source>
        <translation>Objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>DB Relation Name</source>
        <translation>Nombre de la relación de la BD</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Schema</source>
        <translation>Esquema</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="148"/>
        <source>Are you sure you want to remove the [</source>
        <translation>¿Está seguro de que quiere eliminar la conexión [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="148"/>
        <source>] connection and all associated settings?</source>
        <translation>] y toda su configuración?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="149"/>
        <source>Confirm Delete</source>
        <translation>Confirmar borrado</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="172"/>
        <source>Add Shapefiles</source>
        <translation>Añadir archivos shape</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="174"/>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation>Archivos shape (*.shp);;Todos los archivos (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="268"/>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>Los siguientes archivos shape no se han podido cargar:

</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="272"/>
        <source>REASON: File cannot be opened</source>
        <translation>MOTIVO: el archivo no se puede abrir</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="277"/>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>MOTIVO: faltan uno o varios de los archivos del shape (*.dbf, *shx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="354"/>
        <source>General Interface Help:</source>
        <translation>Ayuda de la Interfaz general:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="356"/>
        <source>PostgreSQL Connections:</source>
        <translation>Conexiones PostgreSQL:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="358"/>
        <source>[New ...] - create a new connection</source>
        <translation>[Nueva...] - crear una conexión nueva</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="359"/>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation>[Editar...] - editar la conexión seleccionada actualmente</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="360"/>
        <source>[Remove] - remove the currently selected connection</source>
        <translation>[Eliminar] - eliminar la conexión seleccionada actualmente</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="361"/>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation>-debe seleccionar una conexión que funcione para poder importar archivos</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="362"/>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation>-cuando se cambian las conexiones el esquema global cambia en concordancia</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="363"/>
        <source>Shapefile List:</source>
        <translation>Lista de archivos shape:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="365"/>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation>[Añadir...] - abrir un cuadro de diálogo y buscar los archivos a importar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="366"/>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation>[Eliminar] - eliminar los archivos seleccionados de la lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="367"/>
        <source>[Remove All] - remove all the files in the list</source>
        <translation>[Eliminar todos] - eliminar todos los archivos de la lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="368"/>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] - ID de referencia para los archivos shape a importar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="369"/>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation>[Utilizar (SRID) por omisión] - establecer SRID a -1</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="370"/>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation>[Nombre de columna de la geometría] - nombre de la columna de la geometría en la base de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="371"/>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation>[Utilizar [Nombre de columna de la geometría] por omisión] - establecer el nombre de la columna a &quot;the_geom&quot;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="372"/>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation>[Esquema global] - establecer el esquema para todos los archivos a importar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="374"/>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[Importar] - importar los archivos shape actuales de la lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="375"/>
        <source>[Quit] - quit the program
</source>
        <translation>[Cerrar] - salir del programa
</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="376"/>
        <source>[Help] - display this help dialog</source>
        <translation>[Ayuda] - mostrar esta ayuda</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="833"/>
        <source>Import Shapefiles</source>
        <translation>Importar archivos shape</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="833"/>
        <source>You need to specify a Connection first</source>
        <translation>Primero debe especificar una conexión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="428"/>
        <source>Connection failed - Check settings and try again</source>
        <translation>La conexión ha fallado - Comprobar la configuración y probar de nuevo</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="467"/>
        <source>PostGIS not available</source>
        <translation>PostGIS no está disponible</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="469"/>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;La base de datos seleccionada no tiene instalado PostGIS, lo cual es necesario para almacenar datos espaciales.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="513"/>
        <source>You need to add shapefiles to the list first</source>
        <translation>Primero debe añadir archivos shape a la lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="580"/>
        <source>Importing files</source>
        <translation>Importando archivos</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="518"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="522"/>
        <source>Progress</source>
        <translation>Progreso</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="531"/>
        <source>Problem inserting features from file:</source>
        <translation>Problemas al insertar objetos espaciales del archivo:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="538"/>
        <source>Invalid table name.</source>
        <translation>Nombre de la tabla no válido.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="548"/>
        <source>No fields detected.</source>
        <translation>No se han detectado campos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="564"/>
        <source>Checking to see if </source>
        <translation>Comprobando si </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="573"/>
        <source>The following fields are duplicates:</source>
        <translation>Los siguientes campos están duplicados:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="814"/>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Error al ejecutar la SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="815"/>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation>&lt;/p&gt;&lt;p&gt;La base de datos ha dicho:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="672"/>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>Importar archivos shape - La relación existe</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="673"/>
        <source>The Shapefile:</source>
        <translation>El archivo shape:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>will use [</source>
        <translation>utilizará la relación [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>] relation for its data,</source>
        <translation>] para sus datos,</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>which already exists and possibly contains data.</source>
        <translation>que ya existe y posiblemente contenga datos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>Para evitar la pérdida de datos cambie el &quot;Nombre de la relación de la BD&quot;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation>para este archivo shape en la lista de archivos del diálogo principal.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>Do you want to overwrite the [</source>
        <translation>¿Quiere sobrescribir la relación [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>] relation?</source>
        <translation>]?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="829"/>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation>No se pudieron importar %1 de %2 archivos shape.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="408"/>
        <source>Password for </source>
        <translation>Contraseña para </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="409"/>
        <source>Please enter your password:</source>
        <translation>Por favor, introduzca su contraseña:</translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="19"/>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT - Herramienta para importar archivos shape a PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="172"/>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Añadir un archivo shape a la lista de archivos a importar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="175"/>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="185"/>
        <source>Remove the selected shapefile from the import list</source>
        <translation>Eliminar el archivo shape seleccionado de la lista de importación</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="188"/>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="198"/>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Eliminar todos los archivos shape de la lista de importación</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="201"/>
        <source>Remove All</source>
        <translation>Eliminar todos</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="224"/>
        <source>Set the SRID to the default value</source>
        <translation>Establecer el SRID al valor por omisión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="237"/>
        <source>Set the geometry column name to the default value</source>
        <translation>Establecer el nombre de la columna de la geometría al valor por omisión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="273"/>
        <source>Global Schema</source>
        <translation>Esquema global</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation>Conexiones PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="118"/>
        <source>Create a new PostGIS connection</source>
        <translation>Crear una nueva conexión a PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="121"/>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="105"/>
        <source>Remove the current PostGIS connection</source>
        <translation>Eliminar la conexión actual a PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="92"/>
        <source>Edit the current PostGIS connection</source>
        <translation>Editar la conexión actual a PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="95"/>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="144"/>
        <source>Import options and shapefile list</source>
        <translation>Opciones de importación y lista de archivos shape</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="227"/>
        <source>Use Default SRID or specify here</source>
        <translation>Usar SRID por omisión o especificar aquí</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="240"/>
        <source>Use Default Geometry Column Name or specify here</source>
        <translation>Usar nombre de columna de geometría por omisión o especificar aquí</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="263"/>
        <source>Primary Key Column Name</source>
        <translation>Nombre de la columna de clave primaria</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="131"/>
        <source>Connect to PostGIS</source>
        <translation>Conectar a PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="134"/>
        <source>Connect</source>
        <translation>Conectar</translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="68"/>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>&amp;Importar archivos shape a PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="70"/>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>Importar archivos shape a una base de datos PostgreSQL-PostGIS. El esquema y los nombres de los campos se pueden personalizar al importar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="93"/>
        <source>&amp;Spit</source>
        <translation>I&amp;mportar (Spit)</translation>
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
        <translation>Confirmar borrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsuniquevaluedialog.cpp" line="285"/>
        <source>The classification field was changed from &apos;%1&apos; to &apos;%2&apos;.
Should the existing classes be deleted before classification?</source>
        <translation>El campo de clasificación se cambió de &apos;%1&apos; a &apos;%2&apos;.
¿Deben borrarse las clases existentes antes de la clasificación?</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialogBase</name>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="19"/>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="93"/>
        <source>Classify</source>
        <translation>Clasificar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="49"/>
        <source>Classification field</source>
        <translation>Campo de clasificación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="100"/>
        <source>Add class</source>
        <translation>Añadir clase</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="107"/>
        <source>Delete classes</source>
        <translation>Borrar clases</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="114"/>
        <source>Randomize Colors</source>
        <translation>Colores aleatorios</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="121"/>
        <source>Reset Colors</source>
        <translation>Restablecer colores</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2470"/>
        <source>Could not commit the added features.</source>
        <translation>No se pudieron añadir los objetos espaciales.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2561"/>
        <source>No other types of changes will be committed at this time.</source>
        <translation>No se realizará ningún otro tipo de cambios en este momento.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2492"/>
        <source>Could not commit the changed attributes.</source>
        <translation>No se pudieron realizar los cambios en los atributos.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2551"/>
        <source>However, the added features were committed OK.</source>
        <translation>Sin embargo, se han añadido correctamente los objetos espaciales.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2518"/>
        <source>Could not commit the changed geometries.</source>
        <translation>No se pudieron realizar los cambios en la geometría.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2555"/>
        <source>However, the changed attributes were committed OK.</source>
        <translation>Sin embargo, se cambiaron correctamente los atributos.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2548"/>
        <source>Could not commit the deleted features.</source>
        <translation>No se pudieron borrar los objetos espaciales.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2559"/>
        <source>However, the changed geometries were committed OK.</source>
        <translation>Sin embargo, se cambiaron correctamente las geometrías.</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="113"/>
        <source>Transparency: </source>
        <translation>Transparencia: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="205"/>
        <source>Single Symbol</source>
        <translation>Símbolo único</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="208"/>
        <source>Graduated Symbol</source>
        <translation>Símbolo graduado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="209"/>
        <source>Continuous Color</source>
        <translation>Color graduado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="210"/>
        <source>Unique Value</source>
        <translation>Valor único</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="164"/>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation>Este botón abre el constructor de consultas de PostgreSQL y permite crear un subconjunto de objetos espaciales para mostrar en la vista del mapa, en vez de mostrarlos todos</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="167"/>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>Aquí se muestra la consulta usada para limitar los objetos espaciales de las capas. Esto actualmente sólo está soportado para capas PostgreSQL. Para introducir o modificar la consulta, pulse el botón Constructor de consultas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="398"/>
        <source>Spatial Index</source>
        <translation>Índice espacial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="393"/>
        <source>Creation of spatial index successfull</source>
        <translation>La creación del índice espacial ha sido correcta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="398"/>
        <source>Creation of spatial index failed</source>
        <translation>Ha fallado la creación del índice espacial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="411"/>
        <source>General:</source>
        <translation>General:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="426"/>
        <source>Storage type of this layer : </source>
        <translation>Tipo de almacenamiento de esta capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="432"/>
        <source>Source for this layer : </source>
        <translation>Fuente de esta capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="449"/>
        <source>Geometry type of the features in this layer : </source>
        <translation>Tipo de geometría de los objetos espaciales en esta capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="457"/>
        <source>The number of features in this layer : </source>
        <translation>Número de objetos espaciales en esta capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="462"/>
        <source>Editing capabilities of this layer : </source>
        <translation>Posibilidades de edición de esta capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="469"/>
        <source>Extents:</source>
        <translation>Extensión:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="474"/>
        <source>In layer spatial reference system units : </source>
        <translation>En unidades del sistema espacial de referencia de la capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="475"/>
        <source>xMin,yMin </source>
        <translation>xMín,yMín </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="479"/>
        <source> : xMax,yMax </source>
        <translation> : xMáx,yMáx </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="534"/>
        <source>In project spatial reference system units : </source>
        <translation>En unidades del sistema espacial de referencia del proyecto: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="508"/>
        <source>Layer Spatial Reference System:</source>
        <translation>Sistema de referencia espacial de la capa:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="545"/>
        <source>Attribute field info:</source>
        <translation>Información del campo del atributo:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="552"/>
        <source>Field</source>
        <translation>Campo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="555"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="558"/>
        <source>Length</source>
        <translation>Tamaño</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="561"/>
        <source>Precision</source>
        <translation>Precisión</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="419"/>
        <source>Layer comment: </source>
        <translation>Comentario de la capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="564"/>
        <source>Comment</source>
        <translation>Comentario</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="766"/>
        <source>Default Style</source>
        <translation>Estilo predeterminado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="748"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Archivo de estilo de capa de QGIS (*.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="772"/>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="773"/>
        <source>Unknown style format: </source>
        <translation>Formato de estilo desconocido: </translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="19"/>
        <source>Layer Properties</source>
        <translation>Propiedades de la capa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="38"/>
        <source>Symbology</source>
        <translation>Simbología</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="145"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="157"/>
        <source>Display name</source>
        <translation>Mostrar el nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="170"/>
        <source>Display field for the Identify Results dialog box</source>
        <translation>Campo para mostrar en el cuadro de diálogo de resultados de la identificación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="173"/>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation>Establece el campo que se mostrará con la herramienta de identificación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="176"/>
        <source>Display field</source>
        <translation>Mostrar campo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="186"/>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation>Use este control para indicar el campo que se situará en el nivel superior del cuadro de diálogo de resultados de la identificación.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="217"/>
        <source>Use scale dependent rendering</source>
        <translation>Utilizar represenación dependiente de la escala</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="258"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Escala mínima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="271"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Escala máxima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="200"/>
        <source>Create Spatial Index</source>
        <translation>Crear índice espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="287"/>
        <source>Subset</source>
        <translation>Subconjunto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="334"/>
        <source>Query Builder</source>
        <translation>Constructor de consultas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="345"/>
        <source>Metadata</source>
        <translation>Metadatos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="374"/>
        <source>Labels</source>
        <translation>Etiquetas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="386"/>
        <source>Display labels</source>
        <translation>Mostrar etiquetas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="410"/>
        <source>Actions</source>
        <translation>Acciones</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="453"/>
        <source>Restore Default Style</source>
        <translation>Restaurar estilo predeterminado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="460"/>
        <source>Save As Default</source>
        <translation>Guardar como predeterminado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="467"/>
        <source>Load Style ...</source>
        <translation>Cargar estilo...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="474"/>
        <source>Save Style ...</source>
        <translation>Guardar estilo...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="56"/>
        <source>Legend type</source>
        <translation>Tipo de leyenda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="94"/>
        <source>Transparency</source>
        <translation>Transparencia</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="151"/>
        <source>Options</source>
        <translation>Opciones</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="207"/>
        <source>Change SRS</source>
        <translation>Cambiar sistema de referencia espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="238"/>
        <source>Maximum</source>
        <translation>Máximo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="248"/>
        <source>Minimum</source>
        <translation>Mínimo</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="59"/>
        <source>&amp;Add WFS layer</source>
        <translation>&amp;Añadir capa WFS</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1391"/>
        <source>unknown</source>
        <translation>desconocido</translation>
    </message>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1397"/>
        <source>received %1 bytes from %2</source>
        <translation>recibidos %1 bytes de %2</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="260"/>
        <source>Are you sure you want to remove the </source>
        <translation>¿Está seguro de que quiere eleminar la conexión </translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="260"/>
        <source> connection and all associated settings?</source>
        <translation> y toda su configuración?</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="261"/>
        <source>Confirm Delete</source>
        <translation>Confirmar borrado</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="29"/>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="34"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="39"/>
        <source>Abstract</source>
        <translation>Resumen</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="47"/>
        <source>Coordinate Reference System</source>
        <translation>Sistema de coordenadas de referencia</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="85"/>
        <source>Change ...</source>
        <translation>Cambiar...</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="95"/>
        <source>Server Connections</source>
        <translation>Conexiones de servidor</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="107"/>
        <source>&amp;New</source>
        <translation>&amp;Nuevo</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="117"/>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="127"/>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="153"/>
        <source>C&amp;onnect</source>
        <translation>Co&amp;nectar</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="13"/>
        <source>Add WFS Layer from a Server</source>
        <translation>Añadir capa WFS desde un servidor</translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="706"/>
        <source>Tried URL: </source>
        <translation>URL probada: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="686"/>
        <source>HTTP Exception</source>
        <translation>Excepción HTTP</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="649"/>
        <source>WMS Service Exception</source>
        <translation>Excepción del servicio WMS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1500"/>
        <source>DOM Exception</source>
        <translation>Excepción DOM</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="760"/>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>No se pudieron obtener las capacidades del WMS: %1 en la línea %2 columna %3</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="791"/>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>Probablemente se deba a una URL incorrecta del servidor WMS.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="787"/>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>No se pudieron obtener las capacidades del WMS en el formato esperado (DTD): no se ha encontrado %1 o %2</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1502"/>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>No se pudo obtener una excepción del servicio WMS en %1: %2 en la línea %3 columna %4</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1552"/>
        <source>Request contains a Format not offered by the server.</source>
        <translation>La solicitud contiene un formato no ofrecido por el servidor.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1556"/>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>La solicitud contiene un CRS no ofrecido por el servidor para una o más de las capas solicitadas.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1560"/>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>La solicitud contiene un SRS no ofrecido por el servidor para una o más de las capas solicitadas.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1565"/>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation>La solicitud de obtención de mapa (GetMap) es para una capa no ofrecida por el servidor o la solicitud de información del tema (GetFeatureInfo) es para una capa que no mostrada en el mapa.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1569"/>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>La solicitud es para una capa en un estilo no ofrecido por el servidor.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1573"/>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>La solicitud de información del tema (GetFeatureInfo) se aplica a una capa no declarada consultable.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1577"/>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>La solicitud de información del tema (GetFeatureInfo) contiene valores no válidos de X o Y.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1582"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation>El valor del parámetro (opcional) UpdateSequence en la consulta GetCapabilities es igual al valor actual del número de secuencia del servicio de actualización de los metadatos.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1587"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation>El valor del parámetro (opcional) UpdateSequence en la consulta GetCapabilities es mayor que el valor actual del número de secuencia del servicio de actualización de los metadatos.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1592"/>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation>La solicitud no incluye un valor de muestra para la dimensión y el servidor no ha declarado un valor por omisión para esa dimensión.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1596"/>
        <source>Request contains an invalid sample dimension value.</source>
        <translation>La solicitud contiene un valor de muestra para la dimensión no válido.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1600"/>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>La solicitud es para una operación opcional no soportada por el servidor.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1604"/>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(Código de error desconocido de un servidor post-1.3 WMS)</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1607"/>
        <source>The WMS vendor also reported: </source>
        <translation>El productor WMS también informó: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1610"/>
        <source>This is probably due to a bug in the QGIS program.  Please report this error.</source>
        <translation>Esto probablemente se deba a un fallo en el programa QGIS. Por favor, informe de este error.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1794"/>
        <source>Server Properties:</source>
        <translation>Propiedades del servidor:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1925"/>
        <source>Property</source>
        <translation>Propiedad</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1928"/>
        <source>Value</source>
        <translation>Valor</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1811"/>
        <source>WMS Version</source>
        <translation>Versión WMS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2057"/>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2065"/>
        <source>Abstract</source>
        <translation>Resumen</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1835"/>
        <source>Keywords</source>
        <translation>Palabras clave</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1843"/>
        <source>Online Resource</source>
        <translation>Recursos en línea</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1851"/>
        <source>Contact Person</source>
        <translation>Persona de contacto</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1863"/>
        <source>Fees</source>
        <translation>Cuotas</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1871"/>
        <source>Access Constraints</source>
        <translation>Restricciones de acceso</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1879"/>
        <source>Image Formats</source>
        <translation>Formatos de imagen</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1887"/>
        <source>Identify Formats</source>
        <translation>Formatos de identificación</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1895"/>
        <source>Layer Count</source>
        <translation>Número de capas</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1915"/>
        <source>Layer Properties: </source>
        <translation>Propiedades de la capa: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1933"/>
        <source>Selected</source>
        <translation>Seleccionado</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1990"/>
        <source>Yes</source>
        <translation>Sí</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1990"/>
        <source>No</source>
        <translation>No</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1942"/>
        <source>Visibility</source>
        <translation>Visibilidad</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1948"/>
        <source>Visible</source>
        <translation>Visible</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1949"/>
        <source>Hidden</source>
        <translation>Oculta</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1950"/>
        <source>n/a</source>
        <translation>n/d</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1971"/>
        <source>Can Identify</source>
        <translation>Se puede identificar</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1979"/>
        <source>Can be Transparent</source>
        <translation>Puede ser transparente</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1987"/>
        <source>Can Zoom In</source>
        <translation>Se puede acercar el zum</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1995"/>
        <source>Cascade Count</source>
        <translation>Cuenta en cascada</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2003"/>
        <source>Fixed Width</source>
        <translation>Anchura fija</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2011"/>
        <source>Fixed Height</source>
        <translation>Altura fija</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2019"/>
        <source>WGS 84 Bounding Box</source>
        <translation>Marco de la WGS 84</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2029"/>
        <source>Available in CRS</source>
        <translation>Disponible en CRS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2040"/>
        <source>Available in style</source>
        <translation>Disponible en estilo</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2049"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2150"/>
        <source>Layer cannot be queried.</source>
        <translation>La capa no se puede consultar.</translation>
    </message>
</context>
<context>
    <name>QuickPrintGui</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="129"/>
        <source>Portable Document Format (*.pdf)</source>
        <translation>Formato de documento portátil (*.pdf)</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="154"/>
        <source>quickprint</source>
        <translation>impresión rápida</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="155"/>
        <source>Unknown format: </source>
        <translation>Formato desconocido: </translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="13"/>
        <source>QGIS Quick Print Plugin</source>
        <translation>Complemento de impresión rápida de QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="158"/>
        <source>Quick Print</source>
        <translation>Impresión rápida</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="129"/>
        <source>Map Title e.g. ACME inc.</source>
        <translation>Título del mapa, ej. ACME inc.</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="116"/>
        <source>Map Name e.g. Water Features</source>
        <translation>Nombre del mapa, ej. Elementos acuáticos</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="103"/>
        <source>Copyright</source>
        <translation>Copyright</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="48"/>
        <source>Output</source>
        <translation>Salida</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="60"/>
        <source>Use last filename but incremented.</source>
        <translation>Usar el último nombre de archivo, pero incrementado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="67"/>
        <source>last used filename but incremented will be shown here</source>
        <translation>el último nombre de archivo usado incrementado se mostrará aquí</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="77"/>
        <source>Prompt for file name</source>
        <translation>Preguntar nombre de archivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="38"/>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation>Nota: si quiere más control sobre la disposición del mapa, por favor use la función Diseñador de mapas de QGIS.</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="93"/>
        <source>Page Size</source>
        <translation>Tamaño de página</translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="75"/>
        <source>Quick Print</source>
        <translation>Impresión rápida</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="77"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation>Remplazar con una breve descripción de lo que hace el complemento</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="82"/>
        <source>&amp;Quick Print</source>
        <translation>Impresión &amp;rápida</translation>
    </message>
</context>
<context>
    <name>RepositoryDetailsDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="13"/>
        <source>Repository details</source>
        <translation>Detalles del repositorio</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="19"/>
        <source>Name:</source>
        <translation>Nombre:</translation>
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
        <translation>Plantilla de complementos de QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/pluginguibase.ui" line="47"/>
        <source>Plugin Template</source>
        <translation>Plantilla de complementos</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="75"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation>Reemplazar con una breve descripción de lo que hace el complemento</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="73"/>
        <source>[menuitemname]</source>
        <translation>[nombredeelementodemenu]</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="80"/>
        <source>&amp;[menuname]</source>
        <translation>&amp;{nombremenu]</translation>
    </message>
</context>
</TS>
