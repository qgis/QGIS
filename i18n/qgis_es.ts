<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1">
<context>
    <name>@default</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1642"/>
        <source>OGR Driver Manager</source>
        <translation>Administrador del controlador OGR</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1642"/>
        <source>unable to get OGRDriverManager</source>
        <translation>No se puede conseguir un administrador del controlador OGR</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="13"/>
        <source>QGIS Plugin Installer</source>
        <translation>Instalador de complementos de QGIS</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="23"/>
        <source>Retrieve the list of available plugins, select one and install it</source>
        <translation>Obtener la lista de complementos disponibles, seleccionar uno e instalarlo</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="77"/>
        <source>Name of plugin to install</source>
        <translation>Nombre del complemento a instalar</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="36"/>
        <source>Get List</source>
        <translation>Obtener lista</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="111"/>
        <source>Done</source>
        <translation>Hecho</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="87"/>
        <source>Install Plugin</source>
        <translation>Instalar complemento</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="98"/>
        <source>The plugin will be installed to ~/.qgis/python/plugins</source>
        <translation>El complemento se instalará en ~/.qgis/python/plugins</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="52"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="57"/>
        <source>Version</source>
        <translation>Versión</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="62"/>
        <source>Description</source>
        <translation>Descripción</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="67"/>
        <source>Author</source>
        <translation>Autor</translation>
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
    <name>QObject</name>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="311"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Capturada una excepción del sistema de coordenadas mientras se intentaba transformar un punto.No se puede calcular la longitud de la línea.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="402"/>
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
        <location filename="../src/app/qgisapp.cpp" line="2907"/>
        <source>QGis files (*.qgs)</source>
        <translation>Archivos QGis (*.qgs)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="75"/>
        <source>No active layer</source>
        <translation>No hay capa activa</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="153"/>
        <source>Band</source>
        <translation>Banda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="319"/>
        <source>Length</source>
        <translation>Longitud</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="325"/>
        <source>Area</source>
        <translation>Área</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="332"/>
        <source>action</source>
        <translation>acción</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="341"/>
        <source> features found</source>
        <translation> objetos espaciales encontrados</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="345"/>
        <source> 1 feature found</source>
        <translation> 1 objeto espacial encontrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="351"/>
        <source>No features found</source>
        <translation>No se han encontrado objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="351"/>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>No se han encontrado objetos espaciales en la capa activa en el punto en el que se ha pinchado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="451"/>
        <source>Could not identify objects on</source>
        <translation>No se pudieron identificar objetos en</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="451"/>
        <source>because</source>
        <translation>porque</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="219"/>
        <source>Could not snap vertex. Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>No se pudieron autoensamblar los vértices. ¿Ha establecido la tolerancia en Configuración &gt; Propiedades del proyecto &gt; General?</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="769"/>
        <source>Project file read error: </source>
        <translation>Error de lectura del archivo del proyecto: </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="769"/>
        <source> at line </source>
        <translation> en la línea </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="770"/>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="776"/>
        <source> for file </source>
        <translation> en el archivo </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="944"/>
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
        <location filename="../src/providers/gpx/gpsdata.cpp" line="330"/>
        <source>Couldn&apos;t open the data source: </source>
        <translation>No se pudo abrir la fuente de datos: </translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="352"/>
        <source>Parse error at line </source>
        <translation>Error de análisis en la línea </translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="53"/>
        <source>GPS eXchange format provider</source>
        <translation>Proveedor de formato eXchange GPS</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="156"/>
        <source>GRASS plugin</source>
        <translation>Complemento de GRASS</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="129"/>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>QGIS no pudo encontrar su instalación de GRASS.
¿Podría especificar la ruta (GISBASE) de su instalación de GRASS?</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="143"/>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Seleccionar ruta de instalación de GRASS (GISBASE)</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="157"/>
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
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="30"/>
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
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="54"/>
        <source>GPS Tools</source>
        <translation>Herramientas de GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="56"/>
        <source>Tools for loading and importing GPS data</source>
        <translation>Herramientas para cargar e importar datos de GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="842"/>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="848"/>
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
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="58"/>
        <source>NorthArrow</source>
        <translation>Flecha de Norte</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="59"/>
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
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="28"/>
        <source>WFS plugin</source>
        <translation>Complemento de WFS</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="29"/>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>Añade capas WFS a la vista de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="391"/>
        <source>Not a vector layer</source>
        <translation>No es una capa vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="392"/>
        <source>The current layer is not a vector layer</source>
        <translation>La capa actual no es vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="69"/>
        <source>Layer cannot be added to</source>
        <translation>La capa no se puede añadir a</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="70"/>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>El proveedor de datos de esta capa no da soporte para añadir objetos espaciales.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="405"/>
        <source>Layer not editable</source>
        <translation>Capa no editable</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="51"/>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>No se puede editar la capa vectorial. Para hacerla editable vaya al archivo de la capa, haga clic derecho y marque &apos;Permitir edición&apos;.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="76"/>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Para seleccionar objetos espaciales debe elegir una capa vectorial haciendo clic en su nombre en la leyenda</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="400"/>
        <source>Python error</source>
        <translation>Error de Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="57"/>
        <source>Couldn&apos;t load SIP module.
Python support will be disabled.</source>
        <translation>No se pudo cargar el módulo SIP.
El soporte para Python estará deshabilitado.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="67"/>
        <source>Couldn&apos;t load PyQt bindings.
Python support will be disabled.</source>
        <translation>No se pudieron cargar las conexiones de PyQt.
El soporte para Python estará deshabilitado.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="78"/>
        <source>Couldn&apos;t load QGIS bindings.
Python support will be disabled.</source>
        <translation>No se pudieron cargar las conexiones de QGIS.
El soporte para Python estará deshabilitado.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="380"/>
        <source>Couldn&apos;t load plugin </source>
        <translation>No se pudo cargar el complemento </translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="369"/>
        <source> due an error when calling its classFactory() method</source>
        <translation> debido a un error al llamar a su método classFactory()</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="381"/>
        <source> due an error when calling its initGui() method</source>
        <translation> debido a un error al llamar a su método initGui()</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="401"/>
        <source>Error while unloading plugin </source>
        <translation>Error al descargar complemento </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="56"/>
        <source>2.5D shape type not supported</source>
        <translation>El tipo shape 2.5D no está soportado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="56"/>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>Añadir objetos espaciales a tipo shape 2.5 aún no está soportado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="194"/>
        <source>Wrong editing tool</source>
        <translation>Herramienta de edición incorrecta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="92"/>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>No se puede aplicar la herramienta &apos;capturar punto&apos; en esta capa vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="65"/>
        <source>Coordinate transform error</source>
        <translation>Error de transformación de coordenadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="66"/>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>No se puede transformar el punto al sistema de coordenadas de las capas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="187"/>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>No se puede aplicar la herramienta &apos;capturar línea&apos; en esta capa vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="195"/>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>No se puede aplicar la herramienta &apos;capturar polígono&apos; en esta capa vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="218"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="395"/>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>No se puede añadir el objeto espacial. Tipo WKB desconocido.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdisland.cpp" line="110"/>
        <source>Error, could not add island</source>
        <translation>Error, no se pudo añadir la isla</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="90"/>
        <source>A problem with geometry type occured</source>
        <translation>Ocurrió un problema con el tipo de geometría</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="94"/>
        <source>The inserted Ring is not closed</source>
        <translation>El anillo insertado no está cerrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="98"/>
        <source>The inserted Ring is not a valid geometry</source>
        <translation>El anillo insertado no es una geometría válida</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="102"/>
        <source>The inserted Ring crosses existing rings</source>
        <translation>El anillo insertado cruza anillos existentes</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="106"/>
        <source>The inserted Ring is not contained in a feature</source>
        <translation>El anillo insertado no está contenido en un objeto espacial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="110"/>
        <source>An unknown error occured</source>
        <translation>Ocurrió un error desconocido</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="112"/>
        <source>Error, could not add ring</source>
        <translation>Error, no se pudo añadir el anillo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="398"/>
        <source>Change geometry</source>
        <translation>Cambiar geometría</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="399"/>
        <source>Data provider of the current layer doesn&apos;t allow changing geometries</source>
        <translation>El proveedor de datos de la capa actual no permite cambiar las geometrías</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="406"/>
        <source>Cannot edit the vector layer. Use &apos;Start editing&apos; in the legend item menu</source>
        <translation>No se puede editar la capa vectorial. Use &apos;Comenzar edición&apos; en el menú del elemento en la leyenda.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="648"/>
        <source> km2</source>
        <translation> km²</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="653"/>
        <source> ha</source>
        <translation> Ha</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="658"/>
        <source> m2</source>
        <translation> m²</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="686"/>
        <source> m</source>
        <translation> m</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="671"/>
        <source> km</source>
        <translation> km</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="676"/>
        <source> mm</source>
        <translation> mm</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="681"/>
        <source> cm</source>
        <translation> cm</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="695"/>
        <source> sq mile</source>
        <translation> milla²</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="700"/>
        <source> sq ft</source>
        <translation> pies²</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="707"/>
        <source> mile</source>
        <translation> milla</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="713"/>
        <source> foot</source>
        <translation> pie</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="715"/>
        <source> feet</source>
        <translation> pies</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="722"/>
        <source> sq.deg.</source>
        <translation> grados²</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="727"/>
        <source> degree</source>
        <translation> grado</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="729"/>
        <source> degrees</source>
        <translation> grados</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="733"/>
        <source> unknown</source>
        <translation> desconocido</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="269"/>
        <source>Received %1 of %2 bytes</source>
        <translation>Recibidos %1 de %2 bytes</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="275"/>
        <source>Received %1 bytes (total unknown)</source>
        <translation>Recibidos %1 bytes (total desconocido)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="386"/>
        <source>Not connected</source>
        <translation>No conectado</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="392"/>
        <source>Looking up &apos;%1&apos;</source>
        <translation>Buscando &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="399"/>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>Conectando a &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="406"/>
        <source>Sending request &apos;%1&apos;</source>
        <translation>Enviando petición &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="413"/>
        <source>Receiving reply</source>
        <translation>Recibiendo respuesta</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="419"/>
        <source>Response is complete</source>
        <translation>La respuesta está completa</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="425"/>
        <source>Closing down connection</source>
        <translation>Cerrando la conexión</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="751"/>
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
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="136"/>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Localización: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="136"/>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Directorio de mapas: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="140"/>
        <source>Location: </source>
        <translation>Localización: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="140"/>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Directorio de mapas: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="145"/>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation>&lt;b&gt;Ráster&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="154"/>
        <source>Cannot open raster header</source>
        <translation>No se puede abrir la cabecera del ráster</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="158"/>
        <source>Rows</source>
        <translation>Filas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="159"/>
        <source>Columns</source>
        <translation>Columnas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="160"/>
        <source>N-S resolution</source>
        <translation>Resolución N-S</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="161"/>
        <source>E-W resolution</source>
        <translation>Resolución E-W</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="299"/>
        <source>North</source>
        <translation>Norte</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="301"/>
        <source>South</source>
        <translation>Sur</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="303"/>
        <source>East</source>
        <translation>Este</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="305"/>
        <source>West</source>
        <translation>Oeste</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="188"/>
        <source>Format</source>
        <translation>Formato</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="199"/>
        <source>Minimum value</source>
        <translation>Valor mínimo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="200"/>
        <source>Maximum value</source>
        <translation>Valor máximo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="211"/>
        <source>Data source</source>
        <translation>Fuente de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="216"/>
        <source>Data description</source>
        <translation>Descripción de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="225"/>
        <source>Comments</source>
        <translation>Comentarios</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="240"/>
        <source>Categories</source>
        <translation>Categorías</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="346"/>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;Vectorial&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="273"/>
        <source>Points</source>
        <translation>Puntos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="274"/>
        <source>Lines</source>
        <translation>Líneas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="275"/>
        <source>Boundaries</source>
        <translation>Contornos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="276"/>
        <source>Centroids</source>
        <translation>Centroides</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="279"/>
        <source>Faces</source>
        <translation>Caras</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="280"/>
        <source>Kernels</source>
        <translation>Kernels</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="283"/>
        <source>Areas</source>
        <translation>Áreas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="284"/>
        <source>Islands</source>
        <translation>Islas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="308"/>
        <source>Top</source>
        <translation>Arriba</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="309"/>
        <source>Bottom</source>
        <translation>Abajo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="312"/>
        <source>yes</source>
        <translation>sí</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="312"/>
        <source>no</source>
        <translation>no</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="319"/>
        <source>History&lt;br&gt;</source>
        <translation>Historia&lt;br&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="347"/>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Capa&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="366"/>
        <source>Features</source>
        <translation>Objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="375"/>
        <source>Driver</source>
        <translation>Controlador</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="376"/>
        <source>Database</source>
        <translation>Base de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="377"/>
        <source>Table</source>
        <translation>Tabla</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="378"/>
        <source>Key column</source>
        <translation>Columna clave</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="413"/>
        <source>GISBASE is not set.</source>
        <translation>GISBASE no establecida.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="418"/>
        <source> is not a GRASS mapset.</source>
        <translation> no es un directorio de mapas de GRASS.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="440"/>
        <source>Cannot start </source>
        <translation>No se puede iniciar </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="457"/>
        <source>Mapset is already in use.</source>
        <translation>El directorio de mapas ya está en uso.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="472"/>
        <source>Temporary directory </source>
        <translation>El directorio temporal </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="472"/>
        <source> exist but is not writable</source>
        <translation> existe pero no se puede escribir</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="478"/>
        <source>Cannot create temporary directory </source>
        <translation>No se puede crear el directorio temporal </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="494"/>
        <source>Cannot create </source>
        <translation>No se puede crear </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="567"/>
        <source>Cannot remove mapset lock: </source>
        <translation>No se puede eliminar el bloqueo del directorio de mapas: </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1007"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="955"/>
        <source>Cannot read raster map region</source>
        <translation>No se puede la región del mapa ráster</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="972"/>
        <source>Cannot read vector map region</source>
        <translation>No se puede leer la región del mapa vectorial</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1008"/>
        <source>Cannot read region</source>
        <translation>No se puede leer la región</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2337"/>
        <source>Where is &apos;</source>
        <translation>¿Dónde está &apos;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2337"/>
        <source>original location: </source>
        <translation>localización original: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="122"/>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Para identificar objetos espaciales, debe activar una capa haciendo clic en su nombre en la leyenda</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="47"/>
        <source>PostgreSQL Geoprocessing</source>
        <translation>Geoprocesamiento PostgreSQL</translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="331"/>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS - </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="300"/>
        <source>Checking database</source>
        <translation>Comprobando la base de datos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="306"/>
        <source>Reading settings</source>
        <translation>Leyendo configuraciones</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="310"/>
        <source>Setting up the GUI</source>
        <translation>Configurando la interfaz gráfica de usuario (GUI)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="359"/>
        <source>Restoring loaded plugins</source>
        <translation>Restableciendo complementos cargados</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="363"/>
        <source>Initializing file filters</source>
        <translation>Inicializando filtros de archivo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="388"/>
        <source>Restoring window state</source>
        <translation>Restableciendo el estado de la ventana</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="392"/>
        <source>QGIS Ready!</source>
        <translation>¡QGIS preparado!</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="454"/>
        <source>&amp;New Project</source>
        <translation>&amp;Nuevo proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="455"/>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="456"/>
        <source>New Project</source>
        <translation>Nuevo proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="459"/>
        <source>&amp;Open Project...</source>
        <translation>&amp;Abrir proyecto...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="460"/>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation>Ctrl+A</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="461"/>
        <source>Open a Project</source>
        <translation>Abrir un proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="464"/>
        <source>&amp;Save Project</source>
        <translation>&amp;Guardar proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="465"/>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation>Ctrl+G</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="466"/>
        <source>Save Project</source>
        <translation>Guardar proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="469"/>
        <source>Save Project &amp;As...</source>
        <translation>G&amp;uardar proyecto como...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="470"/>
        <source>Ctrl+A</source>
        <comment>Save Project under a new name</comment>
        <translation>Ctrl+U</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="471"/>
        <source>Save Project under a new name</source>
        <translation>Guardar proyecto con un nombre nuevo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="474"/>
        <source>&amp;Print...</source>
        <translation>Im&amp;primir...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="475"/>
        <source>Ctrl+P</source>
        <comment>Print</comment>
        <translation>Ctrl+P</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="476"/>
        <source>Print</source>
        <translation>Imprimir</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="479"/>
        <source>Save as Image...</source>
        <translation>Guardar como imagen...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="480"/>
        <source>Ctrl+I</source>
        <comment>Save map as image</comment>
        <translation>Ctrl+I</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="481"/>
        <source>Save map as image</source>
        <translation>Guardar mapa como imagen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="484"/>
        <source>Export to MapServer Map...</source>
        <translation>Exportar a mapa de MapServer...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="485"/>
        <source>M</source>
        <comment>Export as MapServer .map file</comment>
        <translation>M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="486"/>
        <source>Export as MapServer .map file</source>
        <translation>Exportar como archivo .map de MapServer</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="489"/>
        <source>Exit</source>
        <translation>Salir</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="490"/>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="491"/>
        <source>Exit QGIS</source>
        <translation>Salir de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="496"/>
        <source>Add a Vector Layer...</source>
        <translation>Añadir una capa vectorial...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="497"/>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation>V</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="498"/>
        <source>Add a Vector Layer</source>
        <translation>Añadir una capa vectorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="501"/>
        <source>Add a Raster Layer...</source>
        <translation>Añadir una capa ráster...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="502"/>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation>R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="503"/>
        <source>Add a Raster Layer</source>
        <translation>Añadir una capa ráster</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="506"/>
        <source>Add a PostGIS Layer...</source>
        <translation>Añadir una capa de PostGIS...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="507"/>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation>D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="508"/>
        <source>Add a PostGIS Layer</source>
        <translation>Añadir una capa de PostGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="518"/>
        <source>New Vector Layer...</source>
        <translation>Nueva capa vectorial...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="519"/>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="520"/>
        <source>Create a New Vector Layer</source>
        <translation>Crear una capa vectorial nueva</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="523"/>
        <source>Remove Layer</source>
        <translation>Eliminar capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="524"/>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="525"/>
        <source>Remove a Layer</source>
        <translation>Eliminar una capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="528"/>
        <source>Add All To Overview</source>
        <translation>Añadir todo al localizador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="529"/>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation>+</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="530"/>
        <source>Show all layers in the overview map</source>
        <translation>Mostrar todas las capas en el localizador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="533"/>
        <source>Remove All From Overview</source>
        <translation>Eliminar todo del localizador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="534"/>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation>-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="535"/>
        <source>Remove all layers from overview map</source>
        <translation>Eliminar todas las capas del localizador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="538"/>
        <source>Show All Layers</source>
        <translation>Mostrar todas las capas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="539"/>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation>M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="540"/>
        <source>Show all layers</source>
        <translation>Mostrar todas las capas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="543"/>
        <source>Hide All Layers</source>
        <translation>Ocultar todas las capas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="544"/>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="545"/>
        <source>Hide all layers</source>
        <translation>Ocultar todas las capas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="550"/>
        <source>Project Properties...</source>
        <translation>Propiedades del proyecto...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="551"/>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation>P</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="552"/>
        <source>Set project properties</source>
        <translation>Definir las propiedades del proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="555"/>
        <source>Options...</source>
        <translation>Opciones...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="557"/>
        <source>Change various QGIS options</source>
        <translation>Cambiar varias opciones de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="560"/>
        <source>Custom Projection...</source>
        <translation>Proyección personalizada...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="562"/>
        <source>Manage custom projections</source>
        <translation>Administrar proyecciones personalizadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="567"/>
        <source>Help Contents</source>
        <translation>Contenidos de la ayuda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="569"/>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation>Ctrl+?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="571"/>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="573"/>
        <source>Help Documentation</source>
        <translation>Documentación de ayuda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="576"/>
        <source>Qgis Home Page</source>
        <translation>Página web de Qgis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="578"/>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation>Ctrl+W</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="580"/>
        <source>QGIS Home Page</source>
        <translation>Página web de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="583"/>
        <source>About</source>
        <translation>Acerca de</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="584"/>
        <source>About QGIS</source>
        <translation>Acerca de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="587"/>
        <source>Check Qgis Version</source>
        <translation>Comprobar versión de Qgis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="588"/>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Comprobar si su versión de QGIS está actualizada (requiere acceso a internet)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="593"/>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="594"/>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation>Ctrl+A</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="595"/>
        <source>Refresh Map</source>
        <translation>Actualizar mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="600"/>
        <source>Zoom In</source>
        <translation>Acercar zum</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="599"/>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation>Ctrl++</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="605"/>
        <source>Zoom Out</source>
        <translation>Alejar zum</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="604"/>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation>Ctrl+-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="608"/>
        <source>Zoom Full</source>
        <translation>Zum general</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="609"/>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation>G</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="610"/>
        <source>Zoom to Full Extents</source>
        <translation>Zum a toda la extensión</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="613"/>
        <source>Zoom To Selection</source>
        <translation>Zum a la selección</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="614"/>
        <source>Ctrl+F</source>
        <comment>Zoom to selection</comment>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="615"/>
        <source>Zoom to selection</source>
        <translation>Zum a la selección</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="618"/>
        <source>Pan Map</source>
        <translation>Desplazar mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="619"/>
        <source>Pan the map</source>
        <translation>Desplazar el mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="622"/>
        <source>Zoom Last</source>
        <translation>Zum anterior</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="624"/>
        <source>Zoom to Last Extent</source>
        <translation>Zum a la extensión anterior</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="627"/>
        <source>Zoom To Layer</source>
        <translation>Zum a la capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="629"/>
        <source>Zoom to Layer</source>
        <translation>Zum a la capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="632"/>
        <source>Identify Features</source>
        <translation>Identificar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="633"/>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation>I</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="634"/>
        <source>Click on features to identify them</source>
        <translation>Pulsar sobre los objetos espaciales para identificarlos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="638"/>
        <source>Select Features</source>
        <translation>Seleccionar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="644"/>
        <source>Open Table</source>
        <translation>Abrir tabla</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="648"/>
        <source>Measure Line </source>
        <translation>Regla </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="649"/>
        <source>Ctrl+M</source>
        <comment>Measure a Line</comment>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="650"/>
        <source>Measure a Line</source>
        <translation>Regla</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="653"/>
        <source>Measure Area</source>
        <translation>Medir áreas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="654"/>
        <source>Ctrl+J</source>
        <comment>Measure an Area</comment>
        <translation>Ctrl+J</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="655"/>
        <source>Measure an Area</source>
        <translation>Medir un área</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="660"/>
        <source>Show Bookmarks</source>
        <translation>Mostrar marcadores</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="659"/>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation>M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="665"/>
        <source>Show most toolbars</source>
        <translation>Mostrar todas las barras de herramientas posibles</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="671"/>
        <source>Hide most toolbars</source>
        <translation>Ocultar todas las barras de herramientas posibles</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="675"/>
        <source>New Bookmark...</source>
        <translation>Nuevo marcador...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="676"/>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation>Ctrl+M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5303"/>
        <source>New Bookmark</source>
        <translation>Nuevo marcador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="680"/>
        <source>Add WMS Layer...</source>
        <translation>Añadir capa WMS...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="681"/>
        <source>W</source>
        <comment>Add Web Mapping Server Layer</comment>
        <translation>W</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="682"/>
        <source>Add Web Mapping Server Layer</source>
        <translation>Añadir capa de servidor web de mapas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="685"/>
        <source>In Overview</source>
        <translation>Llevar al localizador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="686"/>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation>L</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="687"/>
        <source>Add current layer to overview map</source>
        <translation>Añadir la capa actual al localizador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="692"/>
        <source>Plugin Manager...</source>
        <translation>Administrador de complementos...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="694"/>
        <source>Open the plugin manager</source>
        <translation>Abrir el administrador de complementos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="711"/>
        <source>Capture Point</source>
        <translation>Capturar punto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="712"/>
        <source>.</source>
        <comment>Capture Points</comment>
        <translation>.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="713"/>
        <source>Capture Points</source>
        <translation>Capturar puntos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="717"/>
        <source>Capture Line</source>
        <translation>Capturar línea</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="718"/>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation>/</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="719"/>
        <source>Capture Lines</source>
        <translation>Capturar líneas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="723"/>
        <source>Capture Polygon</source>
        <translation>Capturar polígono</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="724"/>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation>Ctrl+/</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="725"/>
        <source>Capture Polygons</source>
        <translation>Capturar polígonos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="730"/>
        <source>Delete Selected</source>
        <translation>Borrar lo seleccionado</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="735"/>
        <source>Add Vertex</source>
        <translation>Añadir vértice</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="740"/>
        <source>Delete Vertex</source>
        <translation>Borrar vértice</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="745"/>
        <source>Move Vertex</source>
        <translation>Mover vértice</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="759"/>
        <source>Cut Features</source>
        <translation>Cortar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="760"/>
        <source>Cut selected features</source>
        <translation>Cortar los objetos espaciales seleccionados</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="764"/>
        <source>Copy Features</source>
        <translation>Copiar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="765"/>
        <source>Copy selected features</source>
        <translation>Copiar los objetos espaciales seleccionados</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="769"/>
        <source>Paste Features</source>
        <translation>Pegar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="770"/>
        <source>Paste selected features</source>
        <translation>Pegar los objetos espaciales seleccionados</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="833"/>
        <source>&amp;File</source>
        <translation>&amp;Archivo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="836"/>
        <source>&amp;Open Recent Projects</source>
        <translation>Abrir proyectos &amp;recientes</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="853"/>
        <source>&amp;View</source>
        <translation>&amp;Ver</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="871"/>
        <source>&amp;Layer</source>
        <translation>&amp;Capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="891"/>
        <source>&amp;Settings</source>
        <translation>C&amp;onfiguración</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="898"/>
        <source>&amp;Plugins</source>
        <translation>Co&amp;mplementos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="914"/>
        <source>&amp;Help</source>
        <translation>A&amp;yuda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="931"/>
        <source>File</source>
        <translation>Archivo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="942"/>
        <source>Manage Layers</source>
        <translation>Administrar capas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="960"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="967"/>
        <source>Digitizing</source>
        <translation>Digitalización</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="985"/>
        <source>Map Navigation</source>
        <translation>Navegación de mapas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="998"/>
        <source>Attributes</source>
        <translation>Atributos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1010"/>
        <source>Plugins</source>
        <translation>Complementos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1034"/>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>Barra de progreso que muestra el estado de representación de las capas y otras operaciones pesadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1059"/>
        <source>Displays the current map scale</source>
        <translation>Muestra la escala del mapa actual</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1074"/>
        <source>Render</source>
        <translation>Representar</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1077"/>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Cuando está marcada, las capas se representan respondiendo a los comandos de navegación y otros eventos. Cuando no está marcada, la representación no se hace. Esto permite añadir un gran número de capas y simbolizarlas antes de su representación.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1078"/>
        <source>Toggle map rendering</source>
        <translation>Conmutar la representación del mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1093"/>
        <source>This icon shows whether on the fly projection is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation>Este icono muestra si la proyección &quot;al vuelo&quot; está activada o no. Pulse el icono para mostrar el cuadro de diálogo de propiedades del proyecto para variar esta característica.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1094"/>
        <source>Projection status - Click to open projection dialog</source>
        <translation>Estado de la proyección - Pulse para abrir el cuadro de diálogo de proyección</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1098"/>
        <source>Ready</source>
        <translation>Preparado</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1210"/>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation>Vista del mapa. Aquí es donde se muestran las capas ráster y vectoriales cuando son añadidas al mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1255"/>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Localizador del mapa. Esta vista puede usarse para visualizar un mapa de localización que muestra la extensión de la vista del mapa. La extensión actual se muestra como un rectángulo rojo. Cualquier capa del mapa se puede añadir al localizador.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1288"/>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>Leyenda del mapa que muestra todas la capas actualmente en la vista del mapa. Marcar la casilla para conmutar su vista. Pulsar dos veces sobre una capa en la leyenda para personalizar su apariencia y ajustar otras propiedades.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1409"/>
        <source>Version </source>
        <translation>Versión </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1416"/>
        <source> with PostgreSQL support</source>
        <translation> con soporte para PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1419"/>
        <source> (no PostgreSQL support)</source>
        <translation> (sin soporte para PostgreSQL)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1421"/>
        <source>
Compiled against Qt </source>
        <translation>
Compilado contra Qt </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1422"/>
        <source>, running against Qt </source>
        <translation>, ejecutándose contra Qt </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1432"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation>Quantum GIS se distribuye bajo la Licencia Pública General GNU</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1434"/>
        <source>http://www.gnu.org/licenses</source>
        <translation>http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1436"/>
        <source>Version</source>
        <translation>Versión</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1438"/>
        <source>New features</source>
        <translation>Nuevos objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1476"/>
        <source>Available Data Provider Plugins</source>
        <translation>Complementos disponibles de proveedores de datos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="664"/>
        <source>T</source>
        <comment>Show most toolbars</comment>
        <translation>T</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="344"/>
        <source>Checking provider plugins</source>
        <translation>Comprobando complementos del proveedor</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="349"/>
        <source>Starting Python</source>
        <translation>Iniciando Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="775"/>
        <source>Python console</source>
        <translation>Consola de Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1575"/>
        <source>Python error</source>
        <translation>Error de Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1575"/>
        <source>Error when reading metadata of plugin </source>
        <translation>Error al leer metadatos del complemento </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="705"/>
        <source>Toggle editing</source>
        <translation>Conmutar edición</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="706"/>
        <source>Toggles the editing state of the current layer</source>
        <translation>Conmuta el estado de edición de la capa activa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="750"/>
        <source>Add Ring</source>
        <translation>Añadir anillo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="754"/>
        <source>Add Island</source>
        <translation>Añadir isla</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="755"/>
        <source>Add Island to multipolygon</source>
        <translation>Añadir isla a multipolígono</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1020"/>
        <source>Toolbar Visibility...</source>
        <translation>Visibilidad de barras de herramientas...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1047"/>
        <source>Scale </source>
        <translation>Escala </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1060"/>
        <source>Current map scale (formatted as x:y)</source>
        <translation>Escala actual del mapa (en formato X:Y)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1071"/>
        <source>Map coordinates at mouse cursor position</source>
        <translation>Coordenadas del mapa en la posición del ratón</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1445"/>
        <source>Python bindings - This is the major focus of this release it is now possible to create plugins using python. It is also possible to create GIS enabled applications written in python that use the QGIS libraries.</source>
        <translation>Enlaces Python - Este es el principal objetivo de esta versión. Ahora es posible crear complementos usando Python. También es posible crear aplicaciones de SIG escritas en python que usen las bibliotecas de QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1448"/>
        <source>Removed automake build system - QGIS now needs CMake for compilation.</source>
        <translation>Eliminado el sistema de compilación de automake - QGIs ahora necesita CMake para compilar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1451"/>
        <source>Many new GRASS tools added (with thanks to http://faunalia.it/)</source>
        <translation>Muchas herramientas de GRASS nuevas añadidas (gracias a http://faunalia.it/)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1454"/>
        <source>Map Composer updates</source>
        <translation>Actualizaciones del diseñador de mapas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1457"/>
        <source>Crash fix for 2.5D shapefiles</source>
        <translation>Resolución de cuelgues con archivos shape 2.5D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1460"/>
        <source>The QGIS libraries have been refactored and better organised.</source>
        <translation>Las bibliotecas de QGIS se han rehecho y organizado mejor.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1463"/>
        <source>Improvements to the GeoReferencer</source>
        <translation>Mejoras en el Georreferenciador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="670"/>
        <source>Ctrl+T</source>
        <comment>Hide most toolbars</comment>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1070"/>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation>Muestra las coordenadas del mapa en la posición actual del cursor. La visualización se actualiza continuamente al mover el ratón.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1466"/>
        <source>Added locale options to options dialog.</source>
        <translation>Se han añadido las opciones de locale al diálogo de opciones.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1853"/>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>Abrir una capa vectorial soportada por OGR</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1987"/>
        <source>is not a valid or recognized data source</source>
        <translation>no es una fuente de datos válida o reconocida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5040"/>
        <source>Invalid Data Source</source>
        <translation>Fuente de datos no válida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2081"/>
        <source>Invalid Layer</source>
        <translation>Capa no válida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2081"/>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>%1 es una capa no válida y no se puede cargar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2564"/>
        <source>Save As</source>
        <translation>Guardar como</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2665"/>
        <source>Choose a QGIS project file to open</source>
        <translation>Seleccionar un archivo de proyecto de QGIS para abrir</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2776"/>
        <source>QGIS Project Read Error</source>
        <translation>Error de lectura del proyecto de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2778"/>
        <source>Try to find missing layers?</source>
        <translation>¿Buscar las capas perdidas?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2794"/>
        <source>Unable to open project</source>
        <translation>No se puede abrir el proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2833"/>
        <source>Choose a QGIS project file</source>
        <translation>Seleccionar un archivo de proyecto de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2950"/>
        <source>Saved project to:</source>
        <translation>Proyecto guardado en:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2957"/>
        <source>Unable to save project</source>
        <translation>No se puede guardar el proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2958"/>
        <source>Unable to save project to </source>
        <translation>No se puede guardar el proyecto en </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2964"/>
        <source>Unable to save project </source>
        <translation>No se puede guardar el proyecto </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2906"/>
        <source>Choose a filename to save the QGIS project file as</source>
        <translation>Seleccionar un nombre de archivo para guardar como proyecto de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3022"/>
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: No se puede cargar el proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3023"/>
        <source>Unable to load project </source>
        <translation>No se puede cargar el proyecto </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3146"/>
        <source>Choose a filename to save the map image as</source>
        <translation>Seleccionar un nombre de archivo para guardar el mapa como imagen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3188"/>
        <source>Saved map image to</source>
        <translation>Imagen del mapa guardada en</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3361"/>
        <source>No Layer Selected</source>
        <translation>Ninguna capa seleccionada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3362"/>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation>Para borrar objetos espaciales, debe seleccionar una capa vectorial en la leyenda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3369"/>
        <source>No Vector Layer Selected</source>
        <translation>Ninguna capa vectorial seleccionada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3370"/>
        <source>Deleting features only works on vector layers</source>
        <translation>Borrar objetos espaciales solo funciona en capas vectoriales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3376"/>
        <source>Provider does not support deletion</source>
        <translation>El proveedor no soporta el borrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3377"/>
        <source>Data provider does not support deleting features</source>
        <translation>El proveedor de datos no soporta el borrado de objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3383"/>
        <source>Layer not editable</source>
        <translation>Capa no editable</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3384"/>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation>La capa activa no se puede editar. Elegir &apos;Conmutar edición&apos; en la barra de herramientas Digitalización.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3391"/>
        <source>Problem deleting features</source>
        <translation>Problema al borrar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3392"/>
        <source>A problem occured during deletion of features</source>
        <translation>Ha ocurrido un problema durante el borrado de objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3616"/>
        <source>Invalid scale</source>
        <translation>Escala no válida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3851"/>
        <source>Error Loading Plugin</source>
        <translation>Error al cargar el complemento</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3851"/>
        <source>There was an error loading %1.</source>
        <translation>Ha habido un error al cargar %1.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3887"/>
        <source>No MapLayer Plugins</source>
        <translation>No hay complementos de MapLayer</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3887"/>
        <source>No MapLayer plugins in ../plugins/maplayer</source>
        <translation>No hay complementos de MapLayer en ../plugins/maplayer</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3964"/>
        <source>No Plugins</source>
        <translation>No hay complementos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3965"/>
        <source>No plugins found in ../plugins. To test plugins, start qgis from the src directory</source>
        <translation>No se han encontrado complementos en ../plugins. Para probar complementos, inicie qgis desde el directorio src</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3999"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3999"/>
        <source>Plugin %1 is named %2</source>
        <translation>El complemento %1 se llama %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4016"/>
        <source>Plugin Information</source>
        <translation>Información del complemento</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4017"/>
        <source>QGis loaded the following plugin:</source>
        <translation>QGis ha cargado el siguiente complemento:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4017"/>
        <source>Name: %1</source>
        <translation>Nombre: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4017"/>
        <source>Version: %1</source>
        <translation>Versión: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4018"/>
        <source>Description: %1</source>
        <translation>Descripción: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4036"/>
        <source>Unable to Load Plugin</source>
        <translation>No se puede cargar el complemento</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4037"/>
        <source>QGIS was unable to load the plugin from: %1</source>
        <translation>QGIS no ha podido cargar el complemento desde: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4093"/>
        <source>There is a new version of QGIS available</source>
        <translation>Hay una nueva versión de QGIS disponible</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4099"/>
        <source>You are running a development version of QGIS</source>
        <translation>Está utilizando una versión de desarrollo de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4103"/>
        <source>You are running the current version of QGIS</source>
        <translation>Está utilizando la versión actualizada de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4108"/>
        <source>Would you like more information?</source>
        <translation>¿Desea más información?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4155"/>
        <source>QGIS Version Information</source>
        <translation>Información de la versión de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4115"/>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS - Cambios en el SVN desde la última versión</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4127"/>
        <source>Unable to get current version information from server</source>
        <translation>No se puede obtener información de la versión actual del servidor</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4141"/>
        <source>Connection refused - server may be down</source>
        <translation>Conexión rehusada - el servidor puede estar fuera de servicio</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4144"/>
        <source>QGIS server was not found</source>
        <translation>No se ha encontrado el servidor QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4147"/>
        <source>Network error while communicating with server</source>
        <translation>Error de red mientras se comunicaba con el servidor</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4150"/>
        <source>Unknown network socket error</source>
        <translation>Error de socket de red desconocido</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4155"/>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>No se puede comunicar con el servidor de versión de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4234"/>
        <source>QGIS Browser Selection</source>
        <translation>Selección de navegador de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4235"/>
        <source>Enter the name of a web browser to use (eg. konqueror).
</source>
        <translation>Introducir el nombre de un navegador web a utilizar (ej. konqueror).
</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4236"/>
        <source>Enter the full path if the browser is not in your PATH.
</source>
        <translation>Introducir la ruta completa si el navegador no está en la variable de entorno PATH.
</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4237"/>
        <source>You can change this option later by selecting Options from the Settings menu (Help Browser tab).</source>
        <translation>Puede cambiar esta opción más tarde seleccionadon Opciones desde el menú Configuración (pestaña Navegador de la ayuda).</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5138"/>
        <source>Layer is not valid</source>
        <translation>La capa no es válida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5139"/>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>La capa no es válida y no se puede añadir al mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4402"/>
        <source>Save?</source>
        <translation>¿Guardar?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4403"/>
        <source>Do you want to save the current project?</source>
        <translation>¿Quiere guardar el proyecto actual?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4589"/>
        <source>Extents: </source>
        <translation>Extensiones: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4876"/>
        <source>Clipboard contents set to: </source>
        <translation>Contenido del portapapeles ajustado a: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4925"/>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>Abrir una fuente de datos ráster soportada por GDAL</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5039"/>
        <source> is not a valid or recognized raster data source</source>
        <translation> no es una fuente de datos ráster válida o reconocida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5217"/>
        <source> is not a supported raster data source</source>
        <translation> no es una fuente de datos ráster soportada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5218"/>
        <source>Unsupported Data Source</source>
        <translation>Fuente de datos no soportada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5304"/>
        <source>Enter a name for the new bookmark:</source>
        <translation>Introducir un nombre para el nuevo marcador:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5321"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5321"/>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>No se puede crear el marcador. Puede que falte su base de datos de usuario o esté dañada</translation>
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
        <location filename="../src/app/qgsabout.cpp" line="111"/>
        <source>QGIS Sponsors</source>
        <translation>Patrocinadores de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="114"/>
        <source>The following have sponsored QGIS by contributing money to fund development and other project costs</source>
        <translation>Los siguientes han patrocinado QGIS aportando fondos para financiar el desarrollo y otros costes del proyecto</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="118"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="118"/>
        <source>Website</source>
        <translation>Página web</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="246"/>
        <source>QGIS Browser Selection</source>
        <translation>Selección del navegador de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="250"/>
        <source>Enter the name of a web browser to use (eg. konqueror).
Enter the full path if the browser is not in your PATH.
You can change this option later by selection Options from the Settings menu (Help Browser tab).</source>
        <translation>Introducir el nombre de un navegador a usar (ej. konqueror).
Introducir la ruta completa si el navegador no está en su variable de entorno PATH.
Puede cambiar esta opción más tarde seleccionando Opciones desde el menú Configuración (pestaña Navegador de la ayuda).</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="13"/>
        <source>About Quantum GIS</source>
        <translation>Acerca de Quantum GIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="32"/>
        <source>About</source>
        <translation>Acerca de</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="76"/>
        <source>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="86"/>
        <source>Version</source>
        <translation>Versión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="100"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Arial&apos;; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p align=&quot;center&quot; style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Quantum GIS is licensed under the GNU General Public License&lt;/p&gt;
&lt;p align=&quot;center&quot; style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;http://www.gnu.org/licenses&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Arial&apos;; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p align=&quot;center&quot; style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Quantum GIS se distribuye bajo la Licencia Pública General GNU&lt;/p&gt;
&lt;p align=&quot;center&quot; style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;http://www.gnu.org/licenses&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="111"/>
        <source>QGIS Home Page</source>
        <translation>Página web de QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="121"/>
        <source>Subscribe to the QGIS-User mailing list</source>
        <translation>Suscribirse a la lista de correo QGIS-User</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="132"/>
        <source>What&apos;s New</source>
        <translation>Novedades</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="161"/>
        <source>Developers</source>
        <translation>Desarrolladores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="229"/>
        <source>&lt;h2&gt;QGIS Developers&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Desarrolladores de QGIS&lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="237"/>
        <source>Providers</source>
        <translation>Proveedores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="253"/>
        <source>Sponsors</source>
        <translation>Patrocinadores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="300"/>
        <source>Ok</source>
        <translation>Aceptar</translation>
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
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="52"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="59"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="87"/>
        <source>Type:</source>
        <translation>Tipo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="100"/>
        <source>Name:</source>
        <translation>Nombre:</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="57"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="58"/>
        <source>Action</source>
        <translation>Acción</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="59"/>
        <source>Capture</source>
        <translation>Capturar</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="142"/>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation>Seleccionar una acción</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="21"/>
        <source>Form1</source>
        <translation>Formulario1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="49"/>
        <source>Remove the selected action</source>
        <translation>Eliminar la acción seleccionada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="52"/>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="59"/>
        <source>Move the selected action down</source>
        <translation>Mover la acción seleccionada hacia abajo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="62"/>
        <source>Move down</source>
        <translation>Bajar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="69"/>
        <source>Move the selected action up</source>
        <translation>Mover la acción seleccionada hacia arriba</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="72"/>
        <source>Move up</source>
        <translation>Subir</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="87"/>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>Esta lista contiene todas las acciones que han sido definidas para la capa actual. Añada acciones introduciendo los detalles en los controles inferiores y presionando el botón Insertar acción. Las acciones se pueden editar pulsando dos veces sobre ellas.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="119"/>
        <source>The valid attribute names for this layer</source>
        <translation>Los nombres de atributo válidos para esta capa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="126"/>
        <source>Browse for action commands</source>
        <translation>Buscar comandos de acción</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="129"/>
        <source>Browse</source>
        <translation>Buscar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="136"/>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation>Inserta el campo seleccionado en la acción, precedido con un %</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="139"/>
        <source>Insert field</source>
        <translation>Insertar campo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="162"/>
        <source>Update the selected action</source>
        <translation>Actualizar la acción seleccionada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="165"/>
        <source>Update action</source>
        <translation>Actualizar acción</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="172"/>
        <source>Inserts the action into the list above</source>
        <translation>Inserta la acción en la lista de arriba</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="175"/>
        <source>Insert action</source>
        <translation>Insertar acción</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="182"/>
        <source>Captures any output from the action</source>
        <translation>Captura cualquier salida de la acción</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="185"/>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation>Captura la salida estándar o el error generado por la acción y la muestra en un cuadro de diálogo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="188"/>
        <source>Capture output</source>
        <translation>Capturar salida</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="195"/>
        <source>Enter the action command here</source>
        <translation>Introduzca aquí el comando de acción</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="208"/>
        <source>Action:</source>
        <translation>Acción:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="218"/>
        <source>Enter the action name here</source>
        <translation>Introduzca el nombre de la acción aquí</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="228"/>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>Introduzca el nombre de la acción aquí. El nombre debe ser único (QGIS lo hará único si es necesario).</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="231"/>
        <source>Name:</source>
        <translation>Nombre:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="205"/>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation>Introduzca la acción aquí. Ésta puede ser cualquier programa, script o comando que esté disponible en su sistema. Cuando se invoca una acción cualquier conjunto de caracteres que empiece por % y luego tenga el nombre de un campo se reemplazará con el valor de ese campo. Los caracteres especiales %% se reemplazarán por el valor del campo que se seleccionó. Las comillas dobles agrupan texto en argumentos simples para el programa, script o comando. Las comillas dobles se ignorarán si se preceden por una barra invertida (\)</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="16"/>
        <source>Enter Attribute Values</source>
        <translation>Introducir valores de los atributos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="32"/>
        <source>1</source>
        <translation>1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="37"/>
        <source>Attribute</source>
        <translation>Atributo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="42"/>
        <source>Value</source>
        <translation>Valor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="50"/>
        <source>&amp;OK</source>
        <translation>&amp;Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="57"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="280"/>
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
        <location filename="../src/ui/qgsattributetablebase.ui" line="214"/>
        <source>&amp;Help</source>
        <translation>A&amp;yuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="282"/>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="224"/>
        <source>Search for:</source>
        <translation>Buscar:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="237"/>
        <source>in</source>
        <translation>en</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="259"/>
        <source>Search</source>
        <translation>Búsqueda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="269"/>
        <source>Adva&amp;nced...</source>
        <translation>Ava&amp;nzado...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="272"/>
        <source>Alt+N</source>
        <translation>Alt+N</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="279"/>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="36"/>
        <source>Remove selection</source>
        <translation>Eliminar selección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="52"/>
        <source>Move selected to top</source>
        <translation>Mover la selección arriba del todo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="61"/>
        <source>Ctrl+T</source>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="68"/>
        <source>Invert selection</source>
        <translation>Invertir selección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="77"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="84"/>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Copiar las filas seleccionadas al portapapeles (Ctrl+C)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="87"/>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Copia las filas seleccionadas al portapapeles</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="96"/>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="127"/>
        <source>New column</source>
        <translation>Nueva columna</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="136"/>
        <source>Ctrl+N</source>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="143"/>
        <source>Delete column</source>
        <translation>Eliminar columna</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="152"/>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="175"/>
        <source>Start editing</source>
        <translation>Comenzar edición</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="182"/>
        <source>Stop editin&amp;g</source>
        <translation>Terminar &amp;edición</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="185"/>
        <source>Alt+G</source>
        <translation>Alt+E</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="103"/>
        <source>Zoom map to the selected rows (Ctrl-F)</source>
        <translation>Zum a las filas seleccionadas (Ctrl+F)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="106"/>
        <source>Zoom map to the selected rows</source>
        <translation>Zum a las filas seleccionadas </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="112"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="94"/>
        <source>select</source>
        <translation>seleccionar</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="95"/>
        <source>select and bring to top</source>
        <translation>seleccionar y llevar arriba</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="96"/>
        <source>show only matching</source>
        <translation>mostrar solo coincidentes</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="321"/>
        <source>Search string parsing error</source>
        <translation>Buscar error de análisis de la cadena</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="373"/>
        <source>Search results</source>
        <translation>Resultados de la búsqueda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="327"/>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>Ha suministrado una cadena de búsqueda vacía.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="361"/>
        <source>Error during search</source>
        <translation>Error durante la búsqueda</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="370"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform>Se han encontrado %d objetos espaciales coincidentes.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="372"/>
        <source>No matching features found.</source>
        <translation>No se han encontrado coincidencias.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="144"/>
        <source>Name conflict</source>
        <translation>Conflicto de nombre</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="193"/>
        <source>Stop editing</source>
        <translation>Terminar edición</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="194"/>
        <source>Do you want to save the changes?</source>
        <translation>¿Quiere guardar los cambios?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="200"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="200"/>
        <source>Could not commit changes</source>
        <translation>No se pudieron guardar los cambios</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="144"/>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation>No se pudo insertar el atributo. El nombre ya existe en la tabla.</translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="140"/>
        <source>Really Delete?</source>
        <translation>¿Borrar realmente?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="141"/>
        <source>Are you sure you want to delete the </source>
        <translation>¿Está seguro de que quiere borrar el marcador </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="141"/>
        <source> bookmark?</source>
        <translation>?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="157"/>
        <source>Error deleting bookmark</source>
        <translation>Error al borrar marcador</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="159"/>
        <source>Failed to delete the </source>
        <translation>Ha fallado el borrado del marcador </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="161"/>
        <source> bookmark from the database. The database said:
</source>
        <translation> de la base de datos. La base de datos ha dicho:</translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="16"/>
        <source>Geospatial Bookmarks</source>
        <translation>Marcadores geoespaciales</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="45"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="50"/>
        <source>Project</source>
        <translation>Proyecto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="55"/>
        <source>Extent</source>
        <translation>Extensión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="60"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="75"/>
        <source>Close the dialog</source>
        <translation>Cerrar el cuadro de diálogo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="68"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="78"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="85"/>
        <source>Delete the currently selected bookmark</source>
        <translation>Borrar el marcador seleccionado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="88"/>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="95"/>
        <source>Zoom to the currently selected bookmark</source>
        <translation>Zum al marcador seleccionado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="98"/>
        <source>Zoom To</source>
        <translation>Zum a</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="51"/>
        <source>QGIS - print composer</source>
        <translation>QGIS - diseñador de mapas</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="73"/>
        <source>Map 1</source>
        <translation>Mapa 1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="500"/>
        <source>Couldn&apos;t open </source>
        <translation>No se puede abrir </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="500"/>
        <source> for read/write</source>
        <translation> para leer/escribir</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="579"/>
        <source>Error in Print</source>
        <translation>Error al imprimir</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="557"/>
        <source>Cannot seek</source>
        <translation>No se puede solicitar</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="482"/>
        <source>Cannot overwrite BoundingBox</source>
        <translation>No se puede sobrescribir el marco</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="490"/>
        <source>Cannot find BoundingBox</source>
        <translation>No se puede encontrar el marco</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="573"/>
        <source>Cannot overwrite translate</source>
        <translation>No se puede sobrescribir la traducción</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="579"/>
        <source>Cannot find translate</source>
        <translation>No se puede encontrar la traducción</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="587"/>
        <source>File IO Error</source>
        <translation>Error de ES del archivo</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="597"/>
        <source>Paper does not match</source>
        <translation>El papel no coincide</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="598"/>
        <source>The selected paper size does not match the composition size</source>
        <translation>El tamaño de papel seleccionado no se ajusta al tamaño del mapa</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="661"/>
        <source>Big image</source>
        <translation>Imagen grande</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="662"/>
        <source>To create image </source>
        <translation>Crear imagen </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="665"/>
        <source> requires circa </source>
        <translation> requiere cerca de </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="665"/>
        <source> MB of memory</source>
        <translation> MB de memoria</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="692"/>
        <source>format</source>
        <translation>formato</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="715"/>
        <source>Choose a filename to save the map image as</source>
        <translation>Seleccione un nombre de archivo para guardar el mapa como imagen</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="787"/>
        <source>SVG warning</source>
        <translation>Aviso de SVG</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="788"/>
        <source>Don&apos;t show this message again</source>
        <translation>No volver a mostrar este mensaje</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="802"/>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the Qt4 svg code. Of note, text does not appear in the SVG file and there are problems with the map bounding box clipping other items such as the legend or scale bar.&lt;/p&gt;If you require a vector-based output file from Qgis it is suggested that you try printing to PostScript if the SVG output is not satisfactory.&lt;/p&gt;</source>
        <translation>&lt;p&gt;La función &quot;Exportar a SVG&quot; en Qgis tiene varios problemas debido a errores y deficiencias en el código svg de Qt4. Entre otros problemas, los textos no aparecen en los archivos SVG y hay problemas cuando el marco corta otros elementos como la leyenda o la barra de escala.&lt;/p&gt;Si necesita un mapa vectorial hecho con Qgis, le sugerimos que lo imprima como PostScript si la salida como SVG no es satisfactoria.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="807"/>
        <source>Choose a filename to save the map as</source>
        <translation>Seleccionar un nombre de archivo para guardar el mapa</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="808"/>
        <source>SVG Format</source>
        <translation>Formato SVG</translation>
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
        <translation>Ítem</translation>
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
        <translation>Seleccionar/Mover ítem</translation>
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
        <location filename="../src/app/composer/qgscomposermap.cpp" line="74"/>
        <source>Map %1</source>
        <translation> Mapa %1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="96"/>
        <source>Extent (calculate scale)</source>
        <translation>Extensión (calcular escala)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="97"/>
        <source>Scale (calculate extent)</source>
        <translation>Escala (calcular extensión)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="104"/>
        <source>Cache</source>
        <translation>Caché</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="105"/>
        <source>Render</source>
        <translation>Representar</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="106"/>
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
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="401"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="402"/>
        <source>Cannot load picture.</source>
        <translation>No se puede cargar el dibujo.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="468"/>
        <source>Pictures (</source>
        <translation>Dibujos (</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="485"/>
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
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="104"/>
        <source>Legend</source>
        <translation>Leyenda</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="117"/>
        <source>Layers</source>
        <translation>Capas</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="118"/>
        <source>Group</source>
        <translation>Grupo</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="125"/>
        <source>Combine selected layers</source>
        <translation>Combinar capas seleccionadas</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="138"/>
        <source>Cache</source>
        <translation>Caché</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="139"/>
        <source>Render</source>
        <translation>Representar</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="140"/>
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
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="163"/>
        <source>Font</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="148"/>
        <source>Box</source>
        <translation>Cajetín</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="140"/>
        <source>Column 1</source>
        <translation>Columna 1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="53"/>
        <source>Preview</source>
        <translation>Previsualización</translation>
    </message>
</context>
<context>
    <name>QgsComposition</name>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="84"/>
        <source>Custom</source>
        <translation>Personalizado</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="85"/>
        <source>A5 (148x210 mm)</source>
        <translation>A5 (148x210 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="86"/>
        <source>A4 (210x297 mm)</source>
        <translation>A4 (210x297 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="87"/>
        <source>A3 (297x420 mm)</source>
        <translation>A3 (297x420 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="88"/>
        <source>A2 (420x594 mm)</source>
        <translation>A2 (420x594 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="89"/>
        <source>A1 (594x841 mm)</source>
        <translation>A1 (594x841 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="90"/>
        <source>A0 (841x1189 mm)</source>
        <translation>A0 (841x1189 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="91"/>
        <source>B5 (176 x 250 mm)</source>
        <translation>B5 (176 x 250 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="92"/>
        <source>B4 (250 x 353 mm)</source>
        <translation>B4 (250 x 353 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="93"/>
        <source>B3 (353 x 500 mm)</source>
        <translation>B3 (353 x 500 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="94"/>
        <source>B2 (500 x 707 mm)</source>
        <translation>B2 (500 x 707 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="95"/>
        <source>B1 (707 x 1000 mm)</source>
        <translation>B1 (707 x 1000 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="96"/>
        <source>B0 (1000 x 1414 mm)</source>
        <translation>B0 (1000 x 1414 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="97"/>
        <source>Letter (8.5x11 inches)</source>
        <translation>Carta (8.5x11 pulgadas)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="98"/>
        <source>Legal (8.5x14 inches)</source>
        <translation>Legal (8.5x14 pulgadas)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="111"/>
        <source>Portrait</source>
        <translation>Vertical</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="112"/>
        <source>Landscape</source>
        <translation>Horizontal</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="607"/>
        <source>Out of memory</source>
        <translation>Sin memoria</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="610"/>
        <source>Qgis is unable to resize the paper size due to insufficient memory.
 It is best that you avoid using the map composer until you restart qgis.
</source>
        <translation>Qgis no puede cambiar el tamaño del papel debido a una falta de memoria.
 Es mejor que no utilice el diseñador de mapas hasta que reinicie qgis.
</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="754"/>
        <source>Label</source>
        <translation>Etiqueta</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="804"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="805"/>
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
    <name>QgsConnectionDialog</name>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="74"/>
        <source>Test connection</source>
        <translation>Probar conexión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="72"/>
        <source>Connection to </source>
        <translation>La conexión a </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="72"/>
        <source> was successfull</source>
        <translation> ha sido correcta</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="74"/>
        <source>Connection failed - Check settings and try again </source>
        <translation>La conexión ha fallado - Compruebe la configuración y pruebe de nuevo </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="96"/>
        <source>General Interface Help:

</source>
        <translation>Ayuda de la interfaz general:

</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialogBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="13"/>
        <source>Create a New PostGIS connection</source>
        <translation>Crear una nueva conexión a PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="31"/>
        <source>Connection Information</source>
        <translation>Información de la conexión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="51"/>
        <source>Save Password</source>
        <translation>Guardar contraseña</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="58"/>
        <source>Test Connect</source>
        <translation>Probar conexión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="83"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="90"/>
        <source>Host</source>
        <translation>Servidor</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="97"/>
        <source>Database</source>
        <translation>Base de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="104"/>
        <source>Port</source>
        <translation>Puerto</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="111"/>
        <source>Username</source>
        <translation>Nombre de usuario</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="118"/>
        <source>Password</source>
        <translation>Contraseña</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="135"/>
        <source>Name of the new connection</source>
        <translation>Nombre de la nueva conexión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="148"/>
        <source>5432</source>
        <translation>5432</translation>
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
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="28"/>
        <source>Draw polygon outline</source>
        <translation>Dibujar contorno del polígono</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="41"/>
        <source>Classification Field:</source>
        <translation>Campo de clasificación:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="57"/>
        <source>Minimum Value:</source>
        <translation>Valor mínimo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="73"/>
        <source>Outline Width:</source>
        <translation>Anchura del contorno:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="99"/>
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
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="148"/>
        <source>Placement</source>
        <translation>Ubicación</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="156"/>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="161"/>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="166"/>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="171"/>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="179"/>
        <source>Orientation</source>
        <translation>Orientación</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="187"/>
        <source>Horizontal</source>
        <translation>Horizontal</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="192"/>
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
        <location filename="" line="7471221"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:14pt;&quot;&gt;&#xa9; QGIS 2006&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:14pt;&quot;&gt;© QGIS 2006&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="130"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:14pt;&quot;&gt;© QGIS 2006&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialog</name>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="165"/>
        <source>Delete Projection Definition?</source>
        <translation>¿Borrar definición de proyección?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="166"/>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Borrar una definición de proyección no es reversible. ¿Desea continuar?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="876"/>
        <source>Abort</source>
        <translation>Abortar</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="878"/>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="933"/>
        <source>QGIS Custom Projection</source>
        <translation>Proyección personalizada de QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="750"/>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, dele un nombre a la proyección antes de guardar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="756"/>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, rellene los parámetros antes de guardar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="771"/>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, añada una cláusula proj= antes de guardar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="778"/>
        <source>This proj4 ellipsoid definition is not valid. Please add a ellips= clause before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, añada una cláusula ellips= antes de guardar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="794"/>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, corregir antes de guardar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="907"/>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Esta definición de proyección proj4 no es válida.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="922"/>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>El Norte y el Este deben estar en formato decimal.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="934"/>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Error interno (¿la proyección original no es válida?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="65"/>
        <source>Custom Projection Definition</source>
        <translation>Definición de proyección personalizada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="32"/>
        <source>Define</source>
        <translation>Definición</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="198"/>
        <source>Parameters:</source>
        <translation>Parámetros:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="90"/>
        <source>|&lt;</source>
        <translation>|&lt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="97"/>
        <source>&lt;</source>
        <translation>&lt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="104"/>
        <source>1 of 1</source>
        <translation>1 de 1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="114"/>
        <source>&gt;</source>
        <translation>&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="121"/>
        <source>&gt;|</source>
        <translation>&gt;|</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="128"/>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="135"/>
        <source>Save</source>
        <translation>Guardar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="142"/>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="149"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="158"/>
        <source>Name:</source>
        <translation>Nombre:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="169"/>
        <source>Test</source>
        <translation>Probar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="181"/>
        <source>Transform from WGS84 to the chosen projection</source>
        <translation>Transformar de WGS84 a la proyección elegida</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="227"/>
        <source>Projected Corrdinate System</source>
        <translation>Sistema de coordenadas proyectado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="234"/>
        <source>Geographic / WGS84</source>
        <translation>Geográficas / WGS84</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="260"/>
        <source>East:</source>
        <translation>Este:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="270"/>
        <source>North:</source>
        <translation>Norte:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="300"/>
        <source>Calculate</source>
        <translation>Calcular</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="44"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;You can define your own custom projection here. The definition must conform to the proj4 format for specifying a Spatial Reference System.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Puede definir su propia proyección personalizada aquí. La definición debe ajustarse al formato proj4 para especificar un sistema de referencia espacial.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="188"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Use the text boxes below to test the projection definition you are creating. Enter a coordinate where both the lat/long and the projected result are known (for example by reading off a map). Then press the calculate button to see if the projection definition you are creating is accurate.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Use los cuadros de texto de abajo para probar la definición de proyección que esté creando. Introduzca una coordenada en la que la tanto lat/long como el resultado proyectado sean conocidos (por ejemplo tomándolos de un mapa). A continuación pulse el botón calcular para ver su la definición de proyección que ha creado es exacta.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="112"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="113"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="114"/>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="250"/>
        <source>Are you sure you want to remove the </source>
        <translation>¿Está seguro de que quiere eliminar la conexión </translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="250"/>
        <source> connection and all associated settings?</source>
        <translation> y su configuración?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="251"/>
        <source>Confirm Delete</source>
        <translation>Confirmar la eliminación</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="317"/>
        <source>Select Table</source>
        <translation>Seleccionar tabla</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="317"/>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Debe seleccionar una tabla para poder añadir una capa.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="366"/>
        <source>Password for </source>
        <translation>Contraseña para </translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="367"/>
        <source>Please enter your password:</source>
        <translation>Por favor, introduzca su contraseña:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="404"/>
        <source>Point layer</source>
        <translation>Capa de puntos</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="407"/>
        <source>Multi-point layer</source>
        <translation>Capa multipunto</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="411"/>
        <source>Linestring layer</source>
        <translation>Capa de líneas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="414"/>
        <source>Multi-linestring layer</source>
        <translation>Capa multilínea</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="418"/>
        <source>Polygon layer</source>
        <translation>Capa de polígonos</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="421"/>
        <source>Multi-polygon layer</source>
        <translation>Capa multipolígono</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="425"/>
        <source>Mixed geometry layer</source>
        <translation>Capa de geometría mixta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="428"/>
        <source>Geometry collection layer</source>
        <translation>Capa de grupo de geometría</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="432"/>
        <source>Waiting for layer type</source>
        <translation>Esperando el tipo de capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="435"/>
        <source>Unknown layer type</source>
        <translation>Tipo de capa desconocido</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="495"/>
        <source>Connection failed</source>
        <translation>Conexión fallida</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="498"/>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>La conexión a %1 en %2 ha fallado. Puede ser que la base de datos esté inactiva o que su configuración sea incorrecta.%3Compruebe el nombre de usuario y la contraseña y pruebe de nuevo.%4La base de datos ha dicho:%5%6</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="329"/>
        <source>double click to open PostgreSQL query builder</source>
        <translation>doble clic para abrir el constructor de consultar de PostgreSQL</translation>
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
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="45"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="48"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="74"/>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="90"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="108"/>
        <source>Encoding:</source>
        <translation>Codificación:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="115"/>
        <source>Tables:</source>
        <translation>Tablas:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="146"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="151"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="156"/>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="167"/>
        <source>PostgreSQL Connections</source>
        <translation>Conexiones de PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="179"/>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="186"/>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="193"/>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="200"/>
        <source>Connect</source>
        <translation>Conectar</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="16"/>
        <source>Delete Attributes</source>
        <translation>Borrar atributos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="52"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="59"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
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
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="118"/>
        <source>No layer name</source>
        <translation>Ningún nombre de capa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="118"/>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Por favor, introduzca un nombre para la capa antes de añadir ésta al mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="204"/>
        <source>No delimiter</source>
        <translation>No hay delimitador</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="204"/>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Por favor, especifique un delimitador antes de analizar el archivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="238"/>
        <source>Choose a delimited text file to open</source>
        <translation>Seleccione un archivo de texto delimitado para abrir</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="35"/>
        <source>Parse</source>
        <translation>Analizar</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="61"/>
        <source>&lt;h2&gt;Description&lt;/h2&gt;&lt;p&gt;Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!&lt;/p&gt;&lt;p&gt;Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.&lt;/p&gt;</source>
        <translation>&lt;h2&gt;Descripción&lt;/h2&gt;&lt;p&gt;Seleccione un archivo de texto delimitado que contenga una fila de encabezado y una o más filas de coordenadas X e Y que le gustaría usar como una capa de puntos y este complemento hará el trabajo por usted&lt;/p&gt;&lt;p&gt;Use el cuadro Nombre de la capa para especificar el nombre para la leyenda de la nueva capa. Use el cuadro Delimitador para especificar qué delimitador se usa en su archivo (ej. espacio, coma, tabulador o una expresión regular en estilo Perl). Después de seleccionar un delimitador, pulse el botón Analizar y seleccione las columnas que contienen los valores de la X y la Y para la capa.&lt;/p&gt;</translation>
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
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="70"/>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Coordenada X&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="91"/>
        <source>Name of the field containing x values</source>
        <translation>Nombre del campo que contiene los valores de la X</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="94"/>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nombre del campo que contiene los valores de la X. Seleccione un campo de la lista. La lista se genera al analizar la fila de encabezado del archivo de texto delimitado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="104"/>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Coordenada Y&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="125"/>
        <source>Name of the field containing y values</source>
        <translation>Nombre del campo que contiene los valores de la Y</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="128"/>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nombre del campo que contiene los valores de la Y. Seleccione un campo de la lista. La lista se genera al analizar la fila de encabezado del archivo de texto delimitado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="249"/>
        <source>Sample text</source>
        <translation>Texto de muestra</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="39"/>
        <source>Delimited Text Layer</source>
        <translation>Capa de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="148"/>
        <source>Delimited text file</source>
        <translation>Archivo de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="155"/>
        <source>Full path to the delimited text file</source>
        <translation>Ruta completa al archivo de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="158"/>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Ruta completa al archivo de texto delimitado. Para poder analizar los campos del archivo correctamente, el delimitador debe definirse antes de introducir el nombre del archivo. Utilice el botón Explorar situado a la derecha de este campo para seleccionar el archivo de entrada.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="171"/>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Explorar para buscar el archivo de texto delimitado a procesar</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="174"/>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Use este botón para buscar el archivo de texto. Este botón no se activará hasta que se haya introducido un delimitador en el recuadro &lt;i&gt; Delimitador&lt;/i&gt;. Una vez que se ha seleccionado un archivo, los cuadros combinados de las coordenadas X e Y se rellenarán con los campos del archivo de texto delimitado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="194"/>
        <source>Layer name</source>
        <translation>Nombre de la capa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="201"/>
        <source>Name to display in the map legend</source>
        <translation>Nombre para mostrar en la leyenda del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="204"/>
        <source>Name displayed in the map legend</source>
        <translation>Nombre mostrado en la leyenda del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="300"/>
        <source>Delimiter</source>
        <translation>Delimitador</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="321"/>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation>Delimitador a usar para dividir campos en el archivo de texto. El delimitador puede ser más de un carácter.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="324"/>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Delimitador a usar para dividir campos en el archivo de texto. El delimitador puede tener uno o más caracteres.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="177"/>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="52"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="334"/>
        <source>The delimiter is taken as is</source>
        <translation>El delimitador se toma tal como es</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="337"/>
        <source>Plain characters</source>
        <translation>Caracteres sencillos</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="347"/>
        <source>The delimiter is a regular expression</source>
        <translation>El delimitador es una expresión regular</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="350"/>
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
    <name>QgsEditReservedWordsBase</name>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="13"/>
        <source>Edit Reserved Words</source>
        <translation>Editar palabras reservadas</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="37"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Double click the Column Name column to change the name of the column.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Pulsar dos veces sobre &quot;Nombre de Columna&quot; para cambiar el nombre de la columna.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="47"/>
        <source>Status</source>
        <translation>Estado</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="52"/>
        <source>Column Name</source>
        <translation>Nombre de columna</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="57"/>
        <source>Index</source>
        <translation>Índice</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="82"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This shapefile contains reserved words. These may affect the import into PostgreSQL. Edit the column names so none of the reserved words listed at the right are used (click on a Column Name entry to edit). You may also change any other column name if desired.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Este archivo shape contiene palabras reservadas. Esto puede afectar a la importación a PostgreSQL. Edite los nombres de columna para que no se emplee ninguna de las palabras reservadas listadas a la derecha (pulsar sobre &quot;Nombre de Columna&quot; para editar). Si lo desea también puede cambiar cualquier otro nombre de columna.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="89"/>
        <source>Reserved Words</source>
        <translation>Palabras reservadas</translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsDialog</name>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsdialog.cpp" line="34"/>
        <source>Status</source>
        <translation>Estado</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsdialog.cpp" line="34"/>
        <source>Column Name</source>
        <translation>Nombre de columna</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsdialog.cpp" line="34"/>
        <source>Index</source>
        <translation>Índice</translation>
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
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="16"/>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="28"/>
        <source>Fill Style</source>
        <translation>Estilo de relleno</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="54"/>
        <source>col</source>
        <translation>col</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="61"/>
        <source>Colour:</source>
        <translation>Color:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="72"/>
        <source>PolyStyleWidget</source>
        <translation>Estilo del polígono (PolyStyleWidget)</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="43"/>
        <source>New device %1</source>
        <translation>Nuevo receptor %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="56"/>
        <source>Are you sure?</source>
        <translation>¿Está seguro?</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="57"/>
        <source>Are you sure that you want to delete this device?</source>
        <translation>¿Está seguro de que quiere borrar este receptor?</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="24"/>
        <source>GPS Device Editor</source>
        <translation>Editor de receptores GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="72"/>
        <source>New device</source>
        <translation>Nuevo receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="87"/>
        <source>Delete device</source>
        <translation>Borrar receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="102"/>
        <source>Update device</source>
        <translation>Actualizar receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="135"/>
        <source>Device name:</source>
        <translation>Nombre del receptor:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="150"/>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Así es como aparecerá el nombre del receptor en la lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="159"/>
        <source>Commands</source>
        <translation>Comandos</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="174"/>
        <source>Track download:</source>
        <translation>Descargar track:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="181"/>
        <source>Route upload:</source>
        <translation>Cargar ruta:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="188"/>
        <source>Waypoint download:</source>
        <translation>Descargar waypoint:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="195"/>
        <source>The command that is used to download routes from the device</source>
        <translation>El comando que se usa para descargar rutas del receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="202"/>
        <source>Route download:</source>
        <translation>Descargar ruta:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="209"/>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>El comando que se usa para cargar waypoints al receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="216"/>
        <source>Track upload:</source>
        <translation>Cargar track:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="223"/>
        <source>The command that is used to download tracks from the device</source>
        <translation>El comando que se usa para descargar tracks del receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="230"/>
        <source>The command that is used to upload routes to the device</source>
        <translation>El comando que se usa para cargar rutas al receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="237"/>
        <source>The command that is used to download waypoints from the device</source>
        <translation>El comando que se usa para descargar waypoints del receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="244"/>
        <source>The command that is used to upload tracks to the device</source>
        <translation>El comando que se usa para cargar tracks al receptor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="251"/>
        <source>Waypoint upload:</source>
        <translation>Cargar waypoint:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="269"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;En los comandos para descargar y cargar puede haber palabras especiales que serán reemplazadas por QGIS cuando se utilicen estos comandos. Estas palabras son:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - ruta de GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%dentro&lt;/span&gt; - el nombre del archivo GPX cuando se está cargando o el puerto cuando se está descargando&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%fuera&lt;/span&gt; - el puerto cuando se está cargando o el nombre del archivo GPX cuando se está descargando&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="301"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="92"/>
        <source>&amp;Gps Tools</source>
        <translation>Herramientas &amp;GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="93"/>
        <source>&amp;Create new GPX layer</source>
        <translation>&amp;Crear nueva capa GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="96"/>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Crea una capa GPX nueva y la muestra en la vista del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="195"/>
        <source>&amp;Gps</source>
        <translation>&amp;GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="159"/>
        <source>Save new GPX file as...</source>
        <translation>Guardar nuevo archivo GPX como...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="159"/>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>Archivo GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="164"/>
        <source>Could not create file</source>
        <translation>No se pudo crear el archivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="166"/>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>No se puede crear un archivo GPX con este nombre. </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="167"/>
        <source>Try again with another name or in another </source>
        <translation>Inténtelo de nuevo con otro nombre o en otro </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="167"/>
        <source>directory.</source>
        <translation>directorio.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="206"/>
        <source>GPX Loader</source>
        <translation>Cargador de GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="208"/>
        <source>Unable to read the selected file.
</source>
        <translation>No se puede leer el archivo seleccionado.
</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="208"/>
        <source>Please reselect a valid file.</source>
        <translation>Por favor, vuelva a seleccionar un archivo válido.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="491"/>
        <source>Could not start process</source>
        <translation>No se pudo iniciar el proceso</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="492"/>
        <source>Could not start GPSBabel!</source>
        <translation>No se pudo iniciar GPSBabel</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="330"/>
        <source>Importing data...</source>
        <translation>Importando datos...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="497"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="275"/>
        <source>Could not import data from %1!

</source>
        <translation>No se pudieron importar datos de %1

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="277"/>
        <source>Error importing data</source>
        <translation>Error al importar datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="481"/>
        <source>Not supported</source>
        <translation>No soportado</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="397"/>
        <source>This device does not support downloading </source>
        <translation>Este receptor no soporta la descarga </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="397"/>
        <source>of </source>
        <translation>de </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="412"/>
        <source>Downloading data...</source>
        <translation>Descargando datos...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="426"/>
        <source>Could not download data from GPS!

</source>
        <translation>No se pudieron descargar datos del GPS

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="428"/>
        <source>Error downloading data</source>
        <translation>Error al descargar datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="482"/>
        <source>This device does not support uploading of </source>
        <translation>Este receptor no soporta el cargar datos de </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="497"/>
        <source>Uploading data...</source>
        <translation>Cargando datos...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="511"/>
        <source>Error while uploading data to GPS!

</source>
        <translation>Error al cargar datos al GPS

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="513"/>
        <source>Error uploading data</source>
        <translation>Error al cargar datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="345"/>
        <source>Could not convert data from %1!

</source>
        <translation>¡No se pudieron convertir los datos desde %1!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="347"/>
        <source>Error converting data</source>
        <translation>Error al convertir datos</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="445"/>
        <source>Choose a filename to save under</source>
        <translation>Seleccione un nombre de archivo para guardar en</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="447"/>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>formato GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="434"/>
        <source>Select GPX file</source>
        <translation>Seleccionar archivo GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="240"/>
        <source>Select file and format to import</source>
        <translation>Seleccionar archivo y formato a importar</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="258"/>
        <source>Waypoints</source>
        <translation>Waypoints</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="260"/>
        <source>Routes</source>
        <translation>Rutas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="262"/>
        <source>Tracks</source>
        <translation>Tracks</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="408"/>
        <source>Route -&gt; Waypoint</source>
        <translation>Ruta -&gt; Waypoint</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="409"/>
        <source>Waypoint -&gt; Route</source>
        <translation>Waypoint -&gt; Ruta</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="414"/>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation>QGIS puede realizar conversiones de archivos GPX, usando GPSBabel (%1) para hacer las conversiones.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="415"/>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation>Esto requiere que tenga instalado GPSBabel donde QGIS lo pueda encontrar</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="416"/>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX filename that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation>Seleccione un nombre de archivo GPX de entrada, el tipo de conversión que quiere realizar, un nombre de archivo GPX con el que quiera guardar el archivo convertido y un nombre para la nueva capa creada a partir del resultado.</translation>
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
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="95"/>
        <source>File:</source>
        <translation>Archivo:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="112"/>
        <source>Feature types:</source>
        <translation>Tipo de objetos espaciales:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="317"/>
        <source>Waypoints</source>
        <translation>Waypoints</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="322"/>
        <source>Routes</source>
        <translation>Rutas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="327"/>
        <source>Tracks</source>
        <translation>Tracks</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="159"/>
        <source>Import other file</source>
        <translation>Importar otro archivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="560"/>
        <source>Layer name:</source>
        <translation>Nombre de la capa:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="584"/>
        <source>GPX output file:</source>
        <translation>Archivo de salida GPX:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="342"/>
        <source>Feature type:</source>
        <translation>Tipo de objeto espacial:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="266"/>
        <source>File to import:</source>
        <translation>Archivo a importar:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="274"/>
        <source>Download from GPS</source>
        <translation>Descargar desde GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="485"/>
        <source>Port:</source>
        <translation>Puerto:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="359"/>
        <source>Output file:</source>
        <translation>Archivo de salida:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="519"/>
        <source>GPS device:</source>
        <translation>Receptor GPS:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="512"/>
        <source>Edit devices</source>
        <translation>Editar receptores</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="433"/>
        <source>Upload to GPS</source>
        <translation>Cargar al GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="526"/>
        <source>Data layer:</source>
        <translation>Capa de datos:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="84"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;GPX is the &lt;/span&gt;&lt;a href=&quot;http://www.topografix.com/gpx.asp&quot;&gt;&lt;span style=&quot; font-size:10pt; text-decoration: underline; color:#0000ff;&quot;&gt;GPS eXchange file format&lt;/span&gt;&lt;/a&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;, which is used to store information about waypoints, routes, and tracks.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Select a GPX file and then select the feature types that you want to load.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;GPX es el &lt;/span&gt;&lt;a href=&quot;http://www.topografix.com/gpx.asp&quot;&gt;&lt;span style=&quot; font-size:10pt; text-decoration: underline; color:#0000ff;&quot;&gt;formato de archivo de intercambio (eXchange) de GPS&lt;/span&gt;&lt;/a&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;, que se usa para guardar información sobre waypoints, rutas y tracks.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Seleccione un archivo GPX y luego elija el tipo de objetos espaciales que quiere cargar.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="574"/>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="174"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (&lt;/span&gt;&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; font-size:10pt; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;). This requires that you have GPSBabel installed where QGIS can find it.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer. All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;QGIS sólo puede cargar archivos GPX por si mismo, pero otros muchos formatos se pueden convertir a GPX usando GPSBabel (&lt;/span&gt;&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; font-size:10pt; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;). Esto requiere que tenga instalado GPSBabel donde QGIS lo pueda encontrar.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Seleccione un formato de archivo de GPS y el archivo que quiere importar, el tipo de objeto espacial que quiere usar, un nombre de archivo con el que quiera guardar el archivo convertido a GPX y un nombre para la nueva capa. No todos los formatos de archivo pueden guardar waypoints, rutas y tracks, por lo que algunos tipos de objeto espacial pueden estar deshabilitados para algunos formatos de archivo.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="567"/>
        <source>Save As...</source>
        <translation>Guardar como...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="289"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This tool will help you download data from a GPS device. Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data. If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;This tool uses the program GPSBabel (&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;) to transfer the data. If you don&apos;t have GPSBabel installed where QGIS can find it, this tool will not work.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Esta herramienta le ayudará a descargar datos de un receptor GPS. Elija su receptor GPS, el puerto al que está conectado, el tipo de objeto espacial que quiere descargar, un nombre para su nueva capa y el archivo GPX donde quiera guardar los datos. Si su receptor no está en la lista o si quiere cambiar alguna configuración, también puede editar los receptores.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Esta herramienta usa el programa GPSBabel (&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;) para transferir los datos. Si no tiene instalado GPSBabel donde QGIS lo pueda encontrar, esta herramienta no funcionará.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="464"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This tool will help you upload data from a GPX layer to a GPS device. Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to. If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;This tool uses the program GPSBabel (&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;) to transfer the data. If you don&apos;t have GPSBabel installed where QGIS can find it, this tool will not work.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Esta herramienta le ayudará a cargar datos de una capa GPX a un receptor GPS. Seleccione la capa que quiere cargar , el receptor en el que la quiere cargar y el puerto al que está conectado. Si su receptor no está en la lista o si quiere cambiar alguna configuración, también puede editar los receptores.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Esta herramienta usa el programa GPSBabel (&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;) para transferir los datos. Si no tiene instalado GPSBabel donde QGIS lo pueda encontrar, esta herramienta no funcionará.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="252"/>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation>(Nota: ¡Seleccionar el tipo de archivo correcto en el diálogo de exploración es importante!)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="535"/>
        <source>GPX Conversions</source>
        <translation>Conversiones GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="617"/>
        <source>Conversion:</source>
        <translation>Conversión:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="631"/>
        <source>GPX input file:</source>
        <translation>Archivo GPX de entrada:</translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="68"/>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>URI errónea - necesita especificar el tipo de objeto espacial.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="112"/>
        <source>GPS eXchange file</source>
        <translation>Archivo GPS eXchange</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="729"/>
        <source>Digitized in QGIS</source>
        <translation>Digitalizado en QGIS</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="31"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="32"/>
        <source>Type</source>
        <translation>Tipo</translation>
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
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="28"/>
        <source>File Format:</source>
        <translation>Formato de archivo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="41"/>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="64"/>
        <source>Attributes:</source>
        <translation>Atributos:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="74"/>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="81"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="93"/>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="100"/>
        <source>Line</source>
        <translation>Línea</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="107"/>
        <source>Polygon</source>
        <translation>Polígono</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="118"/>
        <source>Column 1</source>
        <translation>Columna 1</translation>
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
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="322"/>
        <source>Equal Interval</source>
        <translation>Intervalo igual</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="299"/>
        <source>Quantiles</source>
        <translation>Cuantiles</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="346"/>
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
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="73"/>
        <source>Classification Field:</source>
        <translation>Campo de clasificación:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="89"/>
        <source>Mode:</source>
        <translation>Modo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="105"/>
        <source>Number of Classes:</source>
        <translation>Número de clases:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="159"/>
        <source>Delete class</source>
        <translation>Borrar clase</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="166"/>
        <source>Classify</source>
        <translation>Clasificar</translation>
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
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="454"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="290"/>
        <source>Cannot copy map </source>
        <translation>No se puede copiar el mapa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="412"/>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt;comando: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="356"/>
        <source>Cannot rename map </source>
        <translation>No se puede cambiar el nombre del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="394"/>
        <source>Delete map &lt;b&gt;</source>
        <translation>Borrar mapa &lt;b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="411"/>
        <source>Cannot delete map </source>
        <translation>No se puede borrar el mapa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="455"/>
        <source>Cannot write new region</source>
        <translation>No se puede guardar una nueva región</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="340"/>
        <source>New name</source>
        <translation>Nuevo nombre</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1449"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="233"/>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>No es propietario del directorio de mapas, no puede abrir el vectorial para editarlo.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="238"/>
        <source>Cannot open vector for update.</source>
        <translation>No se puede abrir el vectorial para actualizar.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="246"/>
        <source>Edit tools</source>
        <translation>Herramientas de edición</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="249"/>
        <source>New point</source>
        <translation>Nuevo punto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="255"/>
        <source>New line</source>
        <translation>Nueva línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="261"/>
        <source>New boundary</source>
        <translation>Nuevo contorno</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="267"/>
        <source>New centroid</source>
        <translation>Nuevo centroide</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="273"/>
        <source>Move vertex</source>
        <translation>Mover vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="279"/>
        <source>Add vertex</source>
        <translation>Añadir vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="285"/>
        <source>Delete vertex</source>
        <translation>Borrar vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="291"/>
        <source>Move element</source>
        <translation>Mover elemento</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="297"/>
        <source>Split line</source>
        <translation>Dividir línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="303"/>
        <source>Delete element</source>
        <translation>Borrar elemento</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="309"/>
        <source>Edit attributes</source>
        <translation>Editar atributos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="314"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="697"/>
        <source>Info</source>
        <translation>Info</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="697"/>
        <source>The table was created</source>
        <translation>La tabla ha sido creada</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1322"/>
        <source>Tool not yet implemented.</source>
        <translation>Herramienta no implementada todavía.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1348"/>
        <source>Cannot check orphan record: </source>
        <translation>No se puede comprobar registro huérfano: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1355"/>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation>El registro huérfano se dejó en la tabla de atributos. &lt;br&gt;¿Borrar el registro?</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1364"/>
        <source>Cannot delete orphan record: </source>
        <translation>No se puede borrar el registro huérfano: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1392"/>
        <source>Cannot describe table for field </source>
        <translation>No se puede describir la tabla por el campo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1812"/>
        <source>Left: </source>
        <translation>Izquierda: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1813"/>
        <source>Middle: </source>
        <translation>Medio: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="395"/>
        <source>Background</source>
        <translation>Fondo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="396"/>
        <source>Highlight</source>
        <translation>Resaltado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="397"/>
        <source>Dynamic</source>
        <translation>Dinámico</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="398"/>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="399"/>
        <source>Line</source>
        <translation>Línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="400"/>
        <source>Boundary (no area)</source>
        <translation>Contorno (sin área)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="401"/>
        <source>Boundary (1 area)</source>
        <translation>Contorno (1 área)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="402"/>
        <source>Boundary (2 areas)</source>
        <translation>Contorno (2 áreas)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="403"/>
        <source>Centroid (in area)</source>
        <translation>Centroide (en el área)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="404"/>
        <source>Centroid (outside area)</source>
        <translation>Centroide (fuera del área)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="405"/>
        <source>Centroid (duplicate in area)</source>
        <translation>Centroide (duplicado en el área)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="406"/>
        <source>Node (1 line)</source>
        <translation>Nodo (1 línea)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="407"/>
        <source>Node (2 lines)</source>
        <translation>Nodo (2 líneas)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="440"/>
        <source>Disp</source>
        <comment>Column title</comment>
        <translation>Título</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="476"/>
        <source>Column</source>
        <translation>Columna</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="477"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="478"/>
        <source>Length</source>
        <translation>Longitud</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="533"/>
        <source>Next not used</source>
        <translation>El siguiente no se usa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="534"/>
        <source>Manual entry</source>
        <translation>Entrada manual</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="535"/>
        <source>No category</source>
        <translation>Ninguna categoría</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1814"/>
        <source>Right: </source>
        <translation>Derecha: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="442"/>
        <source>Color</source>
        <comment>Column title</comment>
        <translation>Color</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="444"/>
        <source>Type</source>
        <comment>Column title</comment>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="446"/>
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
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="131"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="162"/>
        <source>Ok</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="167"/>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;¡Introduzca un nombre!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="178"/>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;¡Este es el nombre del origen!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="184"/>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;¡Existe!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="185"/>
        <source>Overwrite</source>
        <translation>Sobrescribir</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="110"/>
        <source>Mapcalc tools</source>
        <translation>Herramientas Mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="113"/>
        <source>Add map</source>
        <translation>Añadir mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="120"/>
        <source>Add constant value</source>
        <translation>Añadir constante</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="127"/>
        <source>Add operator or function</source>
        <translation>Añadir operador o función</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="134"/>
        <source>Add connection</source>
        <translation>Añadir conexión</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="141"/>
        <source>Select item</source>
        <translation>Seleccionar ítem</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="148"/>
        <source>Delete selected item</source>
        <translation>Borrar ítem seleccionado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="158"/>
        <source>Open</source>
        <translation>Abrir</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="163"/>
        <source>Save</source>
        <translation>Guardar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="169"/>
        <source>Save as</source>
        <translation>Guardar como</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="177"/>
        <source>Addition</source>
        <translation>Suma</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="178"/>
        <source>Subtraction</source>
        <translation>Resta</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="179"/>
        <source>Multiplication</source>
        <translation>Multiplicación</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="180"/>
        <source>Division</source>
        <translation>División</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="181"/>
        <source>Modulus</source>
        <translation>Módulo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="182"/>
        <source>Exponentiation</source>
        <translation>Exponencial</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="185"/>
        <source>Equal</source>
        <translation>Igual</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="186"/>
        <source>Not equal</source>
        <translation>Distinto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="187"/>
        <source>Greater than</source>
        <translation>Mayor que</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="188"/>
        <source>Greater than or equal</source>
        <translation>Mayor o igual que</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="189"/>
        <source>Less than</source>
        <translation>Menor que</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="190"/>
        <source>Less than or equal</source>
        <translation>Menor o igual que</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="191"/>
        <source>And</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="192"/>
        <source>Or</source>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="195"/>
        <source>Absolute value of x</source>
        <translation>Valor absoluto de x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="196"/>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>Inverso de la tangente de x (resultado en grados)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="197"/>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>Inverso de la tangente de y/x (resultado en grados)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="198"/>
        <source>Current column of moving window (starts with 1)</source>
        <translation>Columna actual de la ventana movible (empieza con 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="199"/>
        <source>Cosine of x (x is in degrees)</source>
        <translation>Coseno de x (x en grados)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="200"/>
        <source>Convert x to double-precision floating point</source>
        <translation>Convertir x a coma flotante (precisión doble)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="201"/>
        <source>Current east-west resolution</source>
        <translation>Resolución actual Este-Oeste</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="202"/>
        <source>Exponential function of x</source>
        <translation>Elevado a x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="203"/>
        <source>x to the power y</source>
        <translation>x elevado a y</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="204"/>
        <source>Convert x to single-precision floating point</source>
        <translation>Convertir x a coma flotante (precisión simple)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="205"/>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>Decisión: 1 si x no es cero, 0 para el resto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="206"/>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>Decisión: a si x no es cero, 0 para el resto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="207"/>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>Decisión: a si x no es cero, b para el resto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="208"/>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>Decisión: a si x &gt; 0, b si x es cero, c si x &lt; 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="209"/>
        <source>Convert x to integer [ truncates ]</source>
        <translation>Convertir x a entero [ truncar ]</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="210"/>
        <source>Check if x = NULL</source>
        <translation>Comprobar si x = NULO</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="211"/>
        <source>Natural log of x</source>
        <translation>Logaritmo neperiano de x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="212"/>
        <source>Log of x base b</source>
        <translation>Logaritmo de x en base b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="214"/>
        <source>Largest value</source>
        <translation>Máximo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="216"/>
        <source>Median value</source>
        <translation>Mediana</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="218"/>
        <source>Smallest value</source>
        <translation>Mínimo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="220"/>
        <source>Mode value</source>
        <translation>Moda</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="221"/>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1 si x = 0, 0 para el resto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="222"/>
        <source>Current north-south resolution</source>
        <translation>Resolución Norte-Sur actual</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="223"/>
        <source>NULL value</source>
        <translation>Valor NULO</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="224"/>
        <source>Random value between a and b</source>
        <translation>Valor aleatorio entre a y b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="225"/>
        <source>Round x to nearest integer</source>
        <translation>Redondear x al número entero más próximo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="226"/>
        <source>Current row of moving window (Starts with 1)</source>
        <translation>Fila actual de la ventana movible (empieza con 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="227"/>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation>Seno de x (x en grados)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="228"/>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>Raíz cuadrada de x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="229"/>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation>Tangente de x (x en grados)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="230"/>
        <source>Current x-coordinate of moving window</source>
        <translation>Coordenada X de la ventana movible</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="231"/>
        <source>Current y-coordinate of moving window</source>
        <translation>Coordenada Y de la ventana movible</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1317"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="583"/>
        <source>Cannot get current region</source>
        <translation>No se puede estimar la región actual</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="560"/>
        <source>Cannot check region of map </source>
        <translation>No se puede comprobar la región del mapa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="616"/>
        <source>Cannot get region of map </source>
        <translation>No se puede obtener la región del mapa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="812"/>
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
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1367"/>
        <source>Run</source>
        <translation>Ejecutar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1345"/>
        <source>Stop</source>
        <translation>Detener</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="192"/>
        <source>Module</source>
        <translation>Módulo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1339"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="207"/>
        <source>The module file (</source>
        <translation>El archivo del módulo (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="207"/>
        <source>) not found.</source>
        <translation>) no se ha encontrado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="211"/>
        <source>Cannot open module file (</source>
        <translation>No se puede abrir el archivo del módulo (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="979"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="974"/>
        <source>Cannot read module file (</source>
        <translation>No se puede leer el archivo del módulo (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="974"/>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="975"/>
        <source>
at line </source>
        <translation>
en la línea </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="247"/>
        <source>Module </source>
        <translation>Módulo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="247"/>
        <source> not found</source>
        <translation> no encontrado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="288"/>
        <source>Cannot find man page </source>
        <translation>No se puede encontrar la página del manual </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="968"/>
        <source>Not available, cannot open description (</source>
        <translation>No disponible, no se puede abrir la descripción (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="975"/>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="979"/>
        <source>Not available, incorrect description (</source>
        <translation>No disponible, descripción incorrecta (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1166"/>
        <source>Cannot get input region</source>
        <translation>No se puede obtener la región de entrada</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1154"/>
        <source>Use Input Region</source>
        <translation>Usar región de entrada</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1268"/>
        <source>Cannot find module </source>
        <translation>No se puede encontrar el módulo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1340"/>
        <source>Cannot start module: </source>
        <translation>No se puede iniciar el módulo: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1356"/>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation>&lt;B&gt;Finalizado correctamente&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1362"/>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation>&lt;B&gt;Finalizado con error&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1365"/>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation>&lt;B&gt;Módulo finalizado o matado&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="965"/>
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
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2702"/>
        <source>Attribute field</source>
        <translation>Campo de atributos</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2904"/>
        <source>File</source>
        <translation>Archivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3017"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;falta el valor</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3024"/>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation>:&amp;nbsp;el directorio no existe</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2653"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2515"/>
        <source>Cannot find layeroption </source>
        <translation>No se puede encontrar la opción capa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2657"/>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation>El controlador PostGIS de OGR no soporta esquemas.&lt;br&gt;Sólo se usará el nombre de la tabla.&lt;br&gt;Esto puede dar como resultado entradas incorrectas si hay más tablas con el mismo nombre&lt;br&gt;en la base de datos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2680"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;ninguna entrada</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2528"/>
        <source>Cannot find whereoption </source>
        <translation>No se puede encontrar la opción dónde </translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2059"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1972"/>
        <source>Cannot find typeoption </source>
        <translation>No se puede encontrar la opción tipo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1981"/>
        <source>Cannot find values for typeoption </source>
        <translation>No se pueden encontrar valores para la opción tipo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2042"/>
        <source>Cannot find layeroption </source>
        <translation>No se puede encontrar la opción capa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2059"/>
        <source>GRASS element </source>
        <translation>Elemento de GRASS </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2059"/>
        <source> not supported</source>
        <translation> no soportado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2083"/>
        <source>Use region of this map</source>
        <translation>Usar la región de este mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2419"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;ninguna entrada</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1888"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;falta el valor</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2791"/>
        <source>Attribute field</source>
        <translation>Campo de atributo</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="873"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="356"/>
        <source>Cannot find module </source>
        <translation>No se puede encontrar el módulo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="373"/>
        <source>Cannot start module </source>
        <translation>No se puede iniciar el módulo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="386"/>
        <source>Cannot read module description (</source>
        <translation>No se puede leer la descripción del módulo (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="386"/>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="387"/>
        <source>
at line </source>
        <translation>
en la línea </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="387"/>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="411"/>
        <source>Cannot find key </source>
        <translation>No se puede encontrar la clave </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="547"/>
        <source>Item with id </source>
        <translation>Ítem con ID </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="547"/>
        <source> not found</source>
        <translation> no encontrado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="835"/>
        <source>Cannot get current region</source>
        <translation>No se puede obtener la región actual</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="812"/>
        <source>Cannot check region of map </source>
        <translation>No se puede comprobar la región del mapa </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="874"/>
        <source>Cannot set region of map </source>
        <translation>No se puede establecer la región del mapa </translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="114"/>
        <source>GRASS database</source>
        <translation>Base de datos de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="115"/>
        <source>GRASS location</source>
        <translation>Localización de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="116"/>
        <source>Projection</source>
        <translation>Proyección</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="117"/>
        <source>Default GRASS Region</source>
        <translation>Región predeterminada de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="174"/>
        <source>Mapset</source>
        <translation>Directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="119"/>
        <source>Create New Mapset</source>
        <translation>Crear nuevo directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="148"/>
        <source>Tree</source>
        <translation>Árbol</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="149"/>
        <source>Comment</source>
        <translation>Comentario</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="150"/>
        <source>Database</source>
        <translation>Base de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="154"/>
        <source>Location 2</source>
        <translation>Localización 2</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="165"/>
        <source>User&apos;s mapset</source>
        <translation>Directorio de mapas del usuario</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="167"/>
        <source>System mapset</source>
        <translation>Directorio de mapas del sistema</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="161"/>
        <source>Location 1</source>
        <translation>Localización 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="175"/>
        <source>Owner</source>
        <translation>Propietario</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="224"/>
        <source>Enter path to GRASS database</source>
        <translation>Introduzca la ruta a la base de datos de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="232"/>
        <source>The directory doesn&apos;t exist!</source>
        <translation>¡El directorio no existe!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="262"/>
        <source>No writable locations, the database not writable!</source>
        <translation>¡Ninguna localización que se pueda escribir, la base de datos no se puede escribir!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="360"/>
        <source>Enter location name!</source>
        <translation>¡Introduzca el nombre de la localización!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="373"/>
        <source>The location exists!</source>
        <translation>¡La localización existe!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="521"/>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>¡La proyección seleccionada no está soportada por GRASS!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1148"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="568"/>
        <source>Cannot create projection.</source>
        <translation>No se puede crear la proyección.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="617"/>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation>No se puede reproyectar la región establecida previamente, se establece la región predeterminada.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="752"/>
        <source>North must be greater than south</source>
        <translation>El Norte debe ser mayor que el Sur</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="757"/>
        <source>East must be greater than west</source>
        <translation>El Este debe ser mayor que el Oeste</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="804"/>
        <source>Regions file (</source>
        <translation>Archivo de regiones (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="804"/>
        <source>) not found.</source>
        <translation>) no encontrado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="809"/>
        <source>Cannot open locations file (</source>
        <translation>No se puede abrir el archivo de localizaciones (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="809"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="818"/>
        <source>Cannot read locations file (</source>
        <translation>No se puede leer el archivo de localizaciones (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="819"/>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="819"/>
        <source>
at line </source>
        <translation>
en la línea </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="820"/>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1149"/>
        <source>Cannot create QgsSpatialRefSys</source>
        <translation>No se puede crear QgsSpatialRefSys</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="956"/>
        <source>Cannot reproject selected region.</source>
        <translation>No se puede reproyectar la región seleccionada.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1045"/>
        <source>Cannot reproject region</source>
        <translation>No se puede reproyectar la región</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1277"/>
        <source>Enter mapset name.</source>
        <translation>Introduzca el nombre del directorio de mapas.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1294"/>
        <source>The mapset already exists</source>
        <translation>El directorio de mapas ya existe</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1318"/>
        <source>Database: </source>
        <translation>Base de datos: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1329"/>
        <source>Location: </source>
        <translation>Localización: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1331"/>
        <source>Mapset: </source>
        <translation>Directorio de mapas: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1362"/>
        <source>Create location</source>
        <translation>Crear localización</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1364"/>
        <source>Cannot create new location: </source>
        <translation>No se puede crear la nueva localización: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1411"/>
        <source>Create mapset</source>
        <translation>Crear directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1404"/>
        <source>Cannot open DEFAULT_WIND</source>
        <translation>No se puede abrir DEFAULT_WIND</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1411"/>
        <source>Cannot open WIND</source>
        <translation>No se puede abrir WIND</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1438"/>
        <source>New mapset</source>
        <translation>Nuevo directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1434"/>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>El nuevo directorio de mapas de creó correctamente, pero no se puede abrir: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1440"/>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>El nuevo directorio de mapas se creó correctamente y se estableció como el directorio de mapas de trabajo actual.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1394"/>
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
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="158"/>
        <source>Open mapset</source>
        <translation>Abrir directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="159"/>
        <source>New mapset</source>
        <translation>Nuevo directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="160"/>
        <source>Close mapset</source>
        <translation>Cerrar directorio de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="163"/>
        <source>Add GRASS vector layer</source>
        <translation>Añadir capa vectorial de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="165"/>
        <source>Add GRASS raster layer</source>
        <translation>Añadir capa ráster de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="181"/>
        <source>Open GRASS tools</source>
        <translation>Abrir herramientas de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="170"/>
        <source>Display Current Grass Region</source>
        <translation>Mostrar región actual de Grass</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="174"/>
        <source>Edit Current Grass Region</source>
        <translation>Editar la región actual de Grass</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="176"/>
        <source>Edit Grass Vector layer</source>
        <translation>Editar capa vectorial de Grass</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="179"/>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>Añade una capa vectorial de GRASS a la vista del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="180"/>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>Añade una capa ráster de GRASS a la vista del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="182"/>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Muestra la región actual de GRASS como un rectángulo en la vista del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="183"/>
        <source>Edit the current GRASS region</source>
        <translation>Editar la región actual de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="184"/>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Editar la capa vectorial de GRASS seleccionada actualmente.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="811"/>
        <source>&amp;GRASS</source>
        <translation>&amp;GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="211"/>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="93"/>
        <source>GrassVector</source>
        <translation>VectorialDeGrass</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="94"/>
        <source>0.1</source>
        <translation>0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="95"/>
        <source>GRASS layer</source>
        <translation>Capa de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="177"/>
        <source>Create new Grass Vector</source>
        <translation>Crear nuevo vectorial de Grass</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="780"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="488"/>
        <source>GRASS Edit is already running.</source>
        <translation>Le edición de GRASS ya se está ejecutando.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="497"/>
        <source>New vector name</source>
        <translation>Nombre del nuevo vectorial</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="513"/>
        <source>Cannot create new vector: </source>
        <translation>No se puede crear el nuevo vectorial: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="535"/>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation>Se creó el nuevo vectorial pero el proveedor de datos no lo puede abrir.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="546"/>
        <source>Cannot start editing.</source>
        <translation>No se puede iniciar la edición.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="583"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>BASE DE DATOS, NOMBRE_LOCALIZACIÓN o DIRECTORIO DE MAPAS no están establecidos, no se puede mostrar la región actual.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="593"/>
        <source>Cannot read current region: </source>
        <translation>No se puede leer la región actual: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="697"/>
        <source>Cannot open the mapset. </source>
        <translation>No se abrir el directorio de mapas. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="715"/>
        <source>Cannot close mapset. </source>
        <translation>No se puede cerrar el directorio de mapas. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="771"/>
        <source>Cannot close current mapset. </source>
        <translation>No se puede cerrar el directorio de mapas actual. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="780"/>
        <source>Cannot open GRASS mapset. </source>
        <translation>No se puede abrir el directorio de mapas de GRASS. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="419"/>
        <source>Could not add raster layer: </source>
        <translation>No se pudo añadir la capa ráster: </translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="460"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="196"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>BASE DE DATOS, NOMBRE_LOCALIZACIÓN o DIRECTORIO DE MAPAS no están establecidos, no se puede mostrar la región actual.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="203"/>
        <source>Cannot read current region: </source>
        <translation>No se puede leer la región actual: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="460"/>
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
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="408"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="408"/>
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
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="117"/>
        <source>Modules</source>
        <translation>Módulos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="149"/>
        <source>Browser</source>
        <translation>Explorador</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="99"/>
        <source>GRASS Tools</source>
        <translation>Herramientas de GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="376"/>
        <source>GRASS Tools: </source>
        <translation>Herramientas de GRASS: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="298"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="211"/>
        <source>Cannot find MSYS (</source>
        <translation>No se puede encontrar MSYS (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="233"/>
        <source>GRASS Shell is not compiled.</source>
        <translation>La consola de GRASS no está compilada.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="283"/>
        <source>The config file (</source>
        <translation>El archivo de configuración (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="283"/>
        <source>) not found.</source>
        <translation>) no se ha encontrado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="287"/>
        <source>Cannot open config file (</source>
        <translation>No se puede abrir el archivo de configuración (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="287"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="295"/>
        <source>Cannot read config file (</source>
        <translation>No se puede leer el archivo de configuración (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="296"/>
        <source>
at line </source>
        <translation>
en la línea </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="296"/>
        <source> column </source>
        <translation> columna </translation>
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
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="101"/>
        <source>QGIS - Grid Maker</source>
        <translation>QGIS - Creador de cuadrículas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="62"/>
        <source>Longitude Interval is invalid - please correct and try again.</source>
        <translation>El intervalo de la longitud no es válido - por favor, corríjalo y pruebe de nuevo.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="70"/>
        <source>Latitude Interval is invalid - please correct and try again.</source>
        <translation>El intervalo de la latitud no es válido - Por favor, corríjalo y pruebe de nuevo.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="78"/>
        <source>Longitude Origin is invalid - please correct and try again..</source>
        <translation>El origen de la longitud no es válido - Por favor, corríjalo y pruebe de nuevo.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="86"/>
        <source>Latitude Origin is invalid - please correct and try again.</source>
        <translation>El origen de la latitud no es válido - Por favor, corríjalo y pruebe de nuevo.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="94"/>
        <source>End Point Longitude is invalid - please correct and try again.</source>
        <translation>La longitud del punto final no es válida - Por favor, corríjala y pruebe de nuevo.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="102"/>
        <source>End Point Latitude is invalid - please correct and try again.</source>
        <translation>La latitud del punto final no es válida - Por favor, corríjala y pruebe de nuevo.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="162"/>
        <source>Choose a filename to save under</source>
        <translation>Seleccione un nombre para guardar el archivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="164"/>
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
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation>Plantilla de complementos de QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="39"/>
        <source>Graticule Builder</source>
        <translation>Generador de cuadrículas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="94"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:10pt;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Este complemento le ayudará a construir un archivo shape de cuadrícula que podrá usar como guía en su vista del mapa de QGIS.&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:10pt;&quot;&gt;Introduzca todas las unidades en grados decimales&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="101"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="113"/>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="123"/>
        <source>Line</source>
        <translation>Línea</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="130"/>
        <source>Polygon</source>
        <translation>Polígono</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="140"/>
        <source>Origin (lower left)</source>
        <translation>Origen (esquina inferior izquierda)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="195"/>
        <source>Latitude:</source>
        <translation>Latitud:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="259"/>
        <source>#000.00000; </source>
        <translation>#000.00000; </translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="209"/>
        <source>Longitude:</source>
        <translation>Longitud:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="183"/>
        <source>End point (upper right)</source>
        <translation>Punto final (esquina superior derecha)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="226"/>
        <source>Graticle size (units in degrees)</source>
        <translation>Tamaño de la cuadrícula (unidades en grados)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="238"/>
        <source>Latitude Interval:</source>
        <translation>Intervalo de la latitud:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="252"/>
        <source>Longitude Interval:</source>
        <translation>Intervalo de la longitud:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="269"/>
        <source>Output (shape) file</source>
        <translation>Archivo de salida (shape)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="284"/>
        <source>Save As...</source>
        <translation>Guardar como...</translation>
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
        <location filename="../src/ui/qgshelpviewerbase.ui" line="16"/>
        <source>QGIS Help</source>
        <translation>Ayuda de QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="81"/>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="84"/>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="42"/>
        <source>&amp;Home</source>
        <translation>&amp;Inicio</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="45"/>
        <source>Alt+H</source>
        <translation>Alt+I</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="55"/>
        <source>&amp;Forward</source>
        <translation>A&amp;delante</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="58"/>
        <source>Alt+F</source>
        <translation>Alt+D</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="68"/>
        <source>&amp;Back</source>
        <translation>A&amp;trás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="71"/>
        <source>Alt+B</source>
        <translation>Alt+T</translation>
    </message>
</context>
<context>
    <name>QgsHttpTransaction</name>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="230"/>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation>El servidor WMS ha respondido de forma inesperada con el código de estado HTTP %1 (%2)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="309"/>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>Respuesta HTTP completada, sin embargo ha habido un error: %1</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="358"/>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>Transacción HTTP completada, sin embargo ha habido un error: %1</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/core/qgshttptransaction.cpp" line="437"/>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation type="unfinished">
            <numerusform>La conexión a la red terminó después de %1 segundos de inactividad.
Esto puede ser debido a un problema en la conexión a red o en el servidor WMS.</numerusform>
        </translation>
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
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="16"/>
        <source>Identify Results</source>
        <translation>Resultados de la identificación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="46"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="49"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="75"/>
        <source>Close</source>
        <translation>Cerrar</translation>
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
        <location filename="../src/ui/qgsludialogbase.ui" line="31"/>
        <source>Lower value</source>
        <translation>Valor más bajo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="57"/>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="94"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="101"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="126"/>
        <source>Upper value</source>
        <translation>Valor más alto</translation>
    </message>
</context>
<context>
    <name>QgsLabelDialogBase</name>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="21"/>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="78"/>
        <source>Font Style</source>
        <translation>Estilo de fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="103"/>
        <source>Font size units</source>
        <translation>Unidades del tamaño de fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="579"/>
        <source>Map units</source>
        <translation>Unidades del mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="586"/>
        <source>Points</source>
        <translation>Puntos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="980"/>
        <source>Transparency:</source>
        <translation>Transparencia:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="183"/>
        <source>Font</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="534"/>
        <source>Colour</source>
        <translation>Color</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="478"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="236"/>
        <source>Font Alignment</source>
        <translation>Alineación de fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="261"/>
        <source>Placement</source>
        <translation>Ubicación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="273"/>
        <source>Below Right</source>
        <translation>Abajo derecha</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="280"/>
        <source>Right</source>
        <translation>Derecha</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="287"/>
        <source>Below</source>
        <translation>Abajo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="294"/>
        <source>Over</source>
        <translation>Sobre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="304"/>
        <source>Above</source>
        <translation>Encima de</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="311"/>
        <source>Left</source>
        <translation>Izquierda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="318"/>
        <source>Below Left</source>
        <translation>Abajo izquierda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="325"/>
        <source>Above Right</source>
        <translation>Encima derecha</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="332"/>
        <source>Above Left</source>
        <translation>Abajo izquierda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="893"/>
        <source>Angle (deg):</source>
        <translation>Ángulo (grados):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="384"/>
        <source>Buffer</source>
        <translation>Margen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="396"/>
        <source>Buffer size units</source>
        <translation>Unidades de tamaño del margen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="408"/>
        <source>Size is in map units</source>
        <translation>Tamaño en unidades del mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="415"/>
        <source>Size is in points</source>
        <translation>El tamaño está en puntos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1005"/>
        <source>Size:</source>
        <translation>Tamaño:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="451"/>
        <source>Buffer Labels?</source>
        <translation>¿Poner margen a las etiquetas?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="542"/>
        <source>Position</source>
        <translation>Posición</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="567"/>
        <source>Offset units</source>
        <translation>Unidades de desplazamiento</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1080"/>
        <source>X Offset (pts):</source>
        <translation>Desplazamiento en X (pts):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1116"/>
        <source>Y Offset (pts):</source>
        <translation>Desplazamiento en Y (pts):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="625"/>
        <source>Data Defined Style</source>
        <translation>Estilo de los datos definidos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="645"/>
        <source>&amp;Italic:</source>
        <translation>&amp;Cursiva:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="663"/>
        <source>&amp;Size:</source>
        <translation>&amp;Tamaño:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="681"/>
        <source>&amp;Bold:</source>
        <translation>&amp;Negrita:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="719"/>
        <source>&amp;Underline:</source>
        <translation>&amp;Subrayado:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="813"/>
        <source>&amp;Font family:</source>
        <translation>Familia de la &amp;fuente:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="847"/>
        <source>Data Defined Alignment</source>
        <translation>Alineación de los datos definidos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="914"/>
        <source>Placement:</source>
        <translation>Ubicación:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="922"/>
        <source>Data Defined Buffer</source>
        <translation>Margen de los datos definidos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1016"/>
        <source>Data Defined Position</source>
        <translation>Posición de los datos definidos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1062"/>
        <source>X Coordinate:</source>
        <translation>Coordenada X:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1101"/>
        <source>Y Coordinate:</source>
        <translation>Coordenada Y:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="41"/>
        <source>Preview:</source>
        <translation>Previsualizar:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="56"/>
        <source>QGIS Rocks!</source>
        <translation>¡QGIS Avanza!</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1136"/>
        <source>Source</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1148"/>
        <source>Field containing label:</source>
        <translation>Campo que contiene la etiqueta:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1191"/>
        <source>Default label:</source>
        <translation>Etiqueta predeterminada:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="767"/>
        <source>Size Units:</source>
        <translation>Unidades de tamaño:</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>&#xb0;</source>
        <translation type="obsolete">°</translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/ui/qgslabeldialogbase.ui" line="342"/>
        <source>°</source>
        <translation type="unfinished"></translation>
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
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="60"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Define this layer&apos;s projection:&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This layer appears to have no projection specification. By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Define la proyección de esta capa:&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Parece que no se ha especificado ninguna proyección para esta capa. Por omisión, se utilizará la misma que para el proyecto, pero puede ignorarla seleccionado una proyección diferente a continuación.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="83"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="110"/>
        <source>group</source>
        <translation>grupo</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="423"/>
        <source>&amp;Remove</source>
        <translation>E&amp;liminar</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="416"/>
        <source>&amp;Make to toplevel item</source>
        <translation>&amp;Subir el elemento al nivel superior</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="428"/>
        <source>Re&amp;name</source>
        <translation>Cambiar &amp;nombre</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="433"/>
        <source>&amp;Add group</source>
        <translation>&amp;Añadir grupo</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="434"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Expandir todo</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="435"/>
        <source>&amp;Collapse all</source>
        <translation>&amp;Comprimir todo</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="437"/>
        <source>Show file groups</source>
        <translation>Mostrar grupos de archivos</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1818"/>
        <source>No Layer Selected</source>
        <translation>Ninguna capa no seleccionada</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1819"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Para abrir una tabla de atributos, debe seleccionar una capa vectorial en la leyenda</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="474"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Zum a la extensión de la capa</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="477"/>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation>&amp;Zum a la mejor escala (100%)</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="481"/>
        <source>&amp;Show in overview</source>
        <translation>Mo&amp;strar en el localizador</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="487"/>
        <source>&amp;Remove</source>
        <translation>E&amp;liminar</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="494"/>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Abrir tabla de atributos</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="518"/>
        <source>Save as shapefile...</source>
        <translation>Guardar como archivo shape...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="525"/>
        <source>Save selection as shapefile...</source>
        <translation>Guardar selección como archivo shape...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="535"/>
        <source>&amp;Properties</source>
        <translation>&amp;Propiedades</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="584"/>
        <source>More layers</source>
        <translation>Más capas</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="585"/>
        <source>This item contains more layer files. Displaying more layers in table is not supported.</source>
        <translation>Este ítem contiene más archivos de capas. Mostrar más capas en la tabla no está soportado.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="277"/>
        <source>Attribute table - </source>
        <translation>Tabla de atributos - </translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="347"/>
        <source>Save layer as...</source>
        <translation>Guardar capa como...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="427"/>
        <source>Start editing failed</source>
        <translation>Ha fallado el comienzo de la edición</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="428"/>
        <source>Provider cannot be opened for editing</source>
        <translation>El proveedor no se puede abrir para editar</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="441"/>
        <source>Stop editing</source>
        <translation>Terminar edición</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="442"/>
        <source>Do you want to save the changes?</source>
        <translation>¿Quiere guardar los cambios?</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="460"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="449"/>
        <source>Could not commit changes</source>
        <translation>No se pudieron aplicar los cambios</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="461"/>
        <source>Problems during roll back</source>
        <translation type="unfinished">Problemas al desinstalar</translation>
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
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="394"/>
        <source>Saving done</source>
        <translation>Guardado terminado</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="394"/>
        <source>Export to Shapefile has been completed</source>
        <translation>La exportación a archivo shape se ha completado</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="398"/>
        <source>Driver not found</source>
        <translation>No se ha encontrado el controlador</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="398"/>
        <source>ESRI Shapefile driver is not available</source>
        <translation>El controlador de archivos shape de ESRI no está disponible</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="402"/>
        <source>Error creating shapefile</source>
        <translation>Error al crear el archivo shape</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="403"/>
        <source>The shapefile could not be created (</source>
        <translation>El archivo shape no se ha podido crear (</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="407"/>
        <source>Layer creation failed</source>
        <translation>Ha fallado la creación de la capa</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="494"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Zum a la extensión de la capa</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="497"/>
        <source>&amp;Show in overview</source>
        <translation>Mo&amp;strar en el localizador</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="505"/>
        <source>&amp;Remove</source>
        <translation>E&amp;liminar</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="514"/>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Abrir tabla de atributos</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="528"/>
        <source>Save as shapefile...</source>
        <translation>Guardar como archivo shape...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="530"/>
        <source>Save selection as shapefile...</source>
        <translation>Guardar selección como archivo shape...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="547"/>
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
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="411"/>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation>La tabla de atributos de la capa contiene tipos de datos no soportados</translation>
    </message>
</context>
<context>
    <name>QgsLineStyleDialogBase</name>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="13"/>
        <source>Select a line style</source>
        <translation>Seleccionar estilo de línea</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="28"/>
        <source>Styles</source>
        <translation>Estilos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="177"/>
        <source>Ok</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="184"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
</context>
<context>
    <name>QgsLineStyleWidgetBase</name>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="16"/>
        <source>Form2</source>
        <translation>Form2</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="36"/>
        <source>Outline Style</source>
        <translation>Estilo del borde</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="61"/>
        <source>Width:</source>
        <translation>Anchura:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="87"/>
        <source>Colour:</source>
        <translation>Color:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="98"/>
        <source>LineStyleWidget</source>
        <translation>Estilo de la línea (LineStyleWidget)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="120"/>
        <source>col</source>
        <translation>col</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1134"/>
        <source>Could not draw</source>
        <translation>No se pudo dibujar</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1134"/>
        <source>because</source>
        <translation>porque</translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="430"/>
        <source>No features found</source>
        <translation>No se han encontrado objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="433"/>
        <source>&lt;p&gt;No features were found within the search radius. Note that it is currently not possible to use the identify tool on unsaved features.&lt;/p&gt;</source>
        <translation>&lt;p&gt;No se han encontrado objetos espaciales dentro del radio de búsqueda. Tenga en cuenta que actualmente no es posible utilizar la herramienta de identificación en objetos espaciales sin guardar.&lt;/p&gt;</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="356"/>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation type="unfinished">
            <numerusform>- %1 objetos espaciales encontrados</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="289"/>
        <source>(clicked coordinate)</source>
        <translation>(coordenada pinchada)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="222"/>
        <source>WMS identify result for %1
%2</source>
        <translation>Resultado de identificación WMS para %1
%2</translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="248"/>
        <source>Snap tolerance</source>
        <translation>Tolerancia de autoensamblado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="249"/>
        <source>Don&apos;t show this message again</source>
        <translation>No volver a mostrar este mensaje</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="254"/>
        <source>Could not snap segment.</source>
        <translation>No se pudo autoensamblar el segmento.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="257"/>
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
    <name>QgsMarkerDialogBase</name>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="16"/>
        <source>Choose a marker symbol</source>
        <translation>Seleccionar un símbolo de marcador</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="28"/>
        <source>Directory</source>
        <translation>Directorio</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="38"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="71"/>
        <source>Ok</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="81"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="108"/>
        <source>New Item</source>
        <translation>Nuevo ítem</translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="22"/>
        <source>Measure</source>
        <translation>Medida</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="69"/>
        <source>Total:</source>
        <translation>Total:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="122"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="145"/>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="152"/>
        <source>Cl&amp;ose</source>
        <translation>&amp;Cerrar</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="206"/>
        <source>Segments (in meters)</source>
        <translation>Segmentos (en metros)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="209"/>
        <source>Segments (in feet)</source>
        <translation>Segmentos (en pies)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="212"/>
        <source>Segments (in degrees)</source>
        <translation>Segmentos (en grados)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="215"/>
        <source>Segments</source>
        <translation>Segmentos</translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="73"/>
        <source>Incorrect measure results</source>
        <translation>Resultados de medida incorrectos</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="81"/>
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
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="166"/>
        <source>Unable to access relation</source>
        <translation>No se puede acceder a la relación</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="167"/>
        <source>Unable to access the </source>
        <translation>No se puede acceder a la relación </translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="169"/>
        <source> relation.
The error message from the database was:
</source>
        <translation>.
El mensaje de error de la base de datos fue:
</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="186"/>
        <source>No GEOS Support!</source>
        <translation>No hay soporte para GEOS</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="189"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>Su instalación de PostGIS no tiene soporte para GEOS.
La selección e identificación de objetos espaciales no funcionará correctamente.
Por favor, instale PostGIS con soporte para GEOS (http://geos.refractions.net)</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="813"/>
        <source>Save layer as...</source>
        <translation>Guardar capa como...</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="965"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="886"/>
        <source>Error creating field </source>
        <translation>Error al crear el campo </translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="965"/>
        <source>Layer creation failed</source>
        <translation>Ha fallado la creación de la capa</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="971"/>
        <source>Error creating shapefile</source>
        <translation>Error al crear el archivo shape</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="973"/>
        <source>The shapefile could not be created (</source>
        <translation>El archivo shape no se ha podido crear (</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="981"/>
        <source>Driver not found</source>
        <translation>No se ha encontrado el controlador</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="982"/>
        <source> driver is not available</source>
        <translation> el controlador no está disponible</translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="115"/>
        <source>Test connection</source>
        <translation>Probar conexión</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="115"/>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>La conexión ha fallado - Compruebe la configuración y vuelva a intentarlo.

Información extensa sobre el error:
</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="112"/>
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
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="64"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Restrict the search to the public schema for spatial tables not in the geometry_columns table&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Restringir la búsqueda al esquema público de las tablas espaciales que no están en la tabla de columnas de la geometría&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="67"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Cuando se busca en tablas espaciales que no están en las tablas de columnas de la geometría, restringir la búsqueda a tablas que están en el esquema público (en algunas bases de datos esto puede ahorrar mucho tiempo)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="70"/>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation>Buscar sólo en el esquema &quot;público&quot;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="87"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;&quot;&gt;Restrict the displayed tables to those that are in the geometry_columns table&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;&quot;&gt;Restringir las tablas mostradas a aquellas que están en la tabla de columnas de la geometría&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="90"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;&quot;&gt;Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;&quot;&gt;Restringe las tablas mostradas a aquellas que están en la tabla de columnas de la geometría. Esto puede acelerar la visualización inicial de las tablas espaciales.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="93"/>
        <source>Only look in the geometry_columns table</source>
        <translation>Buscar sólo en la tabla de columnas de la geometría</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="108"/>
        <source>Save Password</source>
        <translation>Guardar contraseña</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="115"/>
        <source>Test Connect</source>
        <translation>Probar conexión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="140"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="150"/>
        <source>Host</source>
        <translation>Servidor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="160"/>
        <source>Database</source>
        <translation>Base de datos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="170"/>
        <source>Port</source>
        <translation>Puerto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="180"/>
        <source>Username</source>
        <translation>Nombre de usuario</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="190"/>
        <source>Password</source>
        <translation>Contraseña</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="210"/>
        <source>Name of the new connection</source>
        <translation>Nombre de la nueva conexión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="223"/>
        <source>5432</source>
        <translation>5432</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="255"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="271"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="287"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="290"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="13"/>
        <source>Create a New WMS connection</source>
        <translation>Crear una nueva conexión WMS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="31"/>
        <source>Connection Information</source>
        <translation>Información sobre la conexión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="62"/>
        <source>Name of the new connection</source>
        <translation>Nombre de la nueva conexión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="72"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="85"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="98"/>
        <source>Proxy Host</source>
        <translation>Servidor proxy</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="111"/>
        <source>Proxy Port</source>
        <translation>Puerto proxy</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="124"/>
        <source>Proxy User</source>
        <translation>Usuario proxy</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="137"/>
        <source>Proxy Password</source>
        <translation>Contraseña proxy</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="158"/>
        <source>Your user name for the HTTP proxy (optional)</source>
        <translation>Nombre de usuario para el proxy HTTP (opcional)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="173"/>
        <source>Password for your HTTP proxy (optional)</source>
        <translation>Contraseña para su proxy HTTP (opcional)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="183"/>
        <source>HTTP address of the Web Map Server</source>
        <translation>Dirección HTTP del servidor de mapas web</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="190"/>
        <source>Name of your HTTP proxy (optional)</source>
        <translation>Nombre de su proxy HTTP (opcional)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="205"/>
        <source>Port number of your HTTP proxy (optional)</source>
        <translation>Puerto de su proxy HTTP (opcional)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="226"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="242"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="258"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="261"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="80"/>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="81"/>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="81"/>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="81"/>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="94"/>
        <source>&amp;North Arrow</source>
        <translation>Flecha de &amp;Norte</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="95"/>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>Crea una flecha de norte que se muestra en la vista del mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="250"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Ilustraciones</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="241"/>
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
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="237"/>
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
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="206"/>
        <source>New Item</source>
        <translation>Nuevo Ítem</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="198"/>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
</context>
<context>
    <name>QgsOGRFactory</name>
    <message>
        <location filename="../src/providers/ogr/qgsogrfactory.cpp" line="63"/>
        <source>Wrong Path/URI</source>
        <translation>Ruta/URI errónea</translation>
    </message>
    <message>
        <location filename="../src/providers/ogr/qgsogrfactory.cpp" line="63"/>
        <source>The provided path for the dataset is not valid.</source>
        <translation>La ruta especificada para el conjunto de datos no es válida.</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="149"/>
        <source>Detected active locale on your system: </source>
        <translation>Idioma activo detectado en su sistema: </translation>
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
        <location filename="../src/ui/qgsoptionsbase.ui" line="86"/>
        <source>&amp;Appearance</source>
        <translation>&amp;Apariencia</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="98"/>
        <source>&amp;Splash screen</source>
        <translation>Pantalla de &amp;bienvenida</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="110"/>
        <source>Hide splash screen at startup</source>
        <translation>Ocultar la pantalla de bienvenida al iniciar la aplicación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="120"/>
        <source>&amp;Icon Theme</source>
        <translation>Tema de &amp;iconos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="132"/>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Nota: &lt;/b&gt;Los cambios de tema tendrán efecto la próxima vez que inicie QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="150"/>
        <source>Theme</source>
        <translation>Tema</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="188"/>
        <source>Default Map Appearance (Overridden by project properties)</source>
        <translation>Apariencia predeterminada del mapa (anulada por las propiedades del proyecto)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="200"/>
        <source>Selection Color:</source>
        <translation>Color de selección:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="236"/>
        <source>Background Color:</source>
        <translation>Color de fondo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="275"/>
        <source>Appearance</source>
        <translation>Apariencia</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="287"/>
        <source>Capitalise layer name</source>
        <translation>Comenzar el nombre de la capa por mayúscula</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="311"/>
        <source>&amp;Rendering</source>
        <translation>&amp;Representación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="323"/>
        <source>&amp;Update during drawing</source>
        <translation>&amp;Actualizar durante el dibujado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="335"/>
        <source>features</source>
        <translation>objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="342"/>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>La vista del mapa se actualizará después de que todos estos objetos espaciales sean leídos de la fuente de datos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="355"/>
        <source>Update display after reading</source>
        <translation>Actualizar visualización después de leer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="365"/>
        <source>(Set to 0 to not update the display until all features have been read)</source>
        <translation>(Poner a 0 para no actualizar la visualización hasta que todos los objetos espaciales se hayan leído)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="375"/>
        <source>Initial Visibility</source>
        <translation>Visibilidad inicial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="387"/>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Por omisión, las nuevas &amp;capas añadidas al mapa se deben visualizar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="397"/>
        <source>Rendering</source>
        <translation>Representación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="409"/>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>Hacer que las líneas se muestren menos quebradas a expensas de la calidad del dibujo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="416"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Selecting this will unselect the &apos;make lines less&apos; jagged toggle&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Con esta opción desactivará el &quot;hacer las líneas menos quebradas&quot;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="419"/>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Solucionar problemas con polígonos rellenados incorrectamente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="426"/>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Redibujar el mapa continuamente cuando se desplaza el separador entre mapa y leyenda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="450"/>
        <source>&amp;Map tools</source>
        <translation>Herramientas de &amp;mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="587"/>
        <source>Search radius</source>
        <translation>Radio de búsqueda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="599"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Note:&lt;/span&gt; Specify the search radius as a percentage of the map width.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Nota:&lt;/span&gt; especificar el radio de búsqueda como un porcentaje de la anchura del mapa.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="613"/>
        <source>Search Radius for Identifying Features</source>
        <translation>Radio de búsqueda para identificar objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="623"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="523"/>
        <source>Measure tool</source>
        <translation>Herramienta de medida</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="574"/>
        <source>Ellipsoid for distance calculations:</source>
        <translation>Elipsoide para el cálculo de distancias:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="462"/>
        <source>Panning and zooming</source>
        <translation>Desplazar y zum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="475"/>
        <source>Zoom</source>
        <translation>Zum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="480"/>
        <source>Zoom and recenter</source>
        <translation>Zum y centrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="485"/>
        <source>Nothing</source>
        <translation>Nada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="493"/>
        <source>Zoom factor:</source>
        <translation>Factor de zum:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="500"/>
        <source>Mouse wheel action:</source>
        <translation>Acción de la rueda del ratón:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="656"/>
        <source>Pro&amp;jection</source>
        <translation>Pro&amp;yección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="681"/>
        <source>Select Global Default ...</source>
        <translation>Seleccionar proyección global predeterminada...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="691"/>
        <source>When layer is loaded that has no projection information</source>
        <translation>Cuando se carga una capa que no tiene información sobre la proyección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="703"/>
        <source>Prompt for projection.</source>
        <translation>Preguntar sobre la proyección.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="710"/>
        <source>Project wide default projection will be used.</source>
        <translation>Se va a utilizar la proyección amplia predeterminada del proyecto.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="717"/>
        <source>Global default projection displa&amp;yed below will be used.</source>
        <translation>Se usará la proyección global predeterminada &amp;mostrada a continuación.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="817"/>
        <source>Help &amp;Browser</source>
        <translation>&amp;Navegador de la ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="842"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; The browser must be in your PATH or you can specify the full path above</source>
        <translation>&lt;b&gt;Nota:&lt;/b&gt; El navegador debe estar en la variable PATH o puede especificar la ruta completa arriba</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="858"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="869"/>
        <source>epiphany</source>
        <translation>epiphany</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="874"/>
        <source>firefox</source>
        <translation>firefox</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="879"/>
        <source>mozilla-firefox</source>
        <translation>mozilla-firefox</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="884"/>
        <source>galeon</source>
        <translation>galeon</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="889"/>
        <source>konqueror</source>
        <translation>konqueror</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="894"/>
        <source>mozilla</source>
        <translation>mozilla</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="899"/>
        <source>opera</source>
        <translation>opera</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="907"/>
        <source>Open help documents with</source>
        <translation>Abrir documentos de ayuda con</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="38"/>
        <source>&amp;General</source>
        <translation>&amp;General</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="50"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="62"/>
        <source>Ask to save project changes when required</source>
        <translation>Preguntar si guardar los cambios del proyecto cuando sea necesario</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="564"/>
        <source>Rubberband color:</source>
        <translation>Color de la banda de medida:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="728"/>
        <source>Locale</source>
        <translation>Idioma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="740"/>
        <source>Force Override System Locale</source>
        <translation>Forzar a ignorar el idioma del sistema</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="755"/>
        <source>Locale to use instead</source>
        <translation>Idioma a usar en su lugar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="768"/>
        <source>Note: Enabling / changing overide on local requires an application restart.</source>
        <translation>Nota: activar / cambiar ignorar idioma requiere reiniciar la aplicación.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="794"/>
        <source>Additional Info</source>
        <translation>Información adicional</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="806"/>
        <source>Detected active locale on your system:</source>
        <translation>Idioma activo detectado en su sistema:</translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="19"/>
        <source>Paste Transformations</source>
        <translation>Pegar transformaciones</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="42"/>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation>&lt;b&gt;Nota: Esta función todavía no es útil&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="65"/>
        <source>Source</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="86"/>
        <source>Destination</source>
        <translation>Destino</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="125"/>
        <source>&amp;Help</source>
        <translation>&amp;Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="128"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="154"/>
        <source>Add New Transfer</source>
        <translation>Añadir nueva transformación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="161"/>
        <source>&amp;OK</source>
        <translation>&amp;Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="177"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
</context>
<context>
    <name>QgsPatternDialogBase</name>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="13"/>
        <source>Select a fill pattern</source>
        <translation>Seleccionar un patrón de relleno</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="189"/>
        <source>No Fill</source>
        <translation>Sin relleno</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="205"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="212"/>
        <source>Ok</source>
        <translation>Aceptar</translation>
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
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation>Crear un buffer para una capa PostgreSQL. </translation>
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
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation> no es una capa PostgreSQL/PostGIS.</translation>
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
        <location filename="../src/app/qgspgquerybuilder.cpp" line="74"/>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Tabla &lt;b&gt;%1&lt;/b&gt; en la base de datos &lt;b&gt;%2&lt;/b&gt; en el servidor &lt;b&gt;%3&lt;/b&gt;, usuario &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="59"/>
        <source>Connection Failed</source>
        <translation>Ha fallado la conexión</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="59"/>
        <source>Connection to the database failed:</source>
        <translation>Ha fallado la conexión a la base de datos:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="222"/>
        <source>Database error</source>
        <translation>Error de la base de datos</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="182"/>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;No se pudieron obtener muestras de los valores de los campos utilizando SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="222"/>
        <source>Failed to get sample of field values</source>
        <translation>No se pudieron obtener muestras de los valores de los campos</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="248"/>
        <source>Query Result</source>
        <translation>Resultados de la consulta</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="250"/>
        <source>The where clause returned </source>
        <translation>La cláusula &quot;donde&quot; (WHERE) devolvió </translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="250"/>
        <source> rows.</source>
        <translation> filas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="254"/>
        <source>Query Failed</source>
        <translation>Ha fallado la consulta</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="256"/>
        <source>An error occurred when executing the query:</source>
        <translation>Se ha producido un error mientras se ejecutaba la consulta:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="309"/>
        <source>No Records</source>
        <translation>Sin registros</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="309"/>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>La consulta que ha especificado devuelve cero registros. Las capas válidas de PostgreSQL deben tener al menos un objeto espacial.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="237"/>
        <source>No Query</source>
        <translation>Ninguna consulta</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="237"/>
        <source>You must create a query before you can test it</source>
        <translation>Debe crear una consulta antes de probarlo</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="303"/>
        <source>Error in Query</source>
        <translation>Error en la consulta</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="23"/>
        <source>PostgreSQL Query Builder</source>
        <translation>Constructor de consultas de PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="178"/>
        <source>Clear</source>
        <translation>Limpiar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="188"/>
        <source>Test</source>
        <translation>Probar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="198"/>
        <source>Ok</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="208"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="241"/>
        <source>Values</source>
        <translation>Valores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="253"/>
        <source>All</source>
        <translation>Todos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="263"/>
        <source>Sample</source>
        <translation>Muestra</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="273"/>
        <source>Fields</source>
        <translation>Campos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="291"/>
        <source>Datasource:</source>
        <translation>Fuente de datos:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="41"/>
        <source>Operators</source>
        <translation>Operadores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="53"/>
        <source>=</source>
        <translation>=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="95"/>
        <source>IN</source>
        <translation>EN</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="102"/>
        <source>NOT IN</source>
        <translation>NO EN</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="60"/>
        <source>&lt;</source>
        <translation>&lt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="116"/>
        <source>&gt;</source>
        <translation>&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="88"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="144"/>
        <source>&lt;=</source>
        <translation>&lt;=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="137"/>
        <source>&gt;=</source>
        <translation>&gt;=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="109"/>
        <source>!=</source>
        <translation>!=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="123"/>
        <source>LIKE</source>
        <translation>COMO</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="81"/>
        <source>AND</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="130"/>
        <source>ILIKE</source>
        <translation>DISTINTO DE</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="74"/>
        <source>OR</source>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="67"/>
        <source>NOT</source>
        <translation>NO</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="304"/>
        <source>SQL where clause</source>
        <translation>Cláusula &quot;donde&quot; (WHERE) de SQL</translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="180"/>
        <source>No Plugins</source>
        <translation>No hay complementos</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="180"/>
        <source>No QGIS plugins found in </source>
        <translation>No se han econtrado complementos de QGIS en </translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="76"/>
        <source>Name</source>
        <translation type="unfinished">Nombre</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="77"/>
        <source>Version</source>
        <translation type="unfinished">Versión</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="78"/>
        <source>Description</source>
        <translation type="unfinished">Descripción</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="79"/>
        <source>Library name</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="39"/>
        <source>Plugin Directory</source>
        <translation>Directorio de complementos</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Name</source>
        <translation type="obsolete">Nombre</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Version</source>
        <translation type="obsolete">Versión</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Description</source>
        <translation type="obsolete">Descripción</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Library Name</source>
        <translation type="obsolete">Nombre de la biblioteca</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="61"/>
        <source>To load a plugin, click the checkbox next to the plugin and click Ok</source>
        <translation>Para cargar un complemento, marque la casilla correspondiente y pulse Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="92"/>
        <source>&amp;Select All</source>
        <translation>&amp;Seleccionar todos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="95"/>
        <source>Alt+S</source>
        <translation>Alt+S</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="102"/>
        <source>C&amp;lear All</source>
        <translation>&amp;Deseleccionar todos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="105"/>
        <source>Alt+L</source>
        <translation>Alt+D</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="112"/>
        <source>&amp;Ok</source>
        <translation>&amp;Aceptar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="115"/>
        <source>Alt+O</source>
        <translation>Alt+A</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="122"/>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="125"/>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="16"/>
        <source>QGIS Plugin Manager</source>
        <translation>Administrador de complementos de QGIS</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="487"/>
        <source>Zoom In</source>
        <translation>Acercar zum</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="486"/>
        <source>z</source>
        <translation>z</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="492"/>
        <source>Zoom Out</source>
        <translation>Alejar zum</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="491"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="495"/>
        <source>Zoom To Layer</source>
        <translation>Zum a la capa</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="497"/>
        <source>Zoom to Layer</source>
        <translation>Hace zum a la capa</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="500"/>
        <source>Pan Map</source>
        <translation>Mover mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="501"/>
        <source>Pan the map</source>
        <translation>Mueve el mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="504"/>
        <source>Add Point</source>
        <translation>Añadir punto</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="505"/>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="506"/>
        <source>Capture Points</source>
        <translation>Capturar puntos</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="509"/>
        <source>Delete Point</source>
        <translation>Borrar punto</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="510"/>
        <source>Delete Selected</source>
        <translation>Borrar lo seleccionado</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="556"/>
        <source>Linear</source>
        <translation>Lineal</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="557"/>
        <source>Helmert</source>
        <translation>Helmert</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="197"/>
        <source>Choose a name for the world file</source>
        <translation>Seleccionar un nombre para el archivo de referenciación (world file)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="215"/>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given filename</comment>
        <translation>-modificado</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="264"/>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="279"/>
        <source>Affine</source>
        <translation>Afinidad</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="289"/>
        <source>Not implemented!</source>
        <translation>Sin implementar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="284"/>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Una transformación por afinidad requiere cambiar el archivo ráster original. Esto todavía no está soportado.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="291"/>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt; La transformación </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="292"/>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation> aún no está soportada.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="323"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="324"/>
        <source>Could not write to </source>
        <translation>No se pudo escribir </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="271"/>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>Actualmente todos los archivos modificados se escribirán en formato TIFF.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="270"/>
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
    <name>QgsPointStyleWidgetBase</name>
    <message>
        <location filename="../src/ui/qgspointstylewidgetbase.ui" line="16"/>
        <source>Form3</source>
        <translation>Form3</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspointstylewidgetbase.ui" line="36"/>
        <source>Symbol Style</source>
        <translation>Estilo del símbolo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspointstylewidgetbase.ui" line="51"/>
        <source>Scale</source>
        <translation>Escala</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="117"/>
        <source>Unable to access relation</source>
        <translation>No se puede acceder a la relación</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="118"/>
        <source>Unable to access the </source>
        <translation>No se puede acceder a la relación </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="120"/>
        <source> relation.
The error message from the database was:
</source>
        <translation>
El mensaje de error de la base de datos fue:
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="284"/>
        <source>No GEOS Support!</source>
        <translation>No hay soporte para GEOS</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="288"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>Su instalación de PostGIS no tiene soporte para GEOS.
La selección e identificación de objetos espaciales no funcionarán correctamente.
Instale PostGIS con soporte para GEOS (http://geos.refractions.net)
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="875"/>
        <source>No suitable key column in table</source>
        <translation>No hay una columna de clave adecuada</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="879"/>
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
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="923"/>
        <source>The unique index on column</source>
        <translation>El índice único en la columna</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="925"/>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation>no es adecuado porque actualmente Qgis no soporta columnas de tipo no-int4 como clave de la tabla.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="950"/>
        <source>and </source>
        <translation>y </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="955"/>
        <source>The unique index based on columns </source>
        <translation>El índice único basado en columnas </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="957"/>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation> no es adecuado porque actualmente Qgis no soporta columnas múltiples como clave de la tabla.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1003"/>
        <source>Unable to find a key column</source>
        <translation>No se puede encontrar una columna de clave</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1086"/>
        <source> derives from </source>
        <translation> deriva de </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1090"/>
        <source>and is suitable.</source>
        <translation>y es adecuada.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1094"/>
        <source>and is not suitable </source>
        <translation>y no es adecuada </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1095"/>
        <source>type is </source>
        <translation>es de tipo </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1097"/>
        <source> and has a suitable constraint)</source>
        <translation> y tiene la reserva adecuada)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1099"/>
        <source> and does not have a suitable constraint)</source>
        <translation> y no tiene la reserva adecuada)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1185"/>
        <source>Note: </source>
        <translation>Nota: </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1187"/>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation>inicialmente parecía adecuada pero no contiene datos únicos, por lo que no es aceptable.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1199"/>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation>La vista que ha seleccionado contiene las siguientes columnas, ninguna de las cuales satisface las condiciones anteriores:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1205"/>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation>Qgis necesita que la vista tenga una columna que pueda ser utilizada como clave única. Esta columna debería proceder de una columna de una tabla de tipo int4 y ser una clave primaria, tener una reserva única o ser una columna oid de PostgreSQL. Para un mejor funcionamiento la columna también debería estar indexada.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1206"/>
        <source>The view </source>
        <translation>La vista </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1207"/>
        <source>has no column suitable for use as a unique key.
</source>
        <translation>no tiene columnas adecuadas para usar como clave única.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1208"/>
        <source>No suitable key column in view</source>
        <translation>No hay una columna de clave adecuada en la vista</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1812"/>
        <source>INSERT error</source>
        <translation>Error al INSERTAR</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1806"/>
        <source>An error occured during feature insertion</source>
        <translation>Ha ocurrido un error durante la inserción de objetos espaciales</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1865"/>
        <source>DELETE error</source>
        <translation>Error al BORRAR</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1859"/>
        <source>An error occured during deletion from disk</source>
        <translation>Ha ocurrido un error durante el borrado del disco</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2179"/>
        <source>PostGIS error</source>
        <translation>Error de PostGIS</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2181"/>
        <source>When trying: </source>
        <translation>Cuando se intentaba: </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2601"/>
        <source>Unknown geometry type</source>
        <translation>Tipo de geometría desconocido</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2602"/>
        <source>Column </source>
        <translation>Columna </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2612"/>
        <source> in </source>
        <translation> en </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2604"/>
        <source> has a geometry type of </source>
        <translation> tiene una geometría de tipo </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2604"/>
        <source>, which Qgis does not currently support.</source>
        <translation>, que Qgis no soporta actualmente.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2611"/>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation>Qgis no ha podido determinar el tipo y el srid de la columna </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2613"/>
        <source>. The database communication log was:
</source>
        <translation>. La registro de comunicación de la base de datos fue:
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2614"/>
        <source>Unable to get feature type and srid</source>
        <translation>No se ha podido obtener el tipo ni el srid del objeto espacial</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2173"/>
        <source>An error occured contacting the PostgreSQL database</source>
        <translation>Ocurrió un error al contactar con la base de datos PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2180"/>
        <source>The PostgreSQL database returned: </source>
        <translation>La base de datos PostgrSQL devolvió: </translation>
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
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="48"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="60"/>
        <source>Project Title</source>
        <translation>Título del proyecto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="75"/>
        <source>Default project title</source>
        <translation>Título del proyecto por omisión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="72"/>
        <source>Descriptive project name</source>
        <translation>Nombre descriptivo del proyecto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="85"/>
        <source>Map Units</source>
        <translation>Unidades del mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="97"/>
        <source>Meters</source>
        <translation>Metros</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="107"/>
        <source>Feet</source>
        <translation>Pies</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="114"/>
        <source>Decimal degrees</source>
        <translation>Grados decimales</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="137"/>
        <source>Precision</source>
        <translation>Precisión</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="149"/>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>Establece automáticamente el número de decimales en la visualización en la posición del ratón</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="152"/>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>El número de decimales usado cuando se visualiza en la posición del ratón se establece automaticamente de manera que un movimiento del ratón de un solo píxel cambia la posición de la visualización</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="155"/>
        <source>Automatic</source>
        <translation>Automática</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="168"/>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Estable el número de decimales a usar para la visualización en la posición del ratón</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="171"/>
        <source>Manual</source>
        <translation>Manual</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="181"/>
        <source>The number of decimal places for the manual option</source>
        <translation>Número de decimales para la opción manual</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="196"/>
        <source>decimal places</source>
        <translation>lugares decimales</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="219"/>
        <source>Digitizing</source>
        <translation>Digitalización</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="231"/>
        <source>Line Width:</source>
        <translation>Anchura de línea:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="241"/>
        <source>Line width in pixels</source>
        <translation>Anchura de la línea en píxeles</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="280"/>
        <source>Snapping tolerance in map units</source>
        <translation>Tolerancia del autoensamblaje en unidades del mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="287"/>
        <source>Line Colour:</source>
        <translation>Color de línea:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="300"/>
        <source>Map Appearance</source>
        <translation>Aspecto del mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="312"/>
        <source>Selection Color:</source>
        <translation>Color de selección:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="351"/>
        <source>Background Color:</source>
        <translation>Color de fondo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="404"/>
        <source>Projection</source>
        <translation>Proyección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="416"/>
        <source>Enable on the fly projection</source>
        <translation>Activar proyección al vuelo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="264"/>
        <source>Snapping Tolerance (in map units):</source>
        <translation>Tolerancia de autoensamblado (en unidades del mapa):</translation>
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
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="24"/>
        <source>Projection Selector</source>
        <translation>Selector de proyección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="47"/>
        <source>Projection</source>
        <translation>Proyección</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="67"/>
        <source>Search</source>
        <translation>Buscar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="85"/>
        <source>Find</source>
        <translation>Encontrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="98"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="111"/>
        <source>QGIS SRSID</source>
        <translation>QGIS SRSID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="124"/>
        <source>EPSG ID</source>
        <translation>EPSG ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="137"/>
        <source>Postgis SRID</source>
        <translation>Postgis SRID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="196"/>
        <source>Spatial Reference System</source>
        <translation>Sistema de referencia espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="201"/>
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
        <location filename="../src/ui/qgspythondialog.ui" line="33"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans Condensed&apos;; font-size:10pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;To access Quantum GIS environment from this python console use object &lt;span style=&quot; font-weight:600;&quot;&gt;iface&lt;/span&gt; from global scope which is an instance of QgisInterface class.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Usage e.g.: iface.zoomFull()&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans Condensed&apos;; font-size:10pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Para acceder al entorno de Quantum GIS desde esta consola de python use objetos &lt;span style=&quot; font-weight:600;&quot;&gt;iface&lt;/span&gt; de ámbito global, lo que es una instancia de la calse QgisInterface.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Ejemplo de uso: iface.zoomFull()&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="65"/>
        <source>&gt;&gt;&gt;</source>
        <translation>&gt;&gt;&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="47"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans Condensed&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="281"/>
        <source>and all other files</source>
        <translation>y todos los demás archivos</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3245"/>
        <source>Not Set</source>
        <translation>No establecido</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2633"/>
        <source>Raster Extent: </source>
        <translation>Extensión del ráster: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2636"/>
        <source>Clipped area: </source>
        <translation>Área rectortada: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3862"/>
        <source>Driver:</source>
        <translation>Controlador:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3888"/>
        <source>Dataset Description</source>
        <translation>Descripción del conjunto de datos</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3938"/>
        <source>Dimensions:</source>
        <translation>Dimensiones:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3941"/>
        <source>X: </source>
        <translation>X: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3942"/>
        <source> Y: </source>
        <translation> Y: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3942"/>
        <source> Bands: </source>
        <translation> Bandas: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3949"/>
        <source>No Data Value</source>
        <translation>Valor sin datos</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3957"/>
        <source>Data Type:</source>
        <translation>Tipo de datos:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3963"/>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte - Entero natural de 8 bits</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3966"/>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 - Entero natural de 16 bits </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3969"/>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 - Número entero de 16 bits </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3972"/>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 - Entero natural de 32 bits </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3975"/>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 - Número entero de 32 bits </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3978"/>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 - Número de coma flotante de 32 bits </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3981"/>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 - Número de coma flotante de 64 bits </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3984"/>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 - Número complejo Int16 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3987"/>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 - Número complejo Int32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3990"/>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 - Número conplejo Float32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3993"/>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 - Número complejo Float64 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3996"/>
        <source>Could not determine raster data type.</source>
        <translation>No se pudo determinar el tipo de datos ráster.</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4001"/>
        <source>Pyramid overviews:</source>
        <translation>Pirámides:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4022"/>
        <source>Layer Spatial Reference System: </source>
        <translation>Sistema de referencia espacial de la capa: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4050"/>
        <source>Origin:</source>
        <translation>Origen:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4059"/>
        <source>Pixel Size:</source>
        <translation>Tamaño de píxel:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4076"/>
        <source>Property</source>
        <translation>Propiedad</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4079"/>
        <source>Value</source>
        <translation>Valor</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4923"/>
        <source>Band</source>
        <translation>Banda</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4095"/>
        <source>Band No</source>
        <translation>Número de banda</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4107"/>
        <source>No Stats</source>
        <translation>No hay estadísticas</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4110"/>
        <source>No stats collected yet</source>
        <translation>Todavía no se han recogido estadísticas</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4120"/>
        <source>Min Val</source>
        <translation>Valor mínimo</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4128"/>
        <source>Max Val</source>
        <translation>Valor máximo</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4136"/>
        <source>Range</source>
        <translation>Intervalo</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4144"/>
        <source>Mean</source>
        <translation>Media</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4152"/>
        <source>Sum of squares</source>
        <translation>Suma de cuadrados</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4160"/>
        <source>Standard Deviation</source>
        <translation>Desviación estándar</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4168"/>
        <source>Sum of all cells</source>
        <translation>Suma de todas las celdas</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4176"/>
        <source>Cell Count</source>
        <translation>Número de celdas</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4261"/>
        <source>Average Magphase</source>
        <translation>Fase magnética (Magphase) media</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4266"/>
        <source>Average</source>
        <translation>Media</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4880"/>
        <source>out of extent</source>
        <translation>fuera de la extensión</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4917"/>
        <source>null (no data)</source>
        <translation>nulo (sin datos)</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="688"/>
        <source>Grayscale</source>
        <translation>Escala de grises</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="678"/>
        <source>Pseudocolor</source>
        <translation>Pseudocolor</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="682"/>
        <source>Freak Out</source>
        <translation>Alucinante</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="217"/>
        <source>Not Set</source>
        <translation>No establecido</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="116"/>
        <source>Palette</source>
        <translation>Paleta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="571"/>
        <source>Columns: </source>
        <translation>Columnas: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="572"/>
        <source>Rows: </source>
        <translation>Filas: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="573"/>
        <source>No-Data Value: </source>
        <translation>Valor sin datos: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="573"/>
        <source>n/a</source>
        <translation>n/d</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="651"/>
        <source>&lt;h3&gt;Multiband Image Notes&lt;/h3&gt;&lt;p&gt;This is a multiband image. You can choose to render it as grayscale or color (RGB). For color images, you can associate bands to colors arbitarily. For example, if you have a seven band landsat image, you may choose to render it as:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Visible Blue (0.45 to 0.52 microns) - not mapped&lt;/li&gt;&lt;li&gt;Visible Green (0.52 to 0.60 microns) - not mapped&lt;/li&gt;&lt;/li&gt;Visible Red (0.63 to 0.69 microns) - mapped to red in image&lt;/li&gt;&lt;li&gt;Near Infrared (0.76 to 0.90 microns) - mapped to green in image&lt;/li&gt;&lt;li&gt;Mid Infrared (1.55 to 1.75 microns) - not mapped&lt;/li&gt;&lt;li&gt;Thermal Infrared (10.4 to 12.5 microns) - not mapped&lt;/li&gt;&lt;li&gt;Mid Infrared (2.08 to 2.35 microns) - mapped to blue in image&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>&lt;h3&gt;Notas para las Imágenes Multibanda&lt;/h3&gt;&lt;p&gt;Esta es una imagen multibanda. Puede elegir visualizarla como una imagen de escala de grises o color (RGB). Para imágenes en color, puede asociar bandas a los colores de forma arbitraria. Por ejemplo, si tiene siete bandas en una imagen Landsat, puede asignarlas de la manera siguiente:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Azul Visible (0.45 a 0.52 micras) - no visualizada&lt;/li&gt;&lt;li&gt;Verde Visible (0.52 a 0.60 micras) - no visualizada&lt;/li&gt;&lt;/li&gt;Rojo Visible (0.63 a 0.69 micras) - asignada al rojo en la imagen&lt;/li&gt;&lt;li&gt;Infrarrojo cercano (0.76 a 0.90 micras) - asignada al verde en la imagen&lt;/li&gt;&lt;li&gt;Infrarrojo medio (1.55 a 1.75 micras) - no visualizada&lt;/li&gt;&lt;li&gt;Infrarrojo térmico (10.4 a 12.5 micras) - no visualizada&lt;/li&gt;&lt;li&gt;Infrarrojo medio (2.08 a 2.35 micras) - asignada al azul en la imagen&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="658"/>
        <source>&lt;h3&gt;Paletted Image Notes&lt;/h3&gt; &lt;p&gt;This image uses a fixed color palette. You can remap these colors in different combinations e.g.&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Red - blue in image&lt;/li&gt;&lt;li&gt;Green - blue in image&lt;/li&gt;&lt;li&gt;Blue - green in image&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>&lt;h3&gt;Notas para las imágenes con paleta&lt;/h3&gt; &lt;p&gt;Esta imagen utiliza una paleta fija de color. Puede ajustar estos colores a diferentes combinaciones, por ej.&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Rojo - azul en la imagen&lt;/li&gt;&lt;li&gt;Verde - azul en la imagen&lt;/li&gt;&lt;li&gt;Azul - verden en la imagen&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="665"/>
        <source>&lt;h3&gt;Grayscale Image Notes&lt;/h3&gt; &lt;p&gt;You can remap these grayscale colors to a pseudocolor image using an automatically generated color ramp.&lt;/p&gt;</source>
        <translation>&lt;h3&gt;Notas para las Imágenes de escala de grises&lt;/h3&gt; &lt;p&gt;Puede cambiar el formato de esta imagen de escala de grises a pseudocolor utilizando una rampa de color graduado generada automaticamente.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="484"/>
        <source>Write access denied</source>
        <translation>Acceso de escritura denegado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="485"/>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation>Acceso de escritura denegado. Ajuste los permisos del archivo e inténtelo de nuevo.

</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="494"/>
        <source>Building pyramids failed.</source>
        <translation>Ha fallado la construcción de pirámides.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="490"/>
        <source>The file was not writeable. Some formats can not be written to, only read. You can also try to check the permissions and then try again.</source>
        <translation>El archivo no se puede escribir. Algunos formatos no se pueden escribir, sólo se pueden leer. También puede comprobar los permisos e intentarlo de nuevo.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="495"/>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation>La creación de pirámides no es soportada en este tipo de ráster.</translation>
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
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="38"/>
        <source>Symbology</source>
        <translation>Simbología</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="50"/>
        <source>Display</source>
        <translation>Mostrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="140"/>
        <source>Transparency:</source>
        <translation>Transparencia:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="110"/>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Total&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="90"/>
        <source>0%</source>
        <translation>0%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="70"/>
        <source>None</source>
        <translation>Nada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="208"/>
        <source>Grayscale Image</source>
        <translation>Imagen en escala de grises</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="288"/>
        <source>Color Image</source>
        <translation>Imagen en color</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="150"/>
        <source>Invert Color Map</source>
        <translation>Invertir colores del mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="369"/>
        <source>&lt;b&gt;&lt;font color=&quot;#0000ff&quot;&gt;Blue&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&quot;#0000ff&quot;&gt;Azul&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="336"/>
        <source>&lt;b&gt;&lt;font color=&quot;#00ff00&quot;&gt;Green&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&quot;#00ff00&quot;&gt;Verde&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="346"/>
        <source>&lt;b&gt;&lt;font color=&quot;#ff0000&quot;&gt;Red&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&quot;#ff0000&quot;&gt;Rojo&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="313"/>
        <source>Band</source>
        <translation>Banda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="306"/>
        <source>Color</source>
        <translation>Color</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="249"/>
        <source>Color Map</source>
        <translation>Mapa en color</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="259"/>
        <source>Std Deviations</source>
        <translation>Desviaciones estándar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="275"/>
        <source>Gray</source>
        <translation>Gris</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="393"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="413"/>
        <source>Thumbnail</source>
        <translation>Miniatura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="463"/>
        <source>Legend:</source>
        <translation>Leyenda:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="513"/>
        <source>Palette:</source>
        <translation>Paleta:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="565"/>
        <source>Display Name:</source>
        <translation>Mostrar nombre:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="585"/>
        <source>Layer Source:</source>
        <translation>Fuente de la capa:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="603"/>
        <source>Columns:</source>
        <translation>Columnas:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="610"/>
        <source>Rows:</source>
        <translation>Filas:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="617"/>
        <source>No Data:</source>
        <translation>Sin datos:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="626"/>
        <source>DebugInfo</source>
        <translation>Información de depurado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="633"/>
        <source>Scale Dependent Visibility</source>
        <translation>Visibilidad según la escala</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="648"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Escala máxima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="661"/>
        <source>Maximum 1:</source>
        <translation>Máximo 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="671"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Escala mínima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="684"/>
        <source>Minimum 1:</source>
        <translation>Mínimo 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="697"/>
        <source>Spatial Reference System</source>
        <translation>Sistema de referencia espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="709"/>
        <source>Change</source>
        <translation>Cambiar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="727"/>
        <source>Metadata</source>
        <translation>Metadatos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="747"/>
        <source>Pyramids</source>
        <translation>Pirámides</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="812"/>
        <source>Pyramid Resolutions</source>
        <translation>Resolución de las pirámides</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="843"/>
        <source>Resampling Method</source>
        <translation>Método de remuestreo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="854"/>
        <source>Average</source>
        <translation>Media</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="859"/>
        <source>Nearest Neighbour</source>
        <translation>Vecino más próximo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="867"/>
        <source>Build Pyramids</source>
        <translation>Construir pirámides</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="877"/>
        <source>Histogram</source>
        <translation>Histograma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="969"/>
        <source>Options</source>
        <translation>Opciones</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="981"/>
        <source>Column Count:</source>
        <translation>Número de columnas:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="988"/>
        <source>Out Of Range OK?</source>
        <translation>¿Permitir valores fuera de rango?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="995"/>
        <source>Allow Approximation</source>
        <translation>Permitir aproximación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="916"/>
        <source>Chart Type</source>
        <translation>Tipo de gráfico</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="928"/>
        <source>Line Graph</source>
        <translation>Gráfico de líneas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="938"/>
        <source>Bar Chart</source>
        <translation>Gráfico de barras</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1024"/>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="379"/>
        <source>Transparent</source>
        <translation>Transparente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="168"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot;font-size:10pt;font-family:Sans Serif&quot;&gt;
&lt;p style=&quot;margin-top:14px&quot; dir=&quot;ltr&quot;&gt;&lt;span style=&quot;font-weight:600&quot;&gt;Notes&lt;/span&gt;&lt;/p&gt;
&lt;/body&gt;&lt;/html&gt;
</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot;font-size:9pt;font-family:Sans Serif&quot;&gt;
&lt;p style=&quot;margin-top:14px&quot; dir=&quot;ltr&quot;&gt;&lt;span style=&quot;font-weight:600&quot;&gt;Notas&lt;/span&gt;&lt;/p&gt;
&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="788"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot;font-size:10pt;font-family:Sans Serif&quot;&gt;
&lt;p style=&quot;margin-top:18px&quot; dir=&quot;ltr&quot;&gt;&lt;span style=&quot;font-size:15pt;font-weight:600&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;Large resolution raster layers can slow navigation in QGIS. By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom. You must have write access in the directory where the original data is stored to build pyramids. &lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;&lt;span style=&quot;color:#ff0000&quot;&gt;Please note that building pyramids may alter the original data file and once created they cannot be removed.&lt;/span&gt;&lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;&lt;span style=&quot;color:#ff0000&quot;&gt;Please note that building pyramids could corrupt your image - always make a backup of your data first!&lt;/span&gt;&lt;/p&gt;
&lt;/body&gt;&lt;/html&gt;
</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot;font-size:9pt;font-family:Sans Serif&quot;&gt;
&lt;p style=&quot;margin-top:18px&quot; dir=&quot;ltr&quot;&gt;&lt;span style=&quot;font-size:15pt;font-weight:600&quot;&gt;Descripción&lt;/span&gt;&lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;Las capas ráster de elevada resolución pueden ralentizar la navegación en QGIS. Creando copias de menor resolución de los datos (pirámides) se puede mejorar el rendimiento de forma considerable, ya que QGIS selecciona la resolución más adecuada dependiendo del nivel de zum. Debe tener permiso de escritura en el directorio donde están almacenados los datos originales para construir las pirámides. &lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;&lt;span style=&quot;color:#ff0000&quot;&gt;Nota: la construcción de pirámides puede alterar el archivo original de los datos y una vez creadas no podrán ser eliminadas.&lt;/span&gt;&lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;&lt;span style=&quot;color:#ff0000&quot;&gt;Nota: la construcción de pirámides puede desvirtuar la imagen - realice siempre una copia de seguridad&lt;/span&gt;&lt;/p&gt;
&lt;/body&gt;&lt;/html&gt;</translation>
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
    <message numerus="yes">
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="126"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform>Se han encontrado %d objetos espaciales coincidentes.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="128"/>
        <source>No matching features found.</source>
        <translation>No se han encontrado objetos espaciales coincidentes.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="129"/>
        <source>Search results</source>
        <translation>Resultados de la búsqueda</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="138"/>
        <source>Search string parsing error</source>
        <translation>Error al analizar la cadena de búsqueda</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="194"/>
        <source>No Records</source>
        <translation>Ningún registro</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="194"/>
        <source>The query you specified results in zero records being returned.</source>
        <translation>La consulta que especificó no ha devuelto ningún registro.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="35"/>
        <source>Search query builder</source>
        <translation>Buscar constructor de consultas</translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="171"/>
        <source>Are you sure you want to remove the </source>
        <translation>¿Está seguro de que quiere eliminar la conexión </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="171"/>
        <source> connection and all associated settings?</source>
        <translation> y toda su configuración?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="172"/>
        <source>Confirm Delete</source>
        <translation>Confirmar borrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="454"/>
        <source>WMS Provider</source>
        <translation>Proveedor WMS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="456"/>
        <source>Could not open the WMS Provider</source>
        <translation>No se pudo conectar al proveedor WMS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="465"/>
        <source>Select Layer</source>
        <translation>Seleccionar capa</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="465"/>
        <source>You must select at least one layer first.</source>
        <translation>Primero debe seleccionar al menos una capa.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="469"/>
        <source>Coordinate Reference System</source>
        <translation>Sistema de coordenadas de referencia</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="469"/>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation>No hay un sistema de coordenadas de referencia para las capas seleccionadas.</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsserversourceselect.cpp" line="588"/>
        <source>Coordinate Reference System (%1 available)</source>
        <translation type="unfinished">
            <numerusform>Sistema de referencia de coordenadas (%1 disponible)</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="767"/>
        <source>Could not understand the response.  The</source>
        <translation>La respuesta es ininteligible. El proveedor </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="768"/>
        <source>provider said</source>
        <translation> ha dicho</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="822"/>
        <source>WMS proxies</source>
        <translation>Proxy del servidor WMS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="822"/>
        <source>&lt;p&gt;Several WMS servers have been added to the server list. Note that the proxy fields have been left blank and if you access the internet via a web proxy, you will need to individually set the proxy fields with appropriate values.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Se han añadido varios servidores WMS la lista de servidores. Los campos proxy se han dejado en blanco y si accede a internet a través de un servidor proxy necesitará rellenar los campos del proxy de manera individual con valores apropiados.&lt;/p&gt;</translation>
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
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="236"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="241"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="246"/>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="251"/>
        <source>Abstract</source>
        <translation>Resumen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="276"/>
        <source>Image encoding</source>
        <translation>Codificación de la imagen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="302"/>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="305"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="315"/>
        <source>C&amp;lose</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="318"/>
        <source>Alt+L</source>
        <translation>Alt+C</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="435"/>
        <source>The database gave an error while executing this SQL:</source>
        <translation>La base de datos dio un error mientras ejecutaba esta SQL:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="443"/>
        <source>The error was:</source>
        <translation>El error fue:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="440"/>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation>... (cortado el resto de la SQL)</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="21"/>
        <source>Single Symbol</source>
        <translation>Símbolo único</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="168"/>
        <source>Fill Patterns:</source>
        <translation>Patrones de relleno:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="544"/>
        <source>No Fill</source>
        <translation>Sin relleno</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="898"/>
        <source>Outline Width:</source>
        <translation>Anchura del borde:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="950"/>
        <source>Fill Color:</source>
        <translation>Color de relleno:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="960"/>
        <source>Outline color:</source>
        <translation>Color del borde:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="684"/>
        <source>Outline Style:</source>
        <translation>Estilo del borde:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="69"/>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="121"/>
        <source>Size</source>
        <translation>Tamaño</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="155"/>
        <source>Symbol</source>
        <translation>Símbolo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="47"/>
        <source>Label:</source>
        <translation>Etiqueta:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="641"/>
        <source>Browse:</source>
        <translation>Explorar:</translation>
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
        <location filename="../src/plugins/spit/qgsspit.cpp" line="120"/>
        <source>New Connection</source>
        <translation>Nueva conexión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="145"/>
        <source>Are you sure you want to remove the [</source>
        <translation>¿Está seguro de que quiere eliminar la conexión [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="145"/>
        <source>] connection and all associated settings?</source>
        <translation>] y toda su configuración?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="146"/>
        <source>Confirm Delete</source>
        <translation>Confirmar borrado</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="170"/>
        <source>Add Shapefiles</source>
        <translation>Añadir archivos shape</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="172"/>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation>Archivos shape (*.shp);;Todos los archivos (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="866"/>
        <source> - Edit Column Names</source>
        <translation> - Editar nombres de columna</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="308"/>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>Los siguientes archivos shape no se han podido cargar:

</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="312"/>
        <source>REASON: File cannot be opened</source>
        <translation>MOTIVO: el archivo no se puede abrir</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="317"/>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>MOTIVO: faltan uno o varios de los archivos del shape (*.dbf, *shx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="394"/>
        <source>General Interface Help:</source>
        <translation>Ayuda de la Interfaz general:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="396"/>
        <source>PostgreSQL Connections:</source>
        <translation>Conexiones PostgreSQL:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="398"/>
        <source>[New ...] - create a new connection</source>
        <translation>[Nueva...] - crear una conexión nueva</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="399"/>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation>[Editar...] - editar la conexión seleccionada actualmente</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="400"/>
        <source>[Remove] - remove the currently selected connection</source>
        <translation>[Eliminar] - eliminar la conexión seleccionada actualmente</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="401"/>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation>-debe seleccionar una conexión que funcione para poder importar archivos</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="402"/>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation>-cuando se cambian las conexiones el esquema global cambia en concordancia</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="403"/>
        <source>Shapefile List:</source>
        <translation>Lista de archivos shape:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="405"/>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation>[Añadir...] - abrir un cuadro de diálogo y buscar los archivos a importar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="406"/>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation>[Eliminar] - eliminar los archivos seleccionados de la lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="407"/>
        <source>[Remove All] - remove all the files in the list</source>
        <translation>[Eliminar todos] - eliminar todos los archivos de la lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="408"/>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] - ID de referencia para los archivos shape a importar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="409"/>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation>[Utilizar (SRID) por omisión] - establecer SRID a -1</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="410"/>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation>[Nombre de columna de la geometría] - nombre de la columna de la geometría en la base de datos</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="411"/>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation>[Utilizar [Nombre de columna de la geometría] por omisión] - establecer el nombre de la columna a &quot;the_geom&quot;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="412"/>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation>[Esquema global] - establecer el esquema para todos los archivos a importar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="414"/>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[Importar] - importar los archivos shape actuales de la lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="415"/>
        <source>[Quit] - quit the program
</source>
        <translation>[Cerrar] - salir del programa
</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="416"/>
        <source>[Help] - display this help dialog</source>
        <translation>[Ayuda] - mostrar esta ayuda</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="842"/>
        <source>Import Shapefiles</source>
        <translation>Importar archivos shape</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="430"/>
        <source>You need to specify a Connection first</source>
        <translation>Primero debe especificar una conexión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="446"/>
        <source>Connection failed - Check settings and try again</source>
        <translation>La conexión ha fallado - Comprobar la configuración y probar de nuevo</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="477"/>
        <source>PostGIS not available</source>
        <translation>PostGIS no está disponible</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="479"/>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;La base de datos seleccionada no tiene instalado PostGIS, lo cual es necesario para almacenar datos espaciales.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="550"/>
        <source>You need to add shapefiles to the list first</source>
        <translation>Primero debe añadir archivos shape a la lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="611"/>
        <source>Importing files</source>
        <translation>Importando archivos</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="555"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="559"/>
        <source>Progress</source>
        <translation>Progreso</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="566"/>
        <source>Problem inserting features from file:</source>
        <translation>Problemas al insertar objetos espaciales del archivo:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="573"/>
        <source>Invalid table name.</source>
        <translation>Nombre de la tabla no válido.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="583"/>
        <source>No fields detected.</source>
        <translation>No se han detectado campos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="596"/>
        <source>Checking to see if </source>
        <translation>Comprobando si </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="604"/>
        <source>The following fields are duplicates:</source>
        <translation>Los siguientes campos están duplicados:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="844"/>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Error al ejecutar la SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="845"/>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation>&lt;/p&gt;&lt;p&gt;La base de datos ha dicho:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="704"/>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>Importar archivos shape - La relación existe</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="705"/>
        <source>The Shapefile:</source>
        <translation>El archivo shape:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="706"/>
        <source>will use [</source>
        <translation>utilizará la relación [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="706"/>
        <source>] relation for its data,</source>
        <translation>] para sus datos,</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="706"/>
        <source>which already exists and possibly contains data.</source>
        <translation>que ya existe y posiblemente contenga datos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="707"/>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>Para evitar la pérdida de datos cambie el &quot;Nombre de la relación de la BD&quot;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="707"/>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation>para este archivo shape en la lista de archivos del diálogo principal.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="708"/>
        <source>Do you want to overwrite the [</source>
        <translation>¿Quiere sobrescribir la relación [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="708"/>
        <source>] relation?</source>
        <translation>]?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="869"/>
        <source>Use the table below to edit column names. Make sure that none of the columns are named using a PostgreSQL reserved word</source>
        <translation>Usar la tabla de abajo para editar los nombres de columna. Asegúrese de que ninguno de los nombres utilizados sea una palabra reservada de PostgreSQL</translation>
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
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="53"/>
        <source>Shapefile to PostGIS Import Tool</source>
        <translation>Herramienta para importar archivos shape a PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="63"/>
        <source>Shapefile List</source>
        <translation>Lista de archivos shape</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="96"/>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Añadir un archivo shape a la lista de archivos a importar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="99"/>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="109"/>
        <source>Remove the selected shapefile from the import list</source>
        <translation>Eliminar el archivo shape seleccionado de la lista de importación</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="399"/>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="122"/>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Eliminar todos los archivos shape de la lista de importación</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="125"/>
        <source>Remove All</source>
        <translation>Eliminar todos</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="187"/>
        <source>SRID</source>
        <translation>SRID</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="212"/>
        <source>Set the SRID to the default value</source>
        <translation>Establecer el SRID al valor por omisión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="215"/>
        <source>Use Default SRID</source>
        <translation>Utilizar el SRID por omisión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="251"/>
        <source>Set the geometry column name to the default value</source>
        <translation>Establecer el nombre de la columna de la geometría al valor por omisión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="254"/>
        <source>Use Default Geometry Column Name</source>
        <translation>Usar el nombre de la columna de la geometría por omisión</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="269"/>
        <source>Geometry Column Name</source>
        <translation>Nombre de la columna de la geometría</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="330"/>
        <source>Global Schema</source>
        <translation>Esquema global</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="360"/>
        <source>PostgreSQL Connections</source>
        <translation>Conexiones PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="383"/>
        <source>Create a new PostGIS connection</source>
        <translation>Crear una nueva conexión a PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="386"/>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="396"/>
        <source>Remove the current PostGIS connection</source>
        <translation>Eliminar la conexión actual a PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="409"/>
        <source>Connect</source>
        <translation>Conectar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="419"/>
        <source>Edit the current PostGIS connection</source>
        <translation>Editar la conexión actual a PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="422"/>
        <source>Edit</source>
        <translation>Editar</translation>
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
    <name>QgsUniqueValueDialogBase</name>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="13"/>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="37"/>
        <source>Classification Field:</source>
        <translation>Campo de clasificación:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="47"/>
        <source>Delete class</source>
        <translation>Borrar clase</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="80"/>
        <source>Classify</source>
        <translation>Clasificar</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2032"/>
        <source>Could not commit the added features.</source>
        <translation>No se pudieron añadir los objetos espaciales.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2123"/>
        <source>No other types of changes will be committed at this time.</source>
        <translation>No se realizará ningún otro tipo de cambios en este momento.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2054"/>
        <source>Could not commit the changed attributes.</source>
        <translation>No se pudieron realizar los cambios en los atributos.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2113"/>
        <source>However, the added features were committed OK.</source>
        <translation>Sin embargo, se han añadido correctamente los objetos espaciales.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2080"/>
        <source>Could not commit the changed geometries.</source>
        <translation>No se pudieron realizar los cambios en la geometría.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2117"/>
        <source>However, the changed attributes were committed OK.</source>
        <translation>Sin embargo, se cambiaron correctamente los atributos.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2110"/>
        <source>Could not commit the deleted features.</source>
        <translation>No se pudieron borrar los objetos espaciales.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2121"/>
        <source>However, the changed geometries were committed OK.</source>
        <translation>Sin embargo, se cambiaron correctamente las geometrías.</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="101"/>
        <source>Transparency: </source>
        <translation>Transparencia: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="188"/>
        <source>Single Symbol</source>
        <translation>Símbolo único</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="191"/>
        <source>Graduated Symbol</source>
        <translation>Símbolo graduado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="192"/>
        <source>Continuous Color</source>
        <translation>Color graduado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="193"/>
        <source>Unique Value</source>
        <translation>Valor único</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="150"/>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation>Este botón abre el constructor de consultas de PostgreSQL y permite crear un subconjunto de objetos espaciales para mostrar en la vista del mapa, en vez de mostrarlos todos</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="151"/>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>Aquí se muestra la consulta usada para limitar los objetos espaciales de las capas. Esto actualmente sólo está soportado para capas PostgreSQL. Para introducir o modificar la consulta, pulse el botón Constructor de consultas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="371"/>
        <source>Spatial Index</source>
        <translation>Índice espacial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="366"/>
        <source>Creation of spatial index successfull</source>
        <translation>La creación del índice espacial ha sido correcta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="371"/>
        <source>Creation of spatial index failed</source>
        <translation>Ha fallado la creación del índice espacial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="384"/>
        <source>General:</source>
        <translation>General:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="399"/>
        <source>Storage type of this layer : </source>
        <translation>Tipo de almacenamiento de esta capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="405"/>
        <source>Source for this layer : </source>
        <translation>Fuente de esta capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="422"/>
        <source>Geometry type of the features in this layer : </source>
        <translation>Tipo de geometría de los objetos espaciales en esta capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="430"/>
        <source>The number of features in this layer : </source>
        <translation>Número de objetos espaciales en esta capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="435"/>
        <source>Editing capabilities of this layer : </source>
        <translation>Posibilidades de edición de esta capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="442"/>
        <source>Extents:</source>
        <translation>Extensión:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="447"/>
        <source>In layer spatial reference system units : </source>
        <translation>En unidades del sistema espacial de referencia de la capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="448"/>
        <source>xMin,yMin </source>
        <translation>xMín,yMín </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="452"/>
        <source> : xMax,yMax </source>
        <translation> : xMáx,yMáx </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="507"/>
        <source>In project spatial reference system units : </source>
        <translation>En unidades del sistema espacial de referencia del proyecto: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="481"/>
        <source>Layer Spatial Reference System:</source>
        <translation>Sistema de referencia espacial de la capa:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="518"/>
        <source>Attribute field info:</source>
        <translation>Información del campo del atributo:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="525"/>
        <source>Field</source>
        <translation>Campo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="528"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="531"/>
        <source>Length</source>
        <translation>Tamaño</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="534"/>
        <source>Precision</source>
        <translation>Precisión</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="392"/>
        <source>Layer comment: </source>
        <translation>Comentario de la capa: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="537"/>
        <source>Comment</source>
        <translation>Comentario</translation>
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
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="54"/>
        <source>Symbology</source>
        <translation>Simbología</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="102"/>
        <source>Legend type:</source>
        <translation>Tipo de leyenda:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="112"/>
        <source>Transparency:</source>
        <translation>Transparencia:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="136"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="148"/>
        <source>Display name</source>
        <translation>Mostrar el nombre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="158"/>
        <source>Display field for the Identify Results dialog box</source>
        <translation>Campo para mostrar en el cuadro de diálogo de resultados de la identificación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="161"/>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation>Establece el campo que se mostrará con la herramienta de identificación</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="164"/>
        <source>Display field</source>
        <translation>Mostrar campo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="174"/>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation>Use este control para indicar el campo que se situará en el nivel superior del cuadro de diálogo de resultados de la identificación.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="184"/>
        <source>Use scale dependent rendering</source>
        <translation>Utilizar represenación dependiente de la escala</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="199"/>
        <source>Maximum 1:</source>
        <translation>Máximo 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="209"/>
        <source>Minimum 1:</source>
        <translation>Mínimo 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="219"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Escala mínima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="232"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Escala máxima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="248"/>
        <source>Spatial Index</source>
        <translation>Índice espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="260"/>
        <source>Create Spatial Index</source>
        <translation>Crear índice espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="270"/>
        <source>Create</source>
        <translation>Crear</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="296"/>
        <source>Spatial Reference System</source>
        <translation>Sistema de referencia espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="315"/>
        <source>Change</source>
        <translation>Cambiar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="325"/>
        <source>Subset</source>
        <translation>Subconjunto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="366"/>
        <source>Query Builder</source>
        <translation>Constructor de consultas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="377"/>
        <source>Metadata</source>
        <translation>Metadatos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="397"/>
        <source>Labels</source>
        <translation>Etiquetas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="435"/>
        <source>Display labels</source>
        <translation>Mostrar etiquetas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="443"/>
        <source>Actions</source>
        <translation>Acciones</translation>
    </message>
</context>
<context>
    <name>QgsVectorSymbologyWidgetBase</name>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="16"/>
        <source>Form2</source>
        <translation>Form2</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="44"/>
        <source>Label</source>
        <translation>Etiqueta</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="49"/>
        <source>Min</source>
        <translation>Mín</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="54"/>
        <source>Max</source>
        <translation>Máx</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="62"/>
        <source>Symbol Classes:</source>
        <translation>Clases de símbolos:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="77"/>
        <source>Count:</source>
        <translation>Cuenta:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="90"/>
        <source>Mode:</source>
        <translation>Modo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="100"/>
        <source>Field:</source>
        <translation>Campo:</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="58"/>
        <source>&amp;Add WFS layer</source>
        <translation>&amp;Añadir capa WFS</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1390"/>
        <source>unknown</source>
        <translation>desconocido</translation>
    </message>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1396"/>
        <source>received %1 bytes from %2</source>
        <translation>recibidos %1 bytes de %2</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="259"/>
        <source>Are you sure you want to remove the </source>
        <translation>¿Está seguro de que quiere eleminar la conexión </translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="259"/>
        <source> connection and all associated settings?</source>
        <translation> y toda su configuración?</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="260"/>
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
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="718"/>
        <source>Tried URL: </source>
        <translation>URL probada: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="698"/>
        <source>HTTP Exception</source>
        <translation>Excepción HTTP</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="661"/>
        <source>WMS Service Exception</source>
        <translation>Excepción del servicio WMS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1512"/>
        <source>DOM Exception</source>
        <translation>Excepción DOM</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="772"/>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>No se pudieron obtener las capacidades del WMS: %1 en la línea %2 columna %3</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="803"/>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>Probablemente se deba a una URL incorrecta del servidor WMS.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="799"/>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>No se pudieron obtener las capacidades del WMS en el formato esperado (DTD): no se ha encontrado %1 o %2</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1514"/>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>No se pudo obtener una excepción del servicio WMS en %1: %2 en la línea %3 columna %4</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1564"/>
        <source>Request contains a Format not offered by the server.</source>
        <translation>La solicitud contiene un formato no ofrecido por el servidor.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1568"/>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>La solicitud contiene un CRS no ofrecido por el servidor para una o más de las capas solicitadas.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1572"/>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>La solicitud contiene un SRS no ofrecido por el servidor para una o más de las capas solicitadas.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1577"/>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation>La solicitud de obtención de mapa (GetMap) es para una capa no ofrecida por el servidor o la solicitud de información del tema (GetFeatureInfo) es para una capa que no mostrada en el mapa.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1581"/>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>La solicitud es para una capa en un estilo no ofrecido por el servidor.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1585"/>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>La solicitud de información del tema (GetFeatureInfo) se aplica a una capa no declarada consultable.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1589"/>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>La solicitud de información del tema (GetFeatureInfo) contiene valores no válidos de X o Y.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1594"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation>El valor del parámetro (opcional) UpdateSequence en la consulta GetCapabilities es igual al valor actual del número de secuencia del servicio de actualización de los metadatos.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1599"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation>El valor del parámetro (opcional) UpdateSequence en la consulta GetCapabilities es mayor que el valor actual del número de secuencia del servicio de actualización de los metadatos.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1604"/>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation>La solicitud no incluye un valor de muestra para la dimensión y el servidor no ha declarado un valor por omisión para esa dimensión.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1608"/>
        <source>Request contains an invalid sample dimension value.</source>
        <translation>La solicitud contiene un valor de muestra para la dimensión no válido.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1612"/>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>La solicitud es para una operación opcional no soportada por el servidor.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1616"/>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(Código de error desconocido de un servidor post-1.3 WMS)</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1619"/>
        <source>The WMS vendor also reported: </source>
        <translation>El productor WMS también informó: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1622"/>
        <source>This is probably due to a bug in the QGIS program.  Please report this error.</source>
        <translation>Esto probablemente se deba a un fallo en el programa QGIS. Por favor, informe de este error.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1806"/>
        <source>Server Properties:</source>
        <translation>Propiedades del servidor:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1937"/>
        <source>Property</source>
        <translation>Propiedad</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1940"/>
        <source>Value</source>
        <translation>Valor</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1823"/>
        <source>WMS Version</source>
        <translation>Versión WMS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2069"/>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2077"/>
        <source>Abstract</source>
        <translation>Resumen</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1847"/>
        <source>Keywords</source>
        <translation>Palabras clave</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1855"/>
        <source>Online Resource</source>
        <translation>Recursos en línea</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1863"/>
        <source>Contact Person</source>
        <translation>Persona de contacto</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1875"/>
        <source>Fees</source>
        <translation>Cuotas</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1883"/>
        <source>Access Constraints</source>
        <translation>Restricciones de acceso</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1891"/>
        <source>Image Formats</source>
        <translation>Formatos de imagen</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1899"/>
        <source>Identify Formats</source>
        <translation>Formatos de identificación</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1907"/>
        <source>Layer Count</source>
        <translation>Número de capas</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1927"/>
        <source>Layer Properties: </source>
        <translation>Propiedades de la capa: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1945"/>
        <source>Selected</source>
        <translation>Seleccionado</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2002"/>
        <source>Yes</source>
        <translation>Sí</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2002"/>
        <source>No</source>
        <translation>No</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1954"/>
        <source>Visibility</source>
        <translation>Visibilidad</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1960"/>
        <source>Visible</source>
        <translation>Visible</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1961"/>
        <source>Hidden</source>
        <translation>Oculta</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1962"/>
        <source>n/a</source>
        <translation>n/d</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1983"/>
        <source>Can Identify</source>
        <translation>Se puede identificar</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1991"/>
        <source>Can be Transparent</source>
        <translation>Puede ser transparente</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1999"/>
        <source>Can Zoom In</source>
        <translation>Se puede acercar el zum</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2007"/>
        <source>Cascade Count</source>
        <translation>Cuenta en cascada</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2015"/>
        <source>Fixed Width</source>
        <translation>Anchura fija</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2023"/>
        <source>Fixed Height</source>
        <translation>Altura fija</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2031"/>
        <source>WGS 84 Bounding Box</source>
        <translation>Marco de la WGS 84</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2041"/>
        <source>Available in CRS</source>
        <translation>Disponible en CRS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2052"/>
        <source>Available in style</source>
        <translation>Disponible en estilo</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2061"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2162"/>
        <source>Layer cannot be queried.</source>
        <translation>La capa no se puede consultar.</translation>
    </message>
</context>
<context>
    <name>[pluginname]Gui</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation>Plantilla de complementos de QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.ui" line="47"/>
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
