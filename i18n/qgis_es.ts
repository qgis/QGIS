<!DOCTYPE TS><TS>
<context>
    <name>CoordinateCapture</name>
    <message>
        <source>Coordinate Capture</source>
        <translation>Captura de coordenadas</translation>
    </message>
    <message>
        <source>Click on the map to view coordinates and capture to clipboard.</source>
        <translation>Pulse en el mapa para ver las coordenadas y copiarlas al portapapeles.</translation>
    </message>
    <message>
        <source>&amp;Coordinate Capture</source>
        <translation>&amp;Captura de coordenadas</translation>
    </message>
    <message>
        <source>Click to select the CRS to use for coordinate display</source>
        <translation>Pulse para seleccionar el SRC a usar para la visualización de coordenadas</translation>
    </message>
    <message>
        <source>Coordinate in your selected CRS</source>
        <translation>Coordenadas en su SRC seleccionado</translation>
    </message>
    <message>
        <source>Coordinate in map canvas coordinate reference system</source>
        <translation>Coordenadas en el sistema de referencia de coordenadas de la vista del mapa</translation>
    </message>
    <message>
        <source>Copy to clipboard</source>
        <translation>Copiar al portapapeles</translation>
    </message>
    <message>
        <source>Click to enable mouse tracking. Click the canvas to stop</source>
        <translation>Pulse para activar el seguimiento del ratón. Pulse el lienzo del mapa para detener.</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>¡Bienvenido a su complemento generado automáticamente!</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>Este es sólo un punto de inicio. Ahora necesita modificar el código para que haga algo útil... continúe leyendo para más información sobre cómo empezar.</translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation>Documentación:</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>Ahora necesita leer la documentación del API de QGIS en:</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation>En particular mire las siguientes clases:</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin es una ABC que define los comportamientos requeridos que su complemento debe proporcionar. Vea más abajo para más detalles.</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>¿Para qué son todos los archivos generados en mi directorio de complementos?</translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>Este es el archivo CMake que construye el complemento. Debería añadir las dependencias específicas de su aplicación y los archivos fuente a este archivo.</translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>Esta el la clase que proporciona el &quot;pegamento&quot; entre la lógica de su aplicación personal y la aplicación QGIS. Verá que ya hay implementados algunos métodos para usted - incluyendo algunos ejemplos de cómo añadir una capa ráster o vectorial a la vista principal del mapa de la aplicación. Esta clase es una instancia concreta de la interfaz QgisPlugin que define el comportamiento requerido para un complemento. En particular, un complemento tiene un número de métodos estáticos y miembros de forma que el QgsPluginManager y la lógica del cargador de complementos puedan identificar cada complemento, crear una entrada de menú apropiada para él, etc. Tenga en cuenta que no hay nada que le pare creando múltiples iconos de barras de herramientas y entradas de menú para un solo complemento. Sin embargo, de forma predeterminada sólo se crea una entrada de menú y un botón de barra de herramientas y está preconfigurado para llamar al método run() de esa clase cuando se selecciona. Esta implementación predeterminada proporcionada por el constructor de complementos está bien documentada, así que acuda al código para más información.</translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>Este es un archivo &quot;ui&quot; del diseñador de Qt. Define el aspecto del diálogo predeterminadado del complemento sin implementar ninguna lógica de la aplicación. Puede modificar este formulario para adaptarlo a sus necesidades o eliminarlo por completo si su complemento no necesita mostrar un formulario de usuario (por ej. para personalizar MapTools).</translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>Esta es la clase concreta donde debe ir la lógica de la aplicación para el diálogo mencionado anteriormente. Aquí es donde está el quiz de la cuestión...</translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>Este es el archivo de recursos de Qt4 para su complemento. El Makefile generado para su complemento está configurado por completo para compilar el archivo de recursos, así que todo lo que tiene que hacer es añadir sus iconos adicionales, etc. usando el sencillo formato de archivo xml. Fíjese en el namespace usado para todos sus recursos, ej. (&quot;:/[pluginname]/&quot;). Es importante usar este prefijo para todos sus recursos. Le sugerimos que incluya cualquier otra imagen y datos de tiempo de ejecución también en este archivo de recurso.</translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>Este es el icono que se usará para la entrada de menú e icono de barra de herramientas de su complemento. Simplemente sustituya este icono con el suyo para distinguirlo del resto.</translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation>¡Este archivo contiene la documentación que está leyendo ahora!</translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation>Obtener ayuda de los desarrolladores:</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>Para preguntas y comentarios sobre la plantilla del constructor de complementos y la creación de sus elementos en QGIS usando la interfaz de complementos, por favor póngase en contacto con nosotros via:</translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; la lista de correo de desarrolladores de QGIS o&lt;/li&gt;&lt;li&gt; IRC (#qgis en freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS se distribuye bajo la Licencia Pública Gnu (GPL). Si crea un complemento útil, por favor valore ponerlo a su vez a disposición de la comunidad.</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Diviértase y gracias por elegir QGIS.</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation>Plantilla de complementos de QGIS</translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation>Plantilla de complementos</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <source>Connect</source>
        <translation>Conectar</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Explorar</translation>
    </message>
    <message>
        <source>OGR Converter</source>
        <translation>Conversor OGR</translation>
    </message>
    <message>
        <source>Could not establish connection to: &apos;</source>
        <translation>No se pudo establecer conexión con: &apos;</translation>
    </message>
    <message>
        <source>Open OGR file</source>
        <translation>Abrir archivo OGR</translation>
    </message>
    <message>
        <source>OGR File Data Source (*.*)</source>
        <translation>Origen de datos de archivo OGR (*.*)</translation>
    </message>
    <message>
        <source>Open Directory</source>
        <translation>Abrir directorio</translation>
    </message>
    <message>
        <source>Input OGR dataset is missing!</source>
        <translation>¡Falta el conjunto de datos OGR de entrada!</translation>
    </message>
    <message>
        <source>Input OGR layer name is missing!</source>
        <translation>¡Falta el nombre de la capa OGR de entrada!</translation>
    </message>
    <message>
        <source>Target OGR format not selected!</source>
        <translation>¡No se ha seleccionado el formato OGR objetivo!</translation>
    </message>
    <message>
        <source>Output OGR dataset is missing!</source>
        <translation>¡Falta el conjunto de datos OGR de salida!</translation>
    </message>
    <message>
        <source>Output OGR layer name is missing!</source>
        <translation>¡Falta el nombre de la capa OGR de salida!</translation>
    </message>
    <message>
        <source>Successfully translated layer &apos;</source>
        <translation>Traducida correctamente la capa &apos;</translation>
    </message>
    <message>
        <source>Failed to translate layer &apos;</source>
        <translation>No se pudo traducir la capa &apos;</translation>
    </message>
    <message>
        <source>Successfully connected to: &apos;</source>
        <translation>Conectado correctamente a: &apos;</translation>
    </message>
    <message>
        <source>Choose a file name to save to</source>
        <translation>Seleccione un nombre de archivo en el que guardar</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>¡Bienvenido a su complemento generado automáticamente!</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>Este es sólo un punto de inicio. Ahora necesita modificar el código para que haga algo útil... continúe leyendo para más información sobre cómo empezar.</translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation>Documentación:</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>Ahora necesita leer la documentación del API de QGIS en:</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation>En particular mire las siguientes clases:</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin es una ABC que define los comportamientos requeridos que su complemento debe proporcionar. Vea más abajo para más detalles.</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>¿Para qué son todos los archivos generados en mi directorio de complementos?</translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>Este es el archivo CMake que construye el complemento. Debería añadir las dependencias específicas de su aplicación y los archivos fuente a este archivo.</translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>Esta el la clase que proporciona el &quot;pegamento&quot; entre la lógica de su aplicación personal y la aplicación QGIS. Verá que ya hay implementados algunos métodos para usted - incluyendo algunos ejemplos de cómo añadir una capa ráster o vectorial a la vista principal del mapa de la aplicación. Esta clase es una instancia concreta de la interfaz QgisPlugin que define el comportamiento requerido para un complemento. En particular, un complemento tiene un número de métodos estáticos y miembros de forma que el QgsPluginManager y la lógica del cargador de complementos puedan identificar cada complemento, crear una entrada de menú apropiada para él, etc. Tenga en cuenta que no hay nada que le pare creando múltiples iconos de barras de herramientas y entradas de menú para un solo complemento. Sin embargo, de forma predeterminada sólo se crea una entrada de menú y un botón de barra de herramientas y está preconfigurado para llamar al método run() de esa clase cuando se selecciona. Esta implementación predeterminada proporcionada por el constructor de complementos está bien documentada, así que acuda al código para más información.</translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>Este es un archivo &quot;ui&quot; del diseñador de Qt. Define el aspecto del diálogo predeterminadado del complemento sin implementar ninguna lógica de la aplicación. Puede modificar este formulario para adaptarlo a sus necesidades o eliminarlo por completo si su complemento no necesita mostrar un formulario de usuario (por ej. para personalizar MapTools).</translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>Esta es la clase concreta donde debe ir la lógica de la aplicación para el diálogo mencionado anteriormente. Aquí es donde está el quiz de la cuestión...</translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>Este es el archivo de recursos de Qt4 para su complemento. El Makefile generado para su complemento está configurado por completo para compilar el archivo de recursos, así que todo lo que tiene que hacer es añadir sus iconos adicionales, etc. usando el sencillo formato de archivo xml. Fíjese en el namespace usado para todos sus recursos, ej. (&quot;:/[pluginname]/&quot;). Es importante usar este prefijo para todos sus recursos. Le sugerimos que incluya cualquier otra imagen y datos de tiempo de ejecución también en este archivo de recurso.</translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>Este es el icono que se usará para la entrada de menú e icono de barra de herramientas de su complemento. Simplemente sustituya este icono con el suyo para distinguirlo del resto.</translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation>¡Este archivo contiene la documentación que está leyendo ahora!</translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation>Obtener ayuda de los desarrolladores:</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>Para preguntas y comentarios sobre la plantilla del constructor de complementos y la creación de sus elementos en QGIS usando la interfaz de complementos, por favor póngase en contacto con nosotros via:</translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; la lista de correo de desarrolladores de QGIS o&lt;/li&gt;&lt;li&gt; IRC (#qgis en freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS se distribuye bajo la Licencia Pública Gnu (GPL). Si crea un complemento útil, por favor valore ponerlo a su vez a disposición de la comunidad.</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Diviértase y gracias por elegir QGIS.</translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <source>Enter map coordinates</source>
        <translation>Introducir coordenadas de mapa</translation>
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
        <translation>&amp;Aceptar</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>Introducir las coordenadas X e Y que correspondan con el punto seleccionado en la imagen. De forma alternativa, seleccionar el botón con el icono de un lápiz y luego hacer clic en un punto correspondiente sobre la vista del mapa de QGIS para rellenar las coordenadas de ese punto.</translation>
    </message>
    <message>
        <source> from map canvas</source>
        <translation> de la vista del mapa</translation>
    </message>
</context>
<context>
    <name>OgrConverterGuiBase</name>
    <message>
        <source>OGR Layer Converter</source>
        <translation>Conversor de capas OGR</translation>
    </message>
    <message>
        <source>Source</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Formato</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Archivo</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation>Directorio</translation>
    </message>
    <message>
        <source>Remote source</source>
        <translation>Origen remoto</translation>
    </message>
    <message>
        <source>Dataset</source>
        <translation>Conjunto de datos</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Explorar</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Capa</translation>
    </message>
    <message>
        <source>Target</source>
        <translation>Objetivo</translation>
    </message>
</context>
<context>
    <name>OgrPlugin</name>
    <message>
        <source>Run OGR Layer Converter</source>
        <translation>Ejecutar el conversor de capas OGR</translation>
    </message>
    <message>
        <source>OG&amp;R Converter</source>
        <translation>Conversor OG&amp;R</translation>
    </message>
    <message>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation>Traduce capas vectoriales entre formatos admitidos por la biblioteca OGR</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation>Guardar informe del experimento a formato de documento portátil (.pdf)</translation>
    </message>
    <message>
        <source>Load layer properties from style file (.qml)</source>
        <translation>Cargar propiedades de la capa de archivo de estilo (.qml)</translation>
    </message>
    <message>
        <source>Save layer properties as style file (.qml)</source>
        <translation>Guardar propiedades de la capa como archivo de estilo (.qml)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Capturada una excepción del sistema de coordenadas mientras se intentaba transformar un punto.No se puede calcular la longitud de la línea.</translation>
    </message>
    <message>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation>Capturada una excepción del sistema de coordenadas mientras se intentaba transformar un punto.No se puede calcular el área del polígono.</translation>
    </message>
    <message>
        <source>Label</source>
        <translation>Etiqueta</translation>
    </message>
    <message>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation>No hay complementos de proveedores de datos</translation>
    </message>
    <message>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>No se pueden cargar capas vectoriales. Compruebe su instalación de QGIS.</translation>
    </message>
    <message>
        <source>No Data Providers</source>
        <translation>No hay proveedores de datos</translation>
    </message>
    <message>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>No hay complementos de proveedores de datos disponibles. No se pueden cargar capas vectoriales.</translation>
    </message>
    <message>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>No se ha encontrado la columna indicada: </translation>
    </message>
    <message>
        <source>Division by zero.</source>
        <translation>División entre cero.</translation>
    </message>
    <message>
        <source>QGis files (*.qgs)</source>
        <translation>Archivos QGis (*.qgs)</translation>
    </message>
    <message>
        <source>No active layer</source>
        <translation>No hay capa activa</translation>
    </message>
    <message>
        <source>Band</source>
        <translation>Banda</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Longitud</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Área</translation>
    </message>
    <message>
        <source>action</source>
        <translation>acción</translation>
    </message>
    <message>
        <source> features found</source>
        <translation> objetos espaciales encontrados</translation>
    </message>
    <message>
        <source> 1 feature found</source>
        <translation> 1 objeto espacial encontrado</translation>
    </message>
    <message>
        <source>No features found</source>
        <translation>No se han encontrado objetos espaciales</translation>
    </message>
    <message>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>No se han encontrado objetos espaciales en la capa activa en el punto en el que se ha pinchado</translation>
    </message>
    <message>
        <source>Could not identify objects on</source>
        <translation>No se pudieron identificar objetos en</translation>
    </message>
    <message>
        <source>because</source>
        <translation>porque</translation>
    </message>
    <message>
        <source>Project file read error: </source>
        <translation>Error de lectura del archivo del proyecto: </translation>
    </message>
    <message>
        <source> at line </source>
        <translation> en la línea </translation>
    </message>
    <message>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <source> for file </source>
        <translation> en el archivo </translation>
    </message>
    <message>
        <source>Unable to save to file </source>
        <translation>No se puede guardar a un archivo </translation>
    </message>
    <message>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>Ajustar a una transformación lineal requiere al menos 2 puntos.</translation>
    </message>
    <message>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Ajustar a una transformación de Helmert requiere al menos 2 puntos.</translation>
    </message>
    <message>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Ajustar a una transformación afín requiere al menos 4 puntos.</translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation>Nuevo centroide</translation>
    </message>
    <message>
        <source>New point</source>
        <translation>Nuevo punto</translation>
    </message>
    <message>
        <source>New vertex</source>
        <translation>Nuevo vértice</translation>
    </message>
    <message>
        <source>Undo last point</source>
        <translation>Deshacer último punto</translation>
    </message>
    <message>
        <source>Close line</source>
        <translation>Cerrar línea</translation>
    </message>
    <message>
        <source>Select vertex</source>
        <translation>Seleccionar vértice</translation>
    </message>
    <message>
        <source>Select new position</source>
        <translation>Seleccionar nueva posición</translation>
    </message>
    <message>
        <source>Select line segment</source>
        <translation>Seleccionar segmento de línea</translation>
    </message>
    <message>
        <source>New vertex position</source>
        <translation>Posición del nuevo vértice</translation>
    </message>
    <message>
        <source>Release</source>
        <translation>Liberar</translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation>Borrar vértice</translation>
    </message>
    <message>
        <source>Release vertex</source>
        <translation>Liberar vértice</translation>
    </message>
    <message>
        <source>Select element</source>
        <translation>Seleccionar elemento</translation>
    </message>
    <message>
        <source>New location</source>
        <translation>Nueva localización</translation>
    </message>
    <message>
        <source>Release selected</source>
        <translation>Liberar selección</translation>
    </message>
    <message>
        <source>Delete selected / select next</source>
        <translation>Borrar selección / seleccionar siguiente</translation>
    </message>
    <message>
        <source>Select position on line</source>
        <translation>Seleccionar posición en la línea</translation>
    </message>
    <message>
        <source>Split the line</source>
        <translation>Dividir la línea</translation>
    </message>
    <message>
        <source>Release the line</source>
        <translation>Liberar la línea</translation>
    </message>
    <message>
        <source>Select point on line</source>
        <translation>Seleccionar punto en la línea</translation>
    </message>
    <message>
        <source>Couldn&apos;t open the data source: </source>
        <translation>No se pudo abrir la fuente de datos: </translation>
    </message>
    <message>
        <source>Parse error at line </source>
        <translation>Error de análisis en la línea </translation>
    </message>
    <message>
        <source>GPS eXchange format provider</source>
        <translation>Proveedor de formato eXchange GPS</translation>
    </message>
    <message>
        <source>GRASS plugin</source>
        <translation>Complemento de GRASS</translation>
    </message>
    <message>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>QGIS no pudo encontrar su instalación de GRASS.
¿Podría especificar la ruta (GISBASE) de su instalación de GRASS?</translation>
    </message>
    <message>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Seleccionar ruta de instalación de GRASS (GISBASE)</translation>
    </message>
    <message>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>Los datos de GRASS no estarán accesibles si no se especifica una GISBASE.</translation>
    </message>
    <message>
        <source>CopyrightLabel</source>
        <translation>Etiqueta de Copyright</translation>
    </message>
    <message>
        <source>Draws copyright information</source>
        <translation>Dibuja información de copyright</translation>
    </message>
    <message>
        <source>Version 0.1</source>
        <translation>Versión 0.1</translation>
    </message>
    <message>
        <source>Version 0.2</source>
        <translation>Versión 0.2</translation>
    </message>
    <message>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation>Carga y muestra archivos de texto delimitado que contienen coordenadas X, Y</translation>
    </message>
    <message>
        <source>Add Delimited Text Layer</source>
        <translation>Añadir capa de texto delimitado</translation>
    </message>
    <message>
        <source>Georeferencer</source>
        <translation>Georreferenciador</translation>
    </message>
    <message>
        <source>Adding projection info to rasters</source>
        <translation>Añadir información de proyección a los rásters</translation>
    </message>
    <message>
        <source>GPS Tools</source>
        <translation>Herramientas de GPS</translation>
    </message>
    <message>
        <source>Tools for loading and importing GPS data</source>
        <translation>Herramientas para cargar e importar datos de GPS</translation>
    </message>
    <message>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation>Capa de GRASS</translation>
    </message>
    <message>
        <source>Graticule Creator</source>
        <translation>Creador de cuadrícula</translation>
    </message>
    <message>
        <source>Builds a graticule</source>
        <translation>Construye una cuadrícula</translation>
    </message>
    <message>
        <source>NorthArrow</source>
        <translation>Flecha de Norte</translation>
    </message>
    <message>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Muestra una flecha de Norte superpuesta en el mapa</translation>
    </message>
    <message>
        <source>[menuitemname]</source>
        <translation>[nombredeelementodemenu]</translation>
    </message>
    <message>
        <source>[plugindescription]</source>
        <translation>[descripcióndecomplemento]</translation>
    </message>
    <message>
        <source>ScaleBar</source>
        <translation>Barra de escala</translation>
    </message>
    <message>
        <source>Draws a scale bar</source>
        <translation>Dibuja una barra de escala</translation>
    </message>
    <message>
        <source>SPIT</source>
        <translation>SPIT</translation>
    </message>
    <message>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>Herramienta de importación de shapefiles a PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <source>WFS plugin</source>
        <translation>Complemento de WFS</translation>
    </message>
    <message>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>Añade capas WFS a la vista de QGIS</translation>
    </message>
    <message>
        <source>Not a vector layer</source>
        <translation>No es una capa vectorial</translation>
    </message>
    <message>
        <source>The current layer is not a vector layer</source>
        <translation>La capa actual no es vectorial</translation>
    </message>
    <message>
        <source>Layer cannot be added to</source>
        <translation>La capa no se puede añadir a</translation>
    </message>
    <message>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>El proveedor de datos de esta capa no da soporte para añadir objetos espaciales.</translation>
    </message>
    <message>
        <source>Layer not editable</source>
        <translation>Capa no editable</translation>
    </message>
    <message>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>No se puede editar la capa vectorial. Para hacerla editable vaya al archivo de la capa, haga clic derecho y marque &apos;Permitir edición&apos;.</translation>
    </message>
    <message>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Para seleccionar objetos espaciales debe elegir una capa vectorial haciendo clic en su nombre en la leyenda</translation>
    </message>
    <message>
        <source>Python error</source>
        <translation>Error de Python</translation>
    </message>
    <message>
        <source>Couldn&apos;t load plugin </source>
        <translation>No se pudo cargar el complemento </translation>
    </message>
    <message>
        <source> due an error when calling its classFactory() method</source>
        <translation> debido a un error al llamar a su método classFactory()</translation>
    </message>
    <message>
        <source> due an error when calling its initGui() method</source>
        <translation> debido a un error al llamar a su método initGui()</translation>
    </message>
    <message>
        <source>Error while unloading plugin </source>
        <translation>Error al descargar complemento </translation>
    </message>
    <message>
        <source>2.5D shape type not supported</source>
        <translation>El tipo shape 2.5D no está soportado</translation>
    </message>
    <message>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>Añadir objetos espaciales a tipo shape 2.5 aún no está soportado</translation>
    </message>
    <message>
        <source>Wrong editing tool</source>
        <translation>Herramienta de edición incorrecta</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>No se puede aplicar la herramienta &apos;capturar punto&apos; en esta capa vectorial</translation>
    </message>
    <message>
        <source>Coordinate transform error</source>
        <translation>Error de transformación de coordenadas</translation>
    </message>
    <message>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>No se puede transformar el punto al sistema de coordenadas de las capas</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>No se puede aplicar la herramienta &apos;capturar línea&apos; en esta capa vectorial</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>No se puede aplicar la herramienta &apos;capturar polígono&apos; en esta capa vectorial</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>No se puede añadir el objeto espacial. Tipo WKB desconocido.</translation>
    </message>
    <message>
        <source>Error, could not add island</source>
        <translation>Error, no se pudo añadir la isla</translation>
    </message>
    <message>
        <source>A problem with geometry type occured</source>
        <translation>Ocurrió un problema con el tipo de geometría</translation>
    </message>
    <message>
        <source>The inserted Ring is not closed</source>
        <translation>El anillo insertado no está cerrado</translation>
    </message>
    <message>
        <source>The inserted Ring is not a valid geometry</source>
        <translation>El anillo insertado no es una geometría válida</translation>
    </message>
    <message>
        <source>The inserted Ring crosses existing rings</source>
        <translation>El anillo insertado cruza anillos existentes</translation>
    </message>
    <message>
        <source>The inserted Ring is not contained in a feature</source>
        <translation>El anillo insertado no está contenido en un objeto espacial</translation>
    </message>
    <message>
        <source>An unknown error occured</source>
        <translation>Ocurrió un error desconocido</translation>
    </message>
    <message>
        <source>Error, could not add ring</source>
        <translation>Error, no se pudo añadir el anillo</translation>
    </message>
    <message>
        <source> km2</source>
        <translation> km²</translation>
    </message>
    <message>
        <source> ha</source>
        <translation> Ha</translation>
    </message>
    <message>
        <source> m2</source>
        <translation> m²</translation>
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
        <translation> milla²</translation>
    </message>
    <message>
        <source> sq ft</source>
        <translation> pies²</translation>
    </message>
    <message>
        <source> mile</source>
        <translation> milla</translation>
    </message>
    <message>
        <source> foot</source>
        <translation> pie</translation>
    </message>
    <message>
        <source> feet</source>
        <translation> pies</translation>
    </message>
    <message>
        <source> sq.deg.</source>
        <translation> grados²</translation>
    </message>
    <message>
        <source> degree</source>
        <translation> grado</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation> grados</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation> desconocido</translation>
    </message>
    <message>
        <source>Received %1 of %2 bytes</source>
        <translation>Recibidos %1 de %2 bytes</translation>
    </message>
    <message>
        <source>Received %1 bytes (total unknown)</source>
        <translation>Recibidos %1 bytes (total desconocido)</translation>
    </message>
    <message>
        <source>Not connected</source>
        <translation>No conectado</translation>
    </message>
    <message>
        <source>Looking up &apos;%1&apos;</source>
        <translation>Buscando &apos;%1&apos;</translation>
    </message>
    <message>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>Conectando a &apos;%1&apos;</translation>
    </message>
    <message>
        <source>Sending request &apos;%1&apos;</source>
        <translation>Enviando petición &apos;%1&apos;</translation>
    </message>
    <message>
        <source>Receiving reply</source>
        <translation>Recibiendo respuesta</translation>
    </message>
    <message>
        <source>Response is complete</source>
        <translation>La respuesta está completa</translation>
    </message>
    <message>
        <source>Closing down connection</source>
        <translation>Cerrando la conexión</translation>
    </message>
    <message>
        <source>Unable to open </source>
        <translation>No se puede abrir </translation>
    </message>
    <message>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation>Las expresiones regulares sobre valores numéricos no tienen sentido. Use la comparación en su lugar.</translation>
    </message>
    <message>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Funciones de geoprocesamiento para trabajar con capas PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Localización: </translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Directorio de mapas: </translation>
    </message>
    <message>
        <source>Location: </source>
        <translation>Localización: </translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Directorio de mapas: </translation>
    </message>
    <message>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation>&lt;b&gt;Ráster&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Cannot open raster header</source>
        <translation>No se puede abrir la cabecera del ráster</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation>Filas</translation>
    </message>
    <message>
        <source>Columns</source>
        <translation>Columnas</translation>
    </message>
    <message>
        <source>N-S resolution</source>
        <translation>Resolución N-S</translation>
    </message>
    <message>
        <source>E-W resolution</source>
        <translation>Resolución E-W</translation>
    </message>
    <message>
        <source>North</source>
        <translation>Norte</translation>
    </message>
    <message>
        <source>South</source>
        <translation>Sur</translation>
    </message>
    <message>
        <source>East</source>
        <translation>Este</translation>
    </message>
    <message>
        <source>West</source>
        <translation>Oeste</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Formato</translation>
    </message>
    <message>
        <source>Minimum value</source>
        <translation>Valor mínimo</translation>
    </message>
    <message>
        <source>Maximum value</source>
        <translation>Valor máximo</translation>
    </message>
    <message>
        <source>Data source</source>
        <translation>Fuente de datos</translation>
    </message>
    <message>
        <source>Data description</source>
        <translation>Descripción de datos</translation>
    </message>
    <message>
        <source>Comments</source>
        <translation>Comentarios</translation>
    </message>
    <message>
        <source>Categories</source>
        <translation>Categorías</translation>
    </message>
    <message>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;Vectorial&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Points</source>
        <translation>Puntos</translation>
    </message>
    <message>
        <source>Lines</source>
        <translation>Líneas</translation>
    </message>
    <message>
        <source>Boundaries</source>
        <translation>Contornos</translation>
    </message>
    <message>
        <source>Centroids</source>
        <translation>Centroides</translation>
    </message>
    <message>
        <source>Faces</source>
        <translation>Caras</translation>
    </message>
    <message>
        <source>Kernels</source>
        <translation>Kernels</translation>
    </message>
    <message>
        <source>Areas</source>
        <translation>Áreas</translation>
    </message>
    <message>
        <source>Islands</source>
        <translation>Islas</translation>
    </message>
    <message>
        <source>Top</source>
        <translation>Arriba</translation>
    </message>
    <message>
        <source>Bottom</source>
        <translation>Abajo</translation>
    </message>
    <message>
        <source>yes</source>
        <translation>sí</translation>
    </message>
    <message>
        <source>no</source>
        <translation>no</translation>
    </message>
    <message>
        <source>History&lt;br&gt;</source>
        <translation>Historia&lt;br&gt;</translation>
    </message>
    <message>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Capa&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Features</source>
        <translation>Objetos espaciales</translation>
    </message>
    <message>
        <source>Driver</source>
        <translation>Controlador</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>Base de datos</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabla</translation>
    </message>
    <message>
        <source>Key column</source>
        <translation>Columna clave</translation>
    </message>
    <message>
        <source>GISBASE is not set.</source>
        <translation>GISBASE no establecida.</translation>
    </message>
    <message>
        <source> is not a GRASS mapset.</source>
        <translation> no es un directorio de mapas de GRASS.</translation>
    </message>
    <message>
        <source>Cannot start </source>
        <translation>No se puede iniciar </translation>
    </message>
    <message>
        <source>Mapset is already in use.</source>
        <translation>El directorio de mapas ya está en uso.</translation>
    </message>
    <message>
        <source>Temporary directory </source>
        <translation>El directorio temporal </translation>
    </message>
    <message>
        <source> exist but is not writable</source>
        <translation> existe pero no se puede escribir</translation>
    </message>
    <message>
        <source>Cannot create temporary directory </source>
        <translation>No se puede crear el directorio temporal </translation>
    </message>
    <message>
        <source>Cannot create </source>
        <translation>No se puede crear </translation>
    </message>
    <message>
        <source>Cannot remove mapset lock: </source>
        <translation>No se puede eliminar el bloqueo del directorio de mapas: </translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>Cannot read raster map region</source>
        <translation>No se puede la región del mapa ráster</translation>
    </message>
    <message>
        <source>Cannot read vector map region</source>
        <translation>No se puede leer la región del mapa vectorial</translation>
    </message>
    <message>
        <source>Cannot read region</source>
        <translation>No se puede leer la región</translation>
    </message>
    <message>
        <source>Where is &apos;</source>
        <translation>¿Dónde está &apos;</translation>
    </message>
    <message>
        <source>original location: </source>
        <translation>localización original: </translation>
    </message>
    <message>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Para identificar objetos espaciales, debe activar una capa haciendo clic en su nombre en la leyenda</translation>
    </message>
    <message>
        <source>PostgreSQL Geoprocessing</source>
        <translation>Geoprocesamiento PostgreSQL</translation>
    </message>
    <message>
        <source>Quick Print</source>
        <translation>Impresión rápida</translation>
    </message>
    <message>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation>Impresión rápida es un complemento para imprimir rápidamente un mapa con el mínimo esfuerzo.</translation>
    </message>
    <message>
        <source>Could not remove polygon intersection</source>
        <translation>No se pudo eliminar la intersección de los polígonos</translation>
    </message>
    <message>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation>Se tiene que poder escribir en el directorio que contiene su conjunto de datos.</translation>
    </message>
    <message>
        <source>Created default style file as </source>
        <translation>Se ha creado el archivo de estilo predeterminado </translation>
    </message>
    <message>
        <source> is not writeable.</source>
        <translation> no se puede editar.</translation>
    </message>
    <message>
        <source>Please adjust permissions (if possible) and try again.</source>
        <translation>Por favor, ajuste los permisos (si es posible) y vuelva a intentarlo.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load SIP module.</source>
        <translation>No se pudo cargar el módulo SIP.</translation>
    </message>
    <message>
        <source>Python support will be disabled.</source>
        <translation>El soporte para Python estará deshabilitado.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQt4.</source>
        <translation>No se pudo cargar PyQt4.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQGIS.</source>
        <translation>No se pudo cargar PyQGIS.</translation>
    </message>
    <message>
        <source>An error has occured while executing Python code:</source>
        <translation>Ha ocurrido un error mientras se ejecutaba el código de Python:</translation>
    </message>
    <message>
        <source>Python version:</source>
        <translation>Versión de Python:</translation>
    </message>
    <message>
        <source>Python path:</source>
        <translation>Ruta de Python:</translation>
    </message>
    <message>
        <source>An error occured during execution of following code:</source>
        <translation>Ocurrió un error durante la ejecución del siguiente código:</translation>
    </message>
    <message>
        <source>Uncatched fatal GRASS error</source>
        <translation>Error fatal de GRASS no detectado</translation>
    </message>
    <message>
        <source>ERROR: Failed to created default style file as %1 Check file permissions and retry.</source>
        <translation>ERROR: no se pudo crear el archivo de estilo predefinido como %1. Compruebe los permisos del archivo y vuelva a probar.</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Leyenda</translation>
    </message>
    <message>
        <source>Coordinate Capture</source>
        <translation>Captura de coordenadas</translation>
    </message>
    <message>
        <source>Capture mouse coordinates in different CRS</source>
        <translation>Capturar las coordenadas del ratón en diferentes SRC</translation>
    </message>
    <message>
        <source>Dxf2Shp Converter</source>
        <translation>Conversor DxfaShp</translation>
    </message>
    <message>
        <source>Converts from dxf to shp file format</source>
        <translation>Convierte de formato de archivo dxf a shp</translation>
    </message>
    <message>
        <source>Interpolating...</source>
        <translation>Interpolando...</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation>Abortar</translation>
    </message>
    <message>
        <source>Interpolation plugin</source>
        <translation>Complemento de interpolación</translation>
    </message>
    <message>
        <source>A plugin for interpolation based on vertices of a vector layer</source>
        <translation>Un complemento para interpolación basado en los vértices de una capa vectorial</translation>
    </message>
    <message>
        <source>Version 0.001</source>
        <translation>Versión 0.001</translation>
    </message>
    <message>
        <source>OGR Layer Converter</source>
        <translation>Conversor de capas OGR</translation>
    </message>
    <message>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation>Traduce capas vectoriales entre formatos admitidos por la biblioteca OGR</translation>
    </message>
    <message>
        <source>CRS Exception</source>
        <translation>Excepción de SRC</translation>
    </message>
    <message>
        <source>Selection extends beyond layer&apos;s coordinate system.</source>
        <translation>La selección se extiende más allá del sistema de coordenadas de la capa.</translation>
    </message>
    <message>
        <source>Loading style file </source>
        <translation>La carga del archivo de estilo </translation>
    </message>
    <message>
        <source> failed because:</source>
        <translation> falló porque:</translation>
    </message>
    <message>
        <source>Could not save symbology because:</source>
        <translation>No se pudo guardar la simbología porque:</translation>
    </message>
    <message>
        <source>Unable to save to file. Your project may be corrupted on disk. Try clearing some space on the volume and check file permissions before pressing save again.</source>
        <translation>No se puede guardar en archivo. Puede que su proyecto esté dañado en el disco. Pruebe a liberar espacio en el volumen y compruebe los permisos de archivo antes de pulsar guardar de nuevo.</translation>
    </message>
    <message>
        <source>Error Loading Plugin</source>
        <translation type="unfinished">Error al cargar el complemento</translation>
    </message>
    <message>
        <source>There was an error loading a plugin.The following diagnostic information may help the QGIS developers resolve the issue:
%1.</source>
        <translation type="unfinished">Hubo un problema al cargar un complemento. La siguiente información de diagnóstico puede ayudar a los desarrolladores de QGIS a solucionar el asunto:
%1.</translation>
    </message>
    <message>
        <source>Error when reading metadata of plugin </source>
        <translation type="unfinished">Error al leer metadatos del complemento </translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS - </translation>
    </message>
    <message>
        <source>Checking database</source>
        <translation>Comprobando la base de datos</translation>
    </message>
    <message>
        <source>Reading settings</source>
        <translation>Leyendo configuraciones</translation>
    </message>
    <message>
        <source>Setting up the GUI</source>
        <translation>Configurando la interfaz gráfica de usuario (GUI)</translation>
    </message>
    <message>
        <source>Restoring loaded plugins</source>
        <translation>Restableciendo complementos cargados</translation>
    </message>
    <message>
        <source>Initializing file filters</source>
        <translation>Inicializando filtros de archivo</translation>
    </message>
    <message>
        <source>Restoring window state</source>
        <translation>Restableciendo el estado de la ventana</translation>
    </message>
    <message>
        <source>QGIS Ready!</source>
        <translation>¡QGIS preparado!</translation>
    </message>
    <message>
        <source>&amp;New Project</source>
        <translation>&amp;Nuevo proyecto</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <source>New Project</source>
        <translation>Nuevo proyecto</translation>
    </message>
    <message>
        <source>&amp;Open Project...</source>
        <translation>&amp;Abrir proyecto...</translation>
    </message>
    <message>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation>Ctrl+A</translation>
    </message>
    <message>
        <source>Open a Project</source>
        <translation>Abrir un proyecto</translation>
    </message>
    <message>
        <source>&amp;Save Project</source>
        <translation>&amp;Guardar proyecto</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation>Ctrl+G</translation>
    </message>
    <message>
        <source>Save Project</source>
        <translation>Guardar proyecto</translation>
    </message>
    <message>
        <source>Save Project &amp;As...</source>
        <translation>G&amp;uardar proyecto como...</translation>
    </message>
    <message>
        <source>Save Project under a new name</source>
        <translation>Guardar proyecto con un nombre nuevo</translation>
    </message>
    <message>
        <source>Save as Image...</source>
        <translation>Guardar como imagen...</translation>
    </message>
    <message>
        <source>Save map as image</source>
        <translation>Guardar mapa como imagen</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation>Salir</translation>
    </message>
    <message>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <source>Exit QGIS</source>
        <translation>Salir de QGIS</translation>
    </message>
    <message>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation>V</translation>
    </message>
    <message>
        <source>Add a Vector Layer</source>
        <translation>Añadir una capa vectorial</translation>
    </message>
    <message>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation>R</translation>
    </message>
    <message>
        <source>Add a Raster Layer</source>
        <translation>Añadir una capa ráster</translation>
    </message>
    <message>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation>D</translation>
    </message>
    <message>
        <source>Add a PostGIS Layer</source>
        <translation>Añadir una capa de PostGIS</translation>
    </message>
    <message>
        <source>New Vector Layer...</source>
        <translation>Nueva capa vectorial...</translation>
    </message>
    <message>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation>N</translation>
    </message>
    <message>
        <source>Create a New Vector Layer</source>
        <translation>Crear una capa vectorial nueva</translation>
    </message>
    <message>
        <source>Remove Layer</source>
        <translation>Eliminar capa</translation>
    </message>
    <message>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation>Ctrl+E</translation>
    </message>
    <message>
        <source>Remove a Layer</source>
        <translation>Eliminar una capa</translation>
    </message>
    <message>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation>+</translation>
    </message>
    <message>
        <source>Show all layers in the overview map</source>
        <translation>Mostrar todas las capas en la vista general</translation>
    </message>
    <message>
        <source>Remove All From Overview</source>
        <translation>Eliminar todo de la vista general</translation>
    </message>
    <message>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation>-</translation>
    </message>
    <message>
        <source>Remove all layers from overview map</source>
        <translation>Eliminar todas las capas de la vista general</translation>
    </message>
    <message>
        <source>Show All Layers</source>
        <translation>Mostrar todas las capas</translation>
    </message>
    <message>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation>M</translation>
    </message>
    <message>
        <source>Show all layers</source>
        <translation>Mostrar todas las capas</translation>
    </message>
    <message>
        <source>Hide All Layers</source>
        <translation>Ocultar todas las capas</translation>
    </message>
    <message>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation>O</translation>
    </message>
    <message>
        <source>Hide all layers</source>
        <translation>Ocultar todas las capas</translation>
    </message>
    <message>
        <source>Project Properties...</source>
        <translation>Propiedades del proyecto...</translation>
    </message>
    <message>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation>P</translation>
    </message>
    <message>
        <source>Set project properties</source>
        <translation>Definir las propiedades del proyecto</translation>
    </message>
    <message>
        <source>Options...</source>
        <translation>Opciones...</translation>
    </message>
    <message>
        <source>Change various QGIS options</source>
        <translation>Cambiar varias opciones de QGIS</translation>
    </message>
    <message>
        <source>Help Contents</source>
        <translation>Contenidos de la ayuda</translation>
    </message>
    <message>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation>Ctrl+?</translation>
    </message>
    <message>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation>F1</translation>
    </message>
    <message>
        <source>Help Documentation</source>
        <translation>Documentación de ayuda</translation>
    </message>
    <message>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation>Ctrl+W</translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>Página web de QGIS</translation>
    </message>
    <message>
        <source>About</source>
        <translation>Acerca de</translation>
    </message>
    <message>
        <source>About QGIS</source>
        <translation>Acerca de QGIS</translation>
    </message>
    <message>
        <source>Check Qgis Version</source>
        <translation>Comprobar versión de Qgis</translation>
    </message>
    <message>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Comprobar si su versión de QGIS está actualizada (requiere acceso a internet)</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
    <message>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation>Ctrl+A</translation>
    </message>
    <message>
        <source>Refresh Map</source>
        <translation>Actualizar mapa</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Acercar zum</translation>
    </message>
    <message>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation>Ctrl++</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Alejar zum</translation>
    </message>
    <message>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation>Ctrl+-</translation>
    </message>
    <message>
        <source>Zoom Full</source>
        <translation>Zum general</translation>
    </message>
    <message>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation>G</translation>
    </message>
    <message>
        <source>Zoom to Full Extents</source>
        <translation>Zum a toda la extensión</translation>
    </message>
    <message>
        <source>Pan Map</source>
        <translation>Desplazar mapa</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation>Desplazar el mapa</translation>
    </message>
    <message>
        <source>Zoom Last</source>
        <translation>Zum anterior</translation>
    </message>
    <message>
        <source>Zoom to Last Extent</source>
        <translation>Zum a la extensión anterior</translation>
    </message>
    <message>
        <source>Zoom to Layer</source>
        <translation>Zum a la capa</translation>
    </message>
    <message>
        <source>Identify Features</source>
        <translation>Identificar objetos espaciales</translation>
    </message>
    <message>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation>I</translation>
    </message>
    <message>
        <source>Click on features to identify them</source>
        <translation>Pulsar sobre los objetos espaciales para identificarlos</translation>
    </message>
    <message>
        <source>Select Features</source>
        <translation>Seleccionar objetos espaciales</translation>
    </message>
    <message>
        <source>Measure Line </source>
        <translation>Regla </translation>
    </message>
    <message>
        <source>Measure a Line</source>
        <translation>Regla</translation>
    </message>
    <message>
        <source>Measure Area</source>
        <translation>Medir áreas</translation>
    </message>
    <message>
        <source>Measure an Area</source>
        <translation>Medir un área</translation>
    </message>
    <message>
        <source>Show Bookmarks</source>
        <translation>Mostrar marcadores</translation>
    </message>
    <message>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation>M</translation>
    </message>
    <message>
        <source>New Bookmark...</source>
        <translation>Nuevo marcador...</translation>
    </message>
    <message>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation>Ctrl+M</translation>
    </message>
    <message>
        <source>New Bookmark</source>
        <translation>Nuevo marcador</translation>
    </message>
    <message>
        <source>Add WMS Layer...</source>
        <translation>Añadir capa WMS...</translation>
    </message>
    <message>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation>L</translation>
    </message>
    <message>
        <source>Add current layer to overview map</source>
        <translation>Añadir la capa actual a la vista general</translation>
    </message>
    <message>
        <source>Open the plugin manager</source>
        <translation>Abrir el administrador de complementos</translation>
    </message>
    <message>
        <source>Capture Point</source>
        <translation>Capturar punto</translation>
    </message>
    <message>
        <source>.</source>
        <comment>Capture Points</comment>
        <translation>.</translation>
    </message>
    <message>
        <source>Capture Points</source>
        <translation>Capturar puntos</translation>
    </message>
    <message>
        <source>Capture Line</source>
        <translation>Capturar línea</translation>
    </message>
    <message>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation>/</translation>
    </message>
    <message>
        <source>Capture Lines</source>
        <translation>Capturar líneas</translation>
    </message>
    <message>
        <source>Capture Polygon</source>
        <translation>Capturar polígono</translation>
    </message>
    <message>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation>Ctrl+/</translation>
    </message>
    <message>
        <source>Capture Polygons</source>
        <translation>Capturar polígonos</translation>
    </message>
    <message>
        <source>Delete Selected</source>
        <translation>Borrar lo seleccionado</translation>
    </message>
    <message>
        <source>Add Vertex</source>
        <translation>Añadir vértice</translation>
    </message>
    <message>
        <source>Delete Vertex</source>
        <translation>Borrar vértice</translation>
    </message>
    <message>
        <source>Move Vertex</source>
        <translation>Mover vértice</translation>
    </message>
    <message>
        <source>Cut Features</source>
        <translation>Cortar objetos espaciales</translation>
    </message>
    <message>
        <source>Cut selected features</source>
        <translation>Cortar los objetos espaciales seleccionados</translation>
    </message>
    <message>
        <source>Copy Features</source>
        <translation>Copiar objetos espaciales</translation>
    </message>
    <message>
        <source>Copy selected features</source>
        <translation>Copiar los objetos espaciales seleccionados</translation>
    </message>
    <message>
        <source>Paste Features</source>
        <translation>Pegar objetos espaciales</translation>
    </message>
    <message>
        <source>Paste selected features</source>
        <translation>Pegar los objetos espaciales seleccionados</translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>&amp;Archivo</translation>
    </message>
    <message>
        <source>&amp;Open Recent Projects</source>
        <translation>Abrir proyectos &amp;recientes</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Ver</translation>
    </message>
    <message>
        <source>&amp;Layer</source>
        <translation>&amp;Capa</translation>
    </message>
    <message>
        <source>&amp;Settings</source>
        <translation>C&amp;onfiguración</translation>
    </message>
    <message>
        <source>&amp;Plugins</source>
        <translation>Co&amp;mplementos</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>A&amp;yuda</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Archivo</translation>
    </message>
    <message>
        <source>Manage Layers</source>
        <translation>Administrar capas</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitalización</translation>
    </message>
    <message>
        <source>Map Navigation</source>
        <translation>Navegación de mapas</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Atributos</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation>Complementos</translation>
    </message>
    <message>
        <source>Displays the current map scale</source>
        <translation>Muestra la escala del mapa actual</translation>
    </message>
    <message>
        <source>Render</source>
        <translation>Representar</translation>
    </message>
    <message>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Cuando está marcada, las capas se representan respondiendo a los comandos de navegación y otros eventos. Cuando no está marcada, la representación no se hace. Esto permite añadir un gran número de capas y simbolizarlas antes de su representación.</translation>
    </message>
    <message>
        <source>Toggle map rendering</source>
        <translation>Conmutar la representación del mapa</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation>Preparado</translation>
    </message>
    <message>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Vista general del mapa. Esta vista puede usarse para visualizar un mapa de localización que muestra la extensión de la vista del mapa. La extensión actual se muestra como un rectángulo rojo. Cualquier capa del mapa se puede añadir a la vista general.</translation>
    </message>
    <message>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>Leyenda del mapa que muestra todas la capas actualmente en la vista del mapa. Marcar la casilla para conmutar su vista. Pulsar dos veces sobre una capa en la leyenda para personalizar su apariencia y ajustar otras propiedades.</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Versión</translation>
    </message>
    <message>
        <source>New features</source>
        <translation>Nuevos objetos espaciales</translation>
    </message>
    <message>
        <source>Checking provider plugins</source>
        <translation>Comprobando complementos del proveedor</translation>
    </message>
    <message>
        <source>Starting Python</source>
        <translation>Iniciando Python</translation>
    </message>
    <message>
        <source>Python error</source>
        <translation type="obsolete">Error de Python</translation>
    </message>
    <message>
        <source>Error when reading metadata of plugin </source>
        <translation type="obsolete">Error al leer metadatos del complemento </translation>
    </message>
    <message>
        <source>Toggle editing</source>
        <translation>Conmutar edición</translation>
    </message>
    <message>
        <source>Toggles the editing state of the current layer</source>
        <translation>Conmuta el estado de edición de la capa activa</translation>
    </message>
    <message>
        <source>Add Ring</source>
        <translation>Añadir anillo</translation>
    </message>
    <message>
        <source>Add Island</source>
        <translation>Añadir isla</translation>
    </message>
    <message>
        <source>Add Island to multipolygon</source>
        <translation>Añadir isla a multipolígono</translation>
    </message>
    <message>
        <source>Scale </source>
        <translation>Escala </translation>
    </message>
    <message>
        <source>Current map scale (formatted as x:y)</source>
        <translation>Escala actual del mapa (en formato X:Y)</translation>
    </message>
    <message>
        <source>Map coordinates at mouse cursor position</source>
        <translation>Coordenadas del mapa en la posición del ratón</translation>
    </message>
    <message>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>Abrir una capa vectorial soportada por OGR</translation>
    </message>
    <message>
        <source>is not a valid or recognized data source</source>
        <translation>no es una fuente de datos válida o reconocida</translation>
    </message>
    <message>
        <source>Invalid Data Source</source>
        <translation>Fuente de datos no válida</translation>
    </message>
    <message>
        <source>Invalid Layer</source>
        <translation>Capa no válida</translation>
    </message>
    <message>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>%1 es una capa no válida y no se puede cargar.</translation>
    </message>
    <message>
        <source>Save As</source>
        <translation>Guardar como</translation>
    </message>
    <message>
        <source>Choose a QGIS project file to open</source>
        <translation>Seleccionar un archivo de proyecto de QGIS para abrir</translation>
    </message>
    <message>
        <source>QGIS Project Read Error</source>
        <translation>Error de lectura del proyecto de QGIS</translation>
    </message>
    <message>
        <source>Try to find missing layers?</source>
        <translation>¿Buscar las capas perdidas?</translation>
    </message>
    <message>
        <source>Unable to open project</source>
        <translation>No se puede abrir el proyecto</translation>
    </message>
    <message>
        <source>Choose a QGIS project file</source>
        <translation>Seleccionar un archivo de proyecto de QGIS</translation>
    </message>
    <message>
        <source>Saved project to:</source>
        <translation>Proyecto guardado en:</translation>
    </message>
    <message>
        <source>Unable to save project</source>
        <translation>No se puede guardar el proyecto</translation>
    </message>
    <message>
        <source>Unable to save project to </source>
        <translation>No se puede guardar el proyecto en </translation>
    </message>
    <message>
        <source>Unable to save project </source>
        <translation>No se puede guardar el proyecto </translation>
    </message>
    <message>
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: No se puede cargar el proyecto</translation>
    </message>
    <message>
        <source>Unable to load project </source>
        <translation>No se puede cargar el proyecto </translation>
    </message>
    <message>
        <source>Saved map image to</source>
        <translation>Imagen del mapa guardada en</translation>
    </message>
    <message>
        <source>No Layer Selected</source>
        <translation>Ninguna capa seleccionada</translation>
    </message>
    <message>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation>Para borrar objetos espaciales, debe seleccionar una capa vectorial en la leyenda</translation>
    </message>
    <message>
        <source>No Vector Layer Selected</source>
        <translation>Ninguna capa vectorial seleccionada</translation>
    </message>
    <message>
        <source>Deleting features only works on vector layers</source>
        <translation>Borrar objetos espaciales solo funciona en capas vectoriales</translation>
    </message>
    <message>
        <source>Provider does not support deletion</source>
        <translation>El proveedor no soporta el borrado</translation>
    </message>
    <message>
        <source>Data provider does not support deleting features</source>
        <translation>El proveedor de datos no soporta el borrado de objetos espaciales</translation>
    </message>
    <message>
        <source>Layer not editable</source>
        <translation>Capa no editable</translation>
    </message>
    <message>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation>La capa activa no se puede editar. Elegir &apos;Conmutar edición&apos; en la barra de herramientas Digitalización.</translation>
    </message>
    <message>
        <source>Problem deleting features</source>
        <translation>Problema al borrar objetos espaciales</translation>
    </message>
    <message>
        <source>A problem occured during deletion of features</source>
        <translation>Ha ocurrido un problema durante el borrado de objetos espaciales</translation>
    </message>
    <message>
        <source>Invalid scale</source>
        <translation>Escala no válida</translation>
    </message>
    <message>
        <source>Error Loading Plugin</source>
        <translation type="obsolete">Error al cargar el complemento</translation>
    </message>
    <message>
        <source>There was an error loading %1.</source>
        <translation type="obsolete">Ha habido un error al cargar %1.</translation>
    </message>
    <message>
        <source>There is a new version of QGIS available</source>
        <translation>Hay una nueva versión de QGIS disponible</translation>
    </message>
    <message>
        <source>You are running a development version of QGIS</source>
        <translation>Está utilizando una versión de desarrollo de QGIS</translation>
    </message>
    <message>
        <source>You are running the current version of QGIS</source>
        <translation>Está utilizando la versión actualizada de QGIS</translation>
    </message>
    <message>
        <source>Would you like more information?</source>
        <translation>¿Desea más información?</translation>
    </message>
    <message>
        <source>QGIS Version Information</source>
        <translation>Información de la versión de QGIS</translation>
    </message>
    <message>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS - Cambios en el SVN desde la última versión</translation>
    </message>
    <message>
        <source>Unable to get current version information from server</source>
        <translation>No se puede obtener información de la versión actual del servidor</translation>
    </message>
    <message>
        <source>Connection refused - server may be down</source>
        <translation>Conexión rehusada - el servidor puede estar fuera de servicio</translation>
    </message>
    <message>
        <source>QGIS server was not found</source>
        <translation>No se ha encontrado el servidor QGIS</translation>
    </message>
    <message>
        <source>Network error while communicating with server</source>
        <translation>Error de red mientras se comunicaba con el servidor</translation>
    </message>
    <message>
        <source>Unknown network socket error</source>
        <translation>Error de socket de red desconocido</translation>
    </message>
    <message>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>No se puede comunicar con el servidor de versión de QGIS</translation>
    </message>
    <message>
        <source>Layer is not valid</source>
        <translation>La capa no es válida</translation>
    </message>
    <message>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>La capa no es válida y no se puede añadir al mapa</translation>
    </message>
    <message>
        <source>Save?</source>
        <translation>¿Guardar?</translation>
    </message>
    <message>
        <source>Do you want to save the current project?</source>
        <translation>¿Quiere guardar el proyecto actual?</translation>
    </message>
    <message>
        <source>Extents: </source>
        <translation>Extensiones: </translation>
    </message>
    <message>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>Abrir una fuente de datos ráster soportada por GDAL</translation>
    </message>
    <message>
        <source> is not a valid or recognized raster data source</source>
        <translation> no es una fuente de datos ráster válida o reconocida</translation>
    </message>
    <message>
        <source> is not a supported raster data source</source>
        <translation> no es una fuente de datos ráster soportada</translation>
    </message>
    <message>
        <source>Unsupported Data Source</source>
        <translation>Fuente de datos no soportada</translation>
    </message>
    <message>
        <source>Enter a name for the new bookmark:</source>
        <translation>Introducir un nombre para el nuevo marcador:</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>No se puede crear el marcador. Puede que falte su base de datos de usuario o esté dañada</translation>
    </message>
    <message>
        <source>Move Feature</source>
        <translation>Mover objeto espacial</translation>
    </message>
    <message>
        <source>Split Features</source>
        <translation>Dividir objetos espaciales</translation>
    </message>
    <message>
        <source>Map Tips</source>
        <translation>Avisos del mapa</translation>
    </message>
    <message>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation>Mostrar información sobre un objeto espacial cuando se desliza el ratón sobre él</translation>
    </message>
    <message>
        <source>Current map scale</source>
        <translation>Escala actual del mapa</translation>
    </message>
    <message>
        <source>Project file is older</source>
        <translation>El archivo del proyecto es más antiguo</translation>
    </message>
    <message>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation>&lt;p&gt;Este archivo de proyecto se guardó con una versión más antigua de QGIS.</translation>
    </message>
    <message>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation> Cuando guarde este proyecto, QGIS lo actualizará a la última versión, dejándolo probablemente inservible para versiones anteriores de QGIS.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation>&lt;P&gt;Incluso aunque los desarrolladores de QGIS tratan de mantener retrocompatibilidad, parte de la información del antiguo proyecto puede perderse.</translation>
    </message>
    <message>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation> Para mejorar la calidad de QGIS, agradecemos si rellena un informe de error en %3.</translation>
    </message>
    <message>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation> Asegúrese de incluir el archivo del antiguo proyecto e indicar la versión de QGIS que usó para descubrir el error.</translation>
    </message>
    <message>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation>&lt;p&gt;Para eliminar este aviso al abrir un proyecto antiguo, desmarque la casilla &apos;%5&apos; en el menú %4.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation>&lt;p&gt;Versión del archivo de proyecto: %1 &lt;br&gt;Versión actual de QGIS: %2</translation>
    </message>
    <message>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation>&lt;tt&gt;Configuración:Opciones:General&lt;/tt&gt;</translation>
    </message>
    <message>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Avisarme al abrir archivos de proyecto guardados con una versión anterior de QGIS</translation>
    </message>
    <message>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation>C</translation>
    </message>
    <message>
        <source>Toggle fullscreen mode</source>
        <translation>Cambiar el modo de pantalla completa</translation>
    </message>
    <message>
        <source>Resource Location Error</source>
        <translation>Error en la localización de recursos</translation>
    </message>
    <message>
        <source>Error reading icon resources from: 
 %1
 Quitting...</source>
        <translation>Error al leer recursos de iconos de: 
 %1
 Saliendo...</translation>
    </message>
    <message>
        <source>Overview</source>
        <translation>Vista general</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Leyenda</translation>
    </message>
    <message>
        <source>You are using QGIS version %1 built against code revision %2.</source>
        <translation>Está usando la versión %1 de QGIS compilada contra la revisión %2 del código.</translation>
    </message>
    <message>
        <source> This copy of QGIS has been built with PostgreSQL support.</source>
        <translation> Esta copia de QGIS se ha compilado con soporte para PostgreSQL.</translation>
    </message>
    <message>
        <source> This copy of QGIS has been built without PostgreSQL support.</source>
        <translation> Esta copia de QGIS se ha compilado sin soporte para PostgreSQL.</translation>
    </message>
    <message>
        <source>
This binary was compiled against Qt %1,and is currently running against Qt %2</source>
        <translation>
Este binario se compiló contra Qt %1 y se está ejecutando contra Qt %2</translation>
    </message>
    <message>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>Barra de progreso que muestra el estado de la renderización de capas y otras operaciones prolongadas</translation>
    </message>
    <message>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation>Muestra las coordenadas del mapa en la posición actual del cursor. Se actualiza continuamente a medida que se mueve el ratón.</translation>
    </message>
    <message>
        <source>Stop map rendering</source>
        <translation>Detener la renderización del mapa</translation>
    </message>
    <message>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation>Vista del mapa. Aquí es donde se visualizan las capas ráster y vectoriales cuando se añaden al mapa</translation>
    </message>
    <message>
        <source>Multiple Instances of QgisApp</source>
        <translation>Múltiples instancias de QgisApp</translation>
    </message>
    <message>
        <source>Multiple instances of Quantum GIS application object detected.
Please contact the developers.
</source>
        <translation>Se detectaron múltiples instancias de objetos de la aplicación Quantum GIS.
Por favor, contacte con los desarrolladores.</translation>
    </message>
    <message>
        <source>Shift+Ctrl+S</source>
        <comment>Save Project under a new name</comment>
        <translation>Mayúsculas+Ctrl+S</translation>
    </message>
    <message>
        <source>&amp;Print Composer</source>
        <translation>Diseñador de im&amp;presión</translation>
    </message>
    <message>
        <source>Ctrl+P</source>
        <comment>Print Composer</comment>
        <translation>Ctrl+P</translation>
    </message>
    <message>
        <source>Print Composer</source>
        <translation>Diseñador de impresión</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Deshacer</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation>Ctrl+Z</translation>
    </message>
    <message>
        <source>Undo the last operation</source>
        <translation>Deshacer la última operación</translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>C&amp;ortar</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <source>Cut the current selection&apos;s contents to the clipboard</source>
        <translation>Cortar el contenido de la selección actual y guardar en el portapapeles</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;Copiar</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>Copy the current selection&apos;s contents to the clipboard</source>
        <translation>Copiar el contenido de la selección actual en el portapapeles</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>&amp;Pegar</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Paste the clipboard&apos;s contents into the current selection</source>
        <translation>Pegar el contenido del portapapeles en la selección actual</translation>
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
        <translation>Zum a la selección</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <comment>Zoom to Selection</comment>
        <translation>Ctrl+J</translation>
    </message>
    <message>
        <source>Zoom Actual Size</source>
        <translation>Zum al tamaño real</translation>
    </message>
    <message>
        <source>Zoom to Actual Size</source>
        <translation>Zum al tamaño real</translation>
    </message>
    <message>
        <source>Add Vector Layer...</source>
        <translation>Añadir capa vectorial...</translation>
    </message>
    <message>
        <source>Add Raster Layer...</source>
        <translation>Añadir capa ráster...</translation>
    </message>
    <message>
        <source>Add PostGIS Layer...</source>
        <translation>Añadir capa PostGIS...</translation>
    </message>
    <message>
        <source>W</source>
        <comment>Add a Web Mapping Server Layer</comment>
        <translation>W</translation>
    </message>
    <message>
        <source>Add a Web Mapping Server Layer</source>
        <translation>Añadir una capa de Servidor de Mapas Web (WMS)</translation>
    </message>
    <message>
        <source>Open Attribute Table</source>
        <translation>Abrir tabla de atributos</translation>
    </message>
    <message>
        <source>Save as Shapefile...</source>
        <translation>Guardar como archivo shape...</translation>
    </message>
    <message>
        <source>Save the current layer as a shapefile</source>
        <translation>Guardar la capa actual como un archivo shape</translation>
    </message>
    <message>
        <source>Save Selection as Shapefile...</source>
        <translation>Guardar la selección como archivo shape...</translation>
    </message>
    <message>
        <source>Save the selection as a shapefile</source>
        <translation>Guardar la selección como un archivo shape</translation>
    </message>
    <message>
        <source>Properties...</source>
        <translation>Propiedades...</translation>
    </message>
    <message>
        <source>Set properties of the current layer</source>
        <translation>Establecer las propiedades de la capa actual</translation>
    </message>
    <message>
        <source>Add to Overview</source>
        <translation>Añadir a la vista general</translation>
    </message>
    <message>
        <source>Add All to Overview</source>
        <translation>Añadir todo a la vista general</translation>
    </message>
    <message>
        <source>Manage Plugins...</source>
        <translation>Administrar complementos...</translation>
    </message>
    <message>
        <source>Toggle Full Screen Mode</source>
        <translation>Conmutar el modo de pantalla completa</translation>
    </message>
    <message>
        <source>Custom CRS...</source>
        <translation>SRC personalizado...</translation>
    </message>
    <message>
        <source>Manage custom coordinate reference systems</source>
        <translation>Administrar sistemas de referencia de coordenadas personalizados</translation>
    </message>
    <message>
        <source>Minimize</source>
        <translation>Minimizar</translation>
    </message>
    <message>
        <source>Minimizes the active window to the dock</source>
        <translation>Minimiza la ventana activa</translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation>Zum</translation>
    </message>
    <message>
        <source>Toggles between a predefined size and the window size set by the user</source>
        <translation>Alterna entre un tamaño predefinido y el tamaño de ventana establecido por el usuario</translation>
    </message>
    <message>
        <source>Bring All to Front</source>
        <translation>Traer todo al frente</translation>
    </message>
    <message>
        <source>Bring forward all open windows</source>
        <translation>Traer hacia delante todas las ventanas abiertas</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Edición</translation>
    </message>
    <message>
        <source>Panels</source>
        <translation>Paneles</translation>
    </message>
    <message>
        <source>Toolbars</source>
        <translation>Barras de herramientas</translation>
    </message>
    <message>
        <source>&amp;Window</source>
        <translation>&amp;Ventana</translation>
    </message>
    <message>
        <source>Toggle extents and mouse position display</source>
        <translation>Conmuta la visualización de la extensión y la posición del ratón</translation>
    </message>
    <message>
        <source>This icon shows whether on the fly coordinate reference system transformation is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation>Este icono muestra si está activada o no la transformación al vuelo del sistema de referencia de coordenadas. Pulse el icono para mostrar el diálogo de propiedades del proyecto para cambiar este comportamiento.</translation>
    </message>
    <message>
        <source>CRS status - Click to open coordinate reference system dialog</source>
        <translation>Estado del SRC. Pulse para abrir el diálogo de sistema de referencia de coordenadas</translation>
    </message>
    <message>
        <source>This release candidate includes over 60 bug fixes and enchancements over the QGIS 0.10.0 release. In addition we have added the following new features:</source>
        <translation>Este candidato de lanzamiento incluye más de 60 correcciones de errores y mejoras en relación a la versión 0.10.0. Además hemos añadido las siguientes funciones nuevas:</translation>
    </message>
    <message>
        <source>Revision of all dialogs for user interface consistancy</source>
        <translation>Revisión de todos los diálogos para consistencia de la interfaz de usuario</translation>
    </message>
    <message>
        <source>Improvements to unique value renderer vector dialog</source>
        <translation type="unfinished">Mejoras en el diálogo </translation>
    </message>
    <message>
        <source>Symbol previews when defining vector classes</source>
        <translation>Previsualización de símbolos cuando se definen clases vectoriales</translation>
    </message>
    <message>
        <source>Separation of python support into its own library</source>
        <translation>Separación de la implementación de python en su propia biblioteca</translation>
    </message>
    <message>
        <source>List view and filter for GRASS toolbox to find tools more quickly</source>
        <translation>Vista de lista y filtrado para la caja de herramientas de GRASS para encontrar las herramientas más rápidamente</translation>
    </message>
    <message>
        <source>List view and filter for Plugin Manager to find plugins more easily</source>
        <translation>Vista de lista y filtrado para el administrador de complementos para encontrar los complementos más fácilmente</translation>
    </message>
    <message>
        <source>Updated Spatial Reference System definitions</source>
        <translation>Actualización de las definiciones del Sistema de Referencia Espacial</translation>
    </message>
    <message>
        <source>QML Style support for rasters and database layers</source>
        <translation>Implementación de estilo QML para capas ráster y de base de datos</translation>
    </message>
    <message>
        <source>Choose a file name to save the QGIS project file as</source>
        <translation>Seleccione un nombre de archivo para guardar el archivo de proyecto de QGIS</translation>
    </message>
    <message>
        <source>Choose a file name to save the map image as</source>
        <translation>Seleccione un nombre de archivo para guardar la imagen del mapa</translation>
    </message>
    <message>
        <source>Start editing failed</source>
        <translation>Ha fallado el comienzo de la edición</translation>
    </message>
    <message>
        <source>Provider cannot be opened for editing</source>
        <translation>El proveedor no se puede abrir para editar</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation>Detener edición</translation>
    </message>
    <message>
        <source>Do you want to save the changes to layer %1?</source>
        <translation>¿Quiere guardar los cambios en la capa %1?</translation>
    </message>
    <message>
        <source>Could not commit changes to layer %1

Errors:  %2
</source>
        <translation>No se pudieron transferir los cambios en la capa %1

Errores: %2</translation>
    </message>
    <message>
        <source>Problems during roll back</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Python Console</source>
        <translation>Consola de Python</translation>
    </message>
    <message>
        <source>There was an error loading a plugin.The following diagnostic information may help the QGIS developers resolve the issue:
%1.</source>
        <translation type="obsolete">Hubo un problema al cargar un complemento. La siguiente información de diagnóstico puede ayudar a los desarrolladores de QGIS a solucionar el asunto:
%1.</translation>
    </message>
    <message>
        <source>Map coordinates for the current view extents</source>
        <translation>Coordenadas del mapa para la extensión actual de la vista</translation>
    </message>
    <message>
        <source>Maptips require an active layer</source>
        <translation>Los avisos del mapa requieren una capa activa</translation>
    </message>
    <message>
        <source>Ctrl+M</source>
        <comment>
Minimize Window</comment>
        <translation type="obsolete">Ctrl+R</translation>
    </message>
    <message>
        <source>Ctrl+M</source>
        <comment>Minimize Window</comment>
        <translation type="unfinished">Ctrl+R</translation>
    </message>
    <message>
        <source></source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <source>About Quantum GIS</source>
        <translation>Acerca de Quantum GIS</translation>
    </message>
    <message>
        <source>About</source>
        <translation>Acerca de</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Versión</translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>Página web de QGIS</translation>
    </message>
    <message>
        <source>What&apos;s New</source>
        <translation>Novedades</translation>
    </message>
    <message>
        <source>Developers</source>
        <translation>Desarrolladores</translation>
    </message>
    <message>
        <source>Providers</source>
        <translation>Proveedores</translation>
    </message>
    <message>
        <source>Sponsors</source>
        <translation>Patrocinadores</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>Aceptar</translation>
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
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation>Quantum GIS se distribuye bajo la Licencia Pública General GNU</translation>
    </message>
    <message>
        <source>http://www.gnu.org/licenses</source>
        <translation>http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <source>Join our user mailing list</source>
        <translation>Únase a nuestra lista de correo de usuarios</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Nombre</translation>
    </message>
</context>
<context>
    <name>QgsAddAttrDialogBase</name>
    <message>
        <source>Add Attribute</source>
        <translation>Añadir atributo</translation>
    </message>
    <message>
        <source>Type:</source>
        <translation>Tipo:</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Nombre:</translation>
    </message>
</context>
<context>
    <name>QgsApplication</name>
    <message>
        <source>Exception</source>
        <translation>Excepción</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation>Seleccionar una acción</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <source>Remove the selected action</source>
        <translation>Eliminar la acción seleccionada</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
    <message>
        <source>Move the selected action down</source>
        <translation>Mover la acción seleccionada hacia abajo</translation>
    </message>
    <message>
        <source>Move down</source>
        <translation>Bajar</translation>
    </message>
    <message>
        <source>Move the selected action up</source>
        <translation>Mover la acción seleccionada hacia arriba</translation>
    </message>
    <message>
        <source>Move up</source>
        <translation>Subir</translation>
    </message>
    <message>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>Esta lista contiene todas las acciones que han sido definidas para la capa actual. Añada acciones introduciendo los detalles en los controles inferiores y presionando el botón Insertar acción. Las acciones se pueden editar pulsando dos veces sobre ellas.</translation>
    </message>
    <message>
        <source>The valid attribute names for this layer</source>
        <translation>Los nombres de atributo válidos para esta capa</translation>
    </message>
    <message>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation>Inserta el campo seleccionado en la acción, precedido con un %</translation>
    </message>
    <message>
        <source>Insert field</source>
        <translation>Insertar campo</translation>
    </message>
    <message>
        <source>Update the selected action</source>
        <translation>Actualizar la acción seleccionada</translation>
    </message>
    <message>
        <source>Update action</source>
        <translation>Actualizar acción</translation>
    </message>
    <message>
        <source>Inserts the action into the list above</source>
        <translation>Inserta la acción en la lista de arriba</translation>
    </message>
    <message>
        <source>Insert action</source>
        <translation>Insertar acción</translation>
    </message>
    <message>
        <source>Captures any output from the action</source>
        <translation>Captura cualquier salida de la acción</translation>
    </message>
    <message>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation>Captura la salida estándar o el error generado por la acción y la muestra en un cuadro de diálogo</translation>
    </message>
    <message>
        <source>Capture output</source>
        <translation>Capturar salida</translation>
    </message>
    <message>
        <source>Enter the action command here</source>
        <translation>Introduzca aquí el comando de acción</translation>
    </message>
    <message>
        <source>Enter the action name here</source>
        <translation>Introduzca el nombre de la acción aquí</translation>
    </message>
    <message>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>Introduzca el nombre de la acción aquí. El nombre debe ser único (QGIS lo hará único si es necesario).</translation>
    </message>
    <message>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation>Introduzca la acción aquí. Ésta puede ser cualquier programa, script o comando que esté disponible en su sistema. Cuando se invoca una acción cualquier conjunto de caracteres que empiece por % y luego tenga el nombre de un campo se reemplazará con el valor de ese campo. Los caracteres especiales %% se reemplazarán por el valor del campo que se seleccionó. Las comillas dobles agrupan texto en argumentos simples para el programa, script o comando. Las comillas dobles se ignorarán si se preceden por una barra invertida (\)</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>Action</source>
        <translation>Acción</translation>
    </message>
    <message>
        <source>Capture</source>
        <translation>Capturar</translation>
    </message>
    <message>
        <source>Attribute Actions</source>
        <translation>Acciones de atributos</translation>
    </message>
    <message>
        <source>Action properties</source>
        <translation>Propiedades de acciones</translation>
    </message>
    <message>
        <source>Browse for action</source>
        <translation>Explorar acciones</translation>
    </message>
    <message>
        <source>Click to browse for an action</source>
        <translation>Pulse para explorar en busca de una acción</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Clicking the button will let you select an application to use as the action</source>
        <translation>Pulsar el botón le permitirá seleccionar una aplicación para usar como la acción</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialog</name>
    <message>
        <source> (int)</source>
        <translation> (int)</translation>
    </message>
    <message>
        <source> (dbl)</source>
        <translation> (dbl)</translation>
    </message>
    <message>
        <source> (txt)</source>
        <translation> (txt)</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Select a file</source>
        <translation>Seleccionar un archivo</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <source>Enter Attribute Values</source>
        <translation>Introducir valores de los atributos</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <source>Run action</source>
        <translation>Ejecutar acción</translation>
    </message>
    <message>
        <source>Updating selection...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Abort</source>
        <translation type="unfinished">Abortar</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <source>Attribute Table</source>
        <translation>Tabla de atributos</translation>
    </message>
    <message>
        <source>in</source>
        <translation>en</translation>
    </message>
    <message>
        <source>Search</source>
        <translation>Búsqueda</translation>
    </message>
    <message>
        <source>Adva&amp;nced...</source>
        <translation>Ava&amp;nzado...</translation>
    </message>
    <message>
        <source>Alt+N</source>
        <translation>Alt+N</translation>
    </message>
    <message>
        <source>Remove selection</source>
        <translation>Eliminar selección</translation>
    </message>
    <message>
        <source>Move selected to top</source>
        <translation>Mover la selección arriba del todo</translation>
    </message>
    <message>
        <source>Ctrl+T</source>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <source>Invert selection</source>
        <translation>Invertir selección</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Copiar las filas seleccionadas al portapapeles (Ctrl+C)</translation>
    </message>
    <message>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Copia las filas seleccionadas al portapapeles</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows</source>
        <translation>Zum a las filas seleccionadas </translation>
    </message>
    <message>
        <source>Search for</source>
        <translation>Buscar</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows (Ctrl-J)</source>
        <translation>Acercar el mapa a las filas seleccionadas (Ctrl+F)</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <source>Toggle editing mode</source>
        <translation>Conmutar el modo edición</translation>
    </message>
    <message>
        <source>Click to toggle table editing</source>
        <translation>Pulsar para conmutar la edición de la tabla</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <source>select</source>
        <translation>seleccionar</translation>
    </message>
    <message>
        <source>select and bring to top</source>
        <translation>seleccionar y llevar arriba</translation>
    </message>
    <message>
        <source>show only matching</source>
        <translation>mostrar solo coincidentes</translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation>Buscar error de análisis de la cadena</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation>Resultados de la búsqueda</translation>
    </message>
    <message>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>Ha suministrado una cadena de búsqueda vacía.</translation>
    </message>
    <message>
        <source>Error during search</source>
        <translation>Error durante la búsqueda</translation>
    </message>
    <message>
        <source>No matching features found.</source>
        <translation>No se han encontrado coincidencias.</translation>
    </message>
    <message>
        <source>Attribute table - </source>
        <translation>Tabla de atributos - </translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Archivo</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <source>Ctrl+W</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Deshacer</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation>Ctrl+Z</translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>Cor&amp;tar</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;Copiar</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>&amp;Pegar</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Capa</translation>
    </message>
    <message>
        <source>Zoom to Selection</source>
        <translation>Zum a la selección</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <translation type="unfinished">Ctrl+J</translation>
    </message>
    <message>
        <source>Toggle Editing</source>
        <translation>Conmutar edición</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabla</translation>
    </message>
    <message>
        <source>Move to Top</source>
        <translation>Mover arriba</translation>
    </message>
    <message>
        <source>Invert</source>
        <translation>Invertir</translation>
    </message>
    <message>
        <source>bad_alloc exception</source>
        <translation>excepción bad_alloc</translation>
    </message>
    <message>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation>El relleno de la tabla de atributos se ha detenido porque no quedaba más memoria virtual</translation>
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
        <translation>¿Borrar realmente?</translation>
    </message>
    <message>
        <source>Are you sure you want to delete the </source>
        <translation>¿Está seguro de que quiere borrar el marcador </translation>
    </message>
    <message>
        <source> bookmark?</source>
        <translation> marcador?</translation>
    </message>
    <message>
        <source>Error deleting bookmark</source>
        <translation>Error al borrar marcador</translation>
    </message>
    <message>
        <source>Failed to delete the </source>
        <translation>Ha fallado el borrado del marcador </translation>
    </message>
    <message>
        <source> bookmark from the database. The database said:
</source>
        <translation> marcador de la base de datos. La base de datos ha dicho:</translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation>&amp;Borrar</translation>
    </message>
    <message>
        <source>&amp;Zoom to</source>
        <translation>&amp;Zum a</translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <source>Geospatial Bookmarks</source>
        <translation>Marcadores geoespaciales</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>Project</source>
        <translation>Proyecto</translation>
    </message>
    <message>
        <source>Extent</source>
        <translation>Extensión</translation>
    </message>
    <message>
        <source>Id</source>
        <translation>Id</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <source>QGIS - print composer</source>
        <translation>QGIS - diseñador de mapas</translation>
    </message>
    <message>
        <source>Map 1</source>
        <translation>Mapa 1</translation>
    </message>
    <message>
        <source>Big image</source>
        <translation>Imagen grande</translation>
    </message>
    <message>
        <source>To create image </source>
        <translation>Crear imagen </translation>
    </message>
    <message>
        <source> requires circa </source>
        <translation> requiere cerca de </translation>
    </message>
    <message>
        <source> MB of memory</source>
        <translation> MB de memoria</translation>
    </message>
    <message>
        <source>format</source>
        <translation>formato</translation>
    </message>
    <message>
        <source>SVG warning</source>
        <translation>Aviso de SVG</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>No volver a mostrar este mensaje</translation>
    </message>
    <message>
        <source>SVG Format</source>
        <translation>Formato SVG</translation>
    </message>
    <message>
        <source>Move Content</source>
        <translation>Mover contenido</translation>
    </message>
    <message>
        <source>Move item content</source>
        <translation>Mover contenido del elemento</translation>
    </message>
    <message>
        <source>&amp;Group</source>
        <translation>A&amp;grupar</translation>
    </message>
    <message>
        <source>Group items</source>
        <translation>Agrupar elementos</translation>
    </message>
    <message>
        <source>&amp;Ungroup</source>
        <translation>&amp;Desagrupar</translation>
    </message>
    <message>
        <source>Ungroup items</source>
        <translation>Desagrupar elementos</translation>
    </message>
    <message>
        <source>Raise</source>
        <translation>Subir</translation>
    </message>
    <message>
        <source>Raise selected items</source>
        <translation>Subir los elementos seleccionados</translation>
    </message>
    <message>
        <source>Lower</source>
        <translation>Bajar</translation>
    </message>
    <message>
        <source>Lower selected items</source>
        <translation>Bajar los elementos seleccionados</translation>
    </message>
    <message>
        <source>Bring to Front</source>
        <translation>Traer al frente</translation>
    </message>
    <message>
        <source>Move selected items to top</source>
        <translation>Mover los elementos seleccionados arriba</translation>
    </message>
    <message>
        <source>Send to Back</source>
        <translation>Enviar al fondo</translation>
    </message>
    <message>
        <source>Move selected items to bottom</source>
        <translation>Mover los elementos seleccionados al fondo</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Archivo</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <source>Ctrl+W</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Deshacer</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation>Ctrl+Z</translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>Cor&amp;tar</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;Copiar</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>&amp;Pegar</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <source>View</source>
        <translation>Ver</translation>
    </message>
    <message>
        <source>Layout</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose a file name to save the map image as</source>
        <translation>Seleccione un nombre de archivo para guardar la imagen del mapa</translation>
    </message>
    <message>
        <source>Choose a file name to save the map as</source>
        <translation>Seleccione un nombre de archivo para guardar el mapa</translation>
    </message>
    <message>
        <source>Project contains WMS layers</source>
        <translation>El proyecto contiene capas WMS</translation>
    </message>
    <message>
        <source>Some WMS servers (e.g. UMN mapserver) have a limit for the WIDTH and HEIGHT parameter. Printing layers from such servers may exceed this limit. If this is the case, the WMS layer will not be printed</source>
        <translation>Algunos servidores WMS (ej.: UMN mapserver) tiene un límite para los parámetros ANCHURA y ALTURA. Imprimir capas de esos servidores puede exceder este límite. Si es el caso, la capa WMS no se imprimirá.</translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <source>MainWindow</source>
        <translation>Ventana principal</translation>
    </message>
    <message>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <source>Composition</source>
        <translation>Diseño</translation>
    </message>
    <message>
        <source>Item</source>
        <translation>Elemento</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <source>&amp;Print...</source>
        <translation>Im&amp;primir...</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Acercar zum</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Alejar zum</translation>
    </message>
    <message>
        <source>Add new map</source>
        <translation>Añadir mapa nuevo</translation>
    </message>
    <message>
        <source>Add new label</source>
        <translation>Añadir etiqueta nueva</translation>
    </message>
    <message>
        <source>Add new vect legend</source>
        <translation>Añadir nueva leyenda vectorizada</translation>
    </message>
    <message>
        <source>Select/Move item</source>
        <translation>Seleccionar/Mover elemento</translation>
    </message>
    <message>
        <source>Add new scalebar</source>
        <translation>Añadir nueva barra de escala</translation>
    </message>
    <message>
        <source>Refresh view</source>
        <translation>Actualizar vista</translation>
    </message>
    <message>
        <source>Add Image</source>
        <translation>Añadir imagen</translation>
    </message>
    <message>
        <source>Zoom Full</source>
        <translation>Zum general</translation>
    </message>
    <message>
        <source>Add Map</source>
        <translation>Añadir mapa</translation>
    </message>
    <message>
        <source>Add Label</source>
        <translation>Añadir etiqueta</translation>
    </message>
    <message>
        <source>Add Vector Legend</source>
        <translation>Añadir leyenda vectorial</translation>
    </message>
    <message>
        <source>Move Item</source>
        <translation>Mover elemento</translation>
    </message>
    <message>
        <source>Export as Image...</source>
        <translation>Exportar como imagen...</translation>
    </message>
    <message>
        <source>Export as SVG...</source>
        <translation>Exportar como SVG...</translation>
    </message>
    <message>
        <source>Add Scalebar</source>
        <translation>Añadir barra de escala</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
</context>
<context>
    <name>QgsComposerItemWidgetBase</name>
    <message>
        <source>Form</source>
        <translation>Formulario</translation>
    </message>
    <message>
        <source>Composer item properties</source>
        <translation>Propiedades de elemento del diseñador</translation>
    </message>
    <message>
        <source>Color:</source>
        <translation>Color:</translation>
    </message>
    <message>
        <source>Frame...</source>
        <translation>Marco...</translation>
    </message>
    <message>
        <source>Background...</source>
        <translation>Fondo...</translation>
    </message>
    <message>
        <source>Opacity:</source>
        <translation>Opacidad:</translation>
    </message>
    <message>
        <source>Outline width: </source>
        <translation>Anchura de línea exterior: </translation>
    </message>
    <message>
        <source>Frame</source>
        <translation>Marco</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelWidgetBase</name>
    <message>
        <source>Label Options</source>
        <translation>Opciones de etiquetas</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Tipo de letra</translation>
    </message>
    <message>
        <source>Margin (mm):</source>
        <translation>Margen (mm):</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendItemDialogBase</name>
    <message>
        <source>Legend item properties</source>
        <translation>Propiedades del elemento de la leyenda</translation>
    </message>
    <message>
        <source>Item text:</source>
        <translation>Texto del elemento:</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendWidgetBase</name>
    <message>
        <source>Barscale Options</source>
        <translation>Opciones de la barra de escala</translation>
    </message>
    <message>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <source>Title:</source>
        <translation>Título:</translation>
    </message>
    <message>
        <source>Font:</source>
        <translation>Tipo de letra:</translation>
    </message>
    <message>
        <source>Title...</source>
        <translation>Título...</translation>
    </message>
    <message>
        <source>Layer...</source>
        <translation>Capa...</translation>
    </message>
    <message>
        <source>Item...</source>
        <translation>Elemento...</translation>
    </message>
    <message>
        <source>Symbol width: </source>
        <translation>Anchura de símbolo: </translation>
    </message>
    <message>
        <source>Symbol height:</source>
        <translation>Altura de símbolo:</translation>
    </message>
    <message>
        <source>Layer space: </source>
        <translation>Espacio de capa: </translation>
    </message>
    <message>
        <source>Symbol space:</source>
        <translation>Espacio de símbolo:</translation>
    </message>
    <message>
        <source>Icon label space:</source>
        <translation>Espacio de etiqueta de icono:</translation>
    </message>
    <message>
        <source>Box space:</source>
        <translation>Espacio de recuadro:</translation>
    </message>
    <message>
        <source>Legend items</source>
        <translation>Elementos de la leyenda</translation>
    </message>
    <message>
        <source>down</source>
        <translation>abajo</translation>
    </message>
    <message>
        <source>up</source>
        <translation>arriba</translation>
    </message>
    <message>
        <source>remove</source>
        <translation>eliminar</translation>
    </message>
    <message>
        <source>edit...</source>
        <translation>editar...</translation>
    </message>
    <message>
        <source>update</source>
        <translation>actualizar</translation>
    </message>
    <message>
        <source>update all</source>
        <translation>actualizar todo</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <source>Map will be printed here</source>
        <translation>El mapa se imprimirá aquí</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidget</name>
    <message>
        <source>Cache</source>
        <translation>Caché</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation>Rectángulo</translation>
    </message>
    <message>
        <source>Render</source>
        <translation>Representar</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidgetBase</name>
    <message>
        <source>Map options</source>
        <translation>Opciones de mapa</translation>
    </message>
    <message>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation>&lt;b&gt;Mapa&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Anchura</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <source>Scale:</source>
        <translation>Escala:</translation>
    </message>
    <message>
        <source>1:</source>
        <translation>1:</translation>
    </message>
    <message>
        <source>Map extent</source>
        <translation>Extensión del mapa</translation>
    </message>
    <message>
        <source>X min:</source>
        <translation>X mín:</translation>
    </message>
    <message>
        <source>Y min:</source>
        <translation>Y mín:</translation>
    </message>
    <message>
        <source>X max:</source>
        <translation>X máx:</translation>
    </message>
    <message>
        <source>Y max:</source>
        <translation>Y máx:</translation>
    </message>
    <message>
        <source>set to map canvas extent</source>
        <translation>establecer a la extensión de la vista del mapa</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Vista preliminar</translation>
    </message>
    <message>
        <source>Update preview</source>
        <translation>Actualizar vista preliminar</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidget</name>
    <message>
        <source>Select svg or image file</source>
        <translation>Seleccionar svg o archivo de imagen</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidgetBase</name>
    <message>
        <source>Picture Options</source>
        <translation>Opciones de dibujo</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
    <message>
        <source>Width:</source>
        <translation>Anchura:</translation>
    </message>
    <message>
        <source>Height:</source>
        <translation>Altura:</translation>
    </message>
    <message>
        <source>Rotation:</source>
        <translation>Rotación:</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBar</name>
    <message>
        <source>Single Box</source>
        <translation>Recuadro simple</translation>
    </message>
    <message>
        <source>Double Box</source>
        <translation>Recuadro doble</translation>
    </message>
    <message>
        <source>Line Ticks Middle</source>
        <translation>Línea con marcas en el centro</translation>
    </message>
    <message>
        <source>Line Ticks Down</source>
        <translation>Línea con marcas en el abajo</translation>
    </message>
    <message>
        <source>Line Ticks Up</source>
        <translation>Línea con marcas en el arriba</translation>
    </message>
    <message>
        <source>Numeric</source>
        <translation>Numérico</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidget</name>
    <message>
        <source>Single Box</source>
        <translation>Recuadro simple</translation>
    </message>
    <message>
        <source>Double Box</source>
        <translation>Recuadro doble</translation>
    </message>
    <message>
        <source>Line Ticks Middle</source>
        <translation>Línea con marcas en el centro</translation>
    </message>
    <message>
        <source>Line Ticks Down</source>
        <translation>Línea con marcas en el abajo</translation>
    </message>
    <message>
        <source>Line Ticks Up</source>
        <translation>Línea con marcas en el arriba</translation>
    </message>
    <message>
        <source>Numeric</source>
        <translation>Numérico</translation>
    </message>
    <message>
        <source>Map </source>
        <translation>Mapa </translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidgetBase</name>
    <message>
        <source>Barscale Options</source>
        <translation>Opciones de la barra de escala</translation>
    </message>
    <message>
        <source>Segment size (map units):</source>
        <translation>Tamaño de segmento (unidades de mapa):</translation>
    </message>
    <message>
        <source>Map units per bar unit:</source>
        <translation>Unidades de mapa por unidad de barra:</translation>
    </message>
    <message>
        <source>Number of segments:</source>
        <translation>Número de segmentos:</translation>
    </message>
    <message>
        <source>Segments left:</source>
        <translation>Segmentos a la izquierda:</translation>
    </message>
    <message>
        <source>Style:</source>
        <translation>Estilo:</translation>
    </message>
    <message>
        <source>Map:</source>
        <translation>Mapa:</translation>
    </message>
    <message>
        <source>Height (mm):</source>
        <translation>Altura (mm):</translation>
    </message>
    <message>
        <source>Line width:</source>
        <translation>Anchura de línea:</translation>
    </message>
    <message>
        <source>Label space:</source>
        <translation>Espacio de etiqueta:</translation>
    </message>
    <message>
        <source>Box space:</source>
        <translation>Espacio de recuadro:</translation>
    </message>
    <message>
        <source>Unit label:</source>
        <translation>Etiqueta de unidades:</translation>
    </message>
    <message>
        <source>Font...</source>
        <translation>Tipo de letra...</translation>
    </message>
    <message>
        <source>Color...</source>
        <translation>Color...</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <source>Vector Legend Options</source>
        <translation>Opciones de leyenda vectorizada</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Cajetín</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Previsualización</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation>Capas</translation>
    </message>
    <message>
        <source>Group</source>
        <translation>Grupo</translation>
    </message>
    <message>
        <source>ID</source>
        <translation>ID</translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <source>Composition</source>
        <translation>Diseño</translation>
    </message>
    <message>
        <source>Paper</source>
        <translation>Papel</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Tamaño</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Unidades</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Anchura</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Orientación</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidget</name>
    <message>
        <source>Landscape</source>
        <translation>Horizontal</translation>
    </message>
    <message>
        <source>Portrait</source>
        <translation>Vertical</translation>
    </message>
    <message>
        <source>Custom</source>
        <translation>Personalizado</translation>
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
        <translation>Carta (8.5x11 pulgadas)</translation>
    </message>
    <message>
        <source>Legal (8.5x14 inches)</source>
        <translation>Legal (8.5x14 pulgadas)</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidgetBase</name>
    <message>
        <source>Composition</source>
        <translation>Diseño</translation>
    </message>
    <message>
        <source>Paper</source>
        <translation>Papel</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Orientación</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Anchura</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Unidades</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Tamaño</translation>
    </message>
    <message>
        <source>Print quality (dpi)</source>
        <translation>Calidad de impresión (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <source>Continuous color</source>
        <translation>Color graduado</translation>
    </message>
    <message>
        <source>Draw polygon outline</source>
        <translation>Dibujar contorno del polígono</translation>
    </message>
    <message>
        <source>Classification Field:</source>
        <translation>Campo de clasificación:</translation>
    </message>
    <message>
        <source>Minimum Value:</source>
        <translation>Valor mínimo:</translation>
    </message>
    <message>
        <source>Outline Width:</source>
        <translation>Anchura del contorno:</translation>
    </message>
    <message>
        <source>Maximum Value:</source>
        <translation>Valor máximo:</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <source>Failed</source>
        <translation>Ha fallado</translation>
    </message>
    <message>
        <source>transform of</source>
        <translation>la transformación de</translation>
    </message>
    <message>
        <source>with error: </source>
        <translation>con el error: </translation>
    </message>
    <message>
        <source>The source spatial reference system (CRS) is not valid. </source>
        <translation>El sistema de referencia espacial (SRC) no es válido. </translation>
    </message>
    <message>
        <source>The coordinates can not be reprojected. The CRS is: </source>
        <translation>No se puede reproyectar las coordenadas. El SRC es: </translation>
    </message>
    <message>
        <source>The destination spatial reference system (CRS) is not valid. </source>
        <translation>El sistema de referencia espacial de destino (SRC) no es válido. </translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <source>&amp;Copyright Label</source>
        <translation>Etiqueta de &amp;Copyright</translation>
    </message>
    <message>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>Crea una etiqueta de copyright que se muestra en la vista del mapa.</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Ilustraciones</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <source>Copyright Label Plugin</source>
        <translation>Complemento etiqueta de copyright</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Ubicación</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Orientación</translation>
    </message>
    <message>
        <source>Horizontal</source>
        <translation>Horizontal</translation>
    </message>
    <message>
        <source>Vertical</source>
        <translation>Vertical</translation>
    </message>
    <message>
        <source>Enable Copyright Label</source>
        <translation>Activar etiqueta de copyright</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Color</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Introduzca su etiqueta de copyright debajo. Este complemento admite etiquetas html básicas para dar formato a la etiqueta. Por ejemplo:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Bold text &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Italics &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(note: &amp;amp;copy; proporciona un símbolo de copyright)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
        <translation>¿Borrar definición de proyección?</translation>
    </message>
    <message>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Borrar una definición de proyección no es reversible. ¿Desea continuar?</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation>Abortar</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <source>QGIS Custom Projection</source>
        <translation>Proyección personalizada de QGIS</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, dele un nombre a la proyección antes de guardar.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, rellene los parámetros antes de guardar.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, añada una cláusula proj= antes de guardar.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Esta definición de proyección proj4 no es válida. Por favor, corregir antes de guardar.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Esta definición de proyección proj4 no es válida.</translation>
    </message>
    <message>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>El Norte y el Este deben estar en formato decimal.</translation>
    </message>
    <message>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Error interno (¿la proyección original no es válida?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <source>Define</source>
        <translation>Definición</translation>
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
        <translation>1 de 1</translation>
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
        <translation>Probar</translation>
    </message>
    <message>
        <source>Geographic / WGS84</source>
        <translation>Geográficas / WGS84</translation>
    </message>
    <message>
        <source>Calculate</source>
        <translation>Calcular</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Parámetros</translation>
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
        <translation>Norte</translation>
    </message>
    <message>
        <source>East</source>
        <translation>Este</translation>
    </message>
    <message>
        <source>Custom Coordinate Reference System Definition</source>
        <translation>Definición de sistema de referencia de coordenadas personalizado</translation>
    </message>
    <message>
        <source>You can define your own custom Coordinate Reference System (CRS) here. The definition must conform to the proj4 format for specifying a CRS.</source>
        <translation>Puede definir su propio Sistema de Referencia de Coordenadas (SRC) aquí. La definición debe ajustarse al formato proj4 para especificar un SRC.</translation>
    </message>
    <message>
        <source>Use the text boxes below to test the CRS definition you are creating. Enter a coordinate where both the lat/long and the transformed result are known (for example by reading off a map). Then press the calculate button to see if the CRS definition you are creating is accurate.</source>
        <translation>Use los cuadros de texto para probar la definición del SRC que está creando. Introduzca una coordenada donde tanto la lat/long como el resultado transformado sean conocidos (por ejemplo tomándolos de un mapa). Pulse luego el botón Calcular para ver si le definición del SRC que está creando es correcto.</translation>
    </message>
    <message>
        <source>Destination CRS        </source>
        <translation>SCR de destino         </translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>¿Está seguro de que quiere eliminar la conexión </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> y su configuración?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Confirmar la eliminación</translation>
    </message>
    <message>
        <source>Select Table</source>
        <translation>Seleccionar tabla</translation>
    </message>
    <message>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Debe seleccionar una tabla para poder añadir una capa.</translation>
    </message>
    <message>
        <source>Password for </source>
        <translation>Contraseña para </translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation>Por favor, introduzca su contraseña:</translation>
    </message>
    <message>
        <source>Connection failed</source>
        <translation>Conexión fallida</translation>
    </message>
    <message>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>La conexión a %1 en %2 ha fallado. Puede ser que la base de datos esté inactiva o que su configuración sea incorrecta.%3Compruebe el nombre de usuario y la contraseña y pruebe de nuevo.%4La base de datos ha dicho:%5%6</translation>
    </message>
    <message>
        <source>Wildcard</source>
        <translation>Comodín</translation>
    </message>
    <message>
        <source>RegExp</source>
        <translation>Expresión regular</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Todos</translation>
    </message>
    <message>
        <source>Schema</source>
        <translation>Esquema</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabla</translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation>Columna de geometría</translation>
    </message>
    <message>
        <source>Accessible tables could not be determined</source>
        <translation>No se pudieron determinar las tablas accesibles</translation>
    </message>
    <message>
        <source>Database connection was successful, but the accessible tables could not be determined.

The error message from the database was:
%1
</source>
        <translation>La conexión a la base de datos tuvo éxito, pero no se pudieron determinar las tablas accesibles.

El mensaje de error de la base de datos fue:
%1</translation>
    </message>
    <message>
        <source>No accessible tables found</source>
        <translation>No se encontraron tablas accesibles</translation>
    </message>
    <message>
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
        <source>Add PostGIS Table(s)</source>
        <translation>Añadir tabla(s) PostGIS</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <source>PostgreSQL Connections</source>
        <translation>Conexiones de PostgreSQL</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Conectar</translation>
    </message>
    <message>
        <source>Search:</source>
        <translation>Buscar:</translation>
    </message>
    <message>
        <source>Search mode:</source>
        <translation>Modo de búsqueda:</translation>
    </message>
    <message>
        <source>Search in columns:</source>
        <translation>Buscar en columnas:</translation>
    </message>
    <message>
        <source>Search options...</source>
        <translation>Opciones de búsqueda...</translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <source>Schema</source>
        <translation>Esquema</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabla</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation>Columna de geometría</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <source>Multipoint</source>
        <translation>Múltiples puntos</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Línea</translation>
    </message>
    <message>
        <source>Multiline</source>
        <translation>Múltiples línea</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Polígono</translation>
    </message>
    <message>
        <source>Multipolygon</source>
        <translation>Múltiples polígono</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <source>Delete Attributes</source>
        <translation>Borrar atributos</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <source>&amp;Add Delimited Text Layer</source>
        <translation>&amp;Añadir capa de texto delimitado</translation>
    </message>
    <message>
        <source>Add a delimited text file as a map layer. </source>
        <translation>Añade una capa de texto delimitado como una capa del mapa. </translation>
    </message>
    <message>
        <source>The file must have a header row containing the field names. </source>
        <translation>El archivo debe tener una fila de encabezado con los nombres de los campos. </translation>
    </message>
    <message>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>Los campos X e Y son necesarios y deben contener las coordenadas en unidades decimales.</translation>
    </message>
    <message>
        <source>&amp;Delimited text</source>
        <translation>&amp;Texto delimitado</translation>
    </message>
    <message>
        <source>DelimitedTextLayer</source>
        <translation>CapaDeTextoDelimitado</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <source>No layer name</source>
        <translation>Ningún nombre de capa</translation>
    </message>
    <message>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Por favor, introduzca un nombre para la capa antes de añadir ésta al mapa</translation>
    </message>
    <message>
        <source>No delimiter</source>
        <translation>No hay delimitador</translation>
    </message>
    <message>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Por favor, especifique un delimitador antes de analizar el archivo</translation>
    </message>
    <message>
        <source>Choose a delimited text file to open</source>
        <translation>Seleccione un archivo de texto delimitado para abrir</translation>
    </message>
    <message>
        <source>Parse</source>
        <translation>Analizar</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Descripción</translation>
    </message>
    <message>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation>Seleccione un archivo de texto delimitado que contenga una fila de encabezado y una o más filas de coordenadas X e Y que quiera usar como capa de puntos y este complemento hará el trabajo por usted.</translation>
    </message>
    <message>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation>Use el recuadro Nombre de la capa para especificar el nombre de la nueva capa en la leyenda. Use el recuadro delimitador para especificar qué delimitador se usa en su archivo ej.: espacio, coma, tabulador o una expresión regular en estilo Perl). Después de elegir un delimitador, pulse el botón Analizar y seleccione las columnas que contienen los valores X e Y para la capa.</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <source>Create a Layer from a Delimited Text File</source>
        <translation>Crear una capa a partir de un archivo de texto delimitado</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Coordenada X&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Name of the field containing x values</source>
        <translation>Nombre del campo que contiene los valores de la X</translation>
    </message>
    <message>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nombre del campo que contiene los valores de la X. Seleccione un campo de la lista. La lista se genera al analizar la fila de encabezado del archivo de texto delimitado.</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Coordenada Y&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Name of the field containing y values</source>
        <translation>Nombre del campo que contiene los valores de la Y</translation>
    </message>
    <message>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nombre del campo que contiene los valores de la Y. Seleccione un campo de la lista. La lista se genera al analizar la fila de encabezado del archivo de texto delimitado.</translation>
    </message>
    <message>
        <source>Sample text</source>
        <translation>Texto de muestra</translation>
    </message>
    <message>
        <source>Delimited Text Layer</source>
        <translation>Capa de texto delimitado</translation>
    </message>
    <message>
        <source>Delimited text file</source>
        <translation>Archivo de texto delimitado</translation>
    </message>
    <message>
        <source>Full path to the delimited text file</source>
        <translation>Ruta completa al archivo de texto delimitado</translation>
    </message>
    <message>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Ruta completa al archivo de texto delimitado. Para poder analizar los campos del archivo correctamente, el delimitador debe definirse antes de introducir el nombre del archivo. Utilice el botón Explorar situado a la derecha de este campo para seleccionar el archivo de entrada.</translation>
    </message>
    <message>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Explorar para buscar el archivo de texto delimitado a procesar</translation>
    </message>
    <message>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Use este botón para buscar el archivo de texto. Este botón no se activará hasta que se haya introducido un delimitador en el recuadro &lt;i&gt; Delimitador&lt;/i&gt;. Una vez que se ha seleccionado un archivo, los cuadros combinados de las coordenadas X e Y se rellenarán con los campos del archivo de texto delimitado.</translation>
    </message>
    <message>
        <source>Layer name</source>
        <translation>Nombre de la capa</translation>
    </message>
    <message>
        <source>Name to display in the map legend</source>
        <translation>Nombre para mostrar en la leyenda del mapa</translation>
    </message>
    <message>
        <source>Name displayed in the map legend</source>
        <translation>Nombre mostrado en la leyenda del mapa</translation>
    </message>
    <message>
        <source>Delimiter</source>
        <translation>Delimitador</translation>
    </message>
    <message>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation>Delimitador a usar para dividir campos en el archivo de texto. El delimitador puede ser más de un carácter.</translation>
    </message>
    <message>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Delimitador a usar para dividir campos en el archivo de texto. El delimitador puede tener uno o más caracteres.</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
    <message>
        <source>The delimiter is taken as is</source>
        <translation>El delimitador se toma tal como es</translation>
    </message>
    <message>
        <source>Plain characters</source>
        <translation>Caracteres sencillos</translation>
    </message>
    <message>
        <source>The delimiter is a regular expression</source>
        <translation>El delimitador es una expresión regular</translation>
    </message>
    <message>
        <source>Regular expression</source>
        <translation>Expresión regular</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextProvider</name>
    <message>
        <source>Note: the following lines were not loaded because Qgis was unable to determine values for the x and y coordinates:
</source>
        <translation>Nota: las líneas siguientes no se han cargado porque Qgis no pudo determinar los valores de las coordenadas X e Y:
</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Error</translation>
    </message>
</context>
<context>
    <name>QgsDetailedItemWidgetBase</name>
    <message>
        <source>Form</source>
        <translation>Formulario</translation>
    </message>
    <message>
        <source>Heading Label</source>
        <translation>Etiqueta de cabecera</translation>
    </message>
    <message>
        <source>Detail label</source>
        <translation>Etiqueta de detalle</translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <source>Buffer features</source>
        <translation>Crear buffer de objetos espaciales</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Parámetros</translation>
    </message>
    <message>
        <source>Geometry column:</source>
        <translation>Columna de la geometría:</translation>
    </message>
    <message>
        <source>Add the buffered layer to the map?</source>
        <translation>¿Añadir la capa de buffer al mapa?</translation>
    </message>
    <message>
        <source>Spatial reference ID:</source>
        <translation>ID de la referencia espacial:</translation>
    </message>
    <message>
        <source>Schema:</source>
        <translation>Esquema:</translation>
    </message>
    <message>
        <source>Unique field to use as feature id:</source>
        <translation>Campo único a utilizar como identificador:</translation>
    </message>
    <message>
        <source>Table name for the buffered layer:</source>
        <translation>Nombre de la tabla de la capa del buffer:</translation>
    </message>
    <message>
        <source>Create unique object id</source>
        <translation>Crear id de objetos único</translation>
    </message>
    <message>
        <source>public</source>
        <translation>público</translation>
    </message>
    <message>
        <source>Buffer distance in map units:</source>
        <translation>Distancia de buffer en unidades del mapa:</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Crear buffer de los objetos espaciales de la capa: &lt;/h2&gt;</translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <source>Encoding:</source>
        <translation>Codificación:</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <source>New device %1</source>
        <translation>Nuevo receptor %1</translation>
    </message>
    <message>
        <source>Are you sure?</source>
        <translation>¿Está seguro?</translation>
    </message>
    <message>
        <source>Are you sure that you want to delete this device?</source>
        <translation>¿Está seguro de que quiere borrar este receptor?</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <source>GPS Device Editor</source>
        <translation>Editor de receptores GPS</translation>
    </message>
    <message>
        <source>New device</source>
        <translation>Nuevo receptor</translation>
    </message>
    <message>
        <source>Delete device</source>
        <translation>Borrar receptor</translation>
    </message>
    <message>
        <source>Update device</source>
        <translation>Actualizar receptor</translation>
    </message>
    <message>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Así es como aparecerá el nombre del receptor en la lista</translation>
    </message>
    <message>
        <source>Commands</source>
        <translation>Comandos</translation>
    </message>
    <message>
        <source>Track download:</source>
        <translation>Descargar track:</translation>
    </message>
    <message>
        <source>Route upload:</source>
        <translation>Cargar ruta:</translation>
    </message>
    <message>
        <source>Waypoint download:</source>
        <translation>Descargar waypoint:</translation>
    </message>
    <message>
        <source>The command that is used to download routes from the device</source>
        <translation>El comando que se usa para descargar rutas del receptor</translation>
    </message>
    <message>
        <source>Route download:</source>
        <translation>Descargar ruta:</translation>
    </message>
    <message>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>El comando que se usa para cargar waypoints al receptor</translation>
    </message>
    <message>
        <source>Track upload:</source>
        <translation>Cargar track:</translation>
    </message>
    <message>
        <source>The command that is used to download tracks from the device</source>
        <translation>El comando que se usa para descargar tracks del receptor</translation>
    </message>
    <message>
        <source>The command that is used to upload routes to the device</source>
        <translation>El comando que se usa para cargar rutas al receptor</translation>
    </message>
    <message>
        <source>The command that is used to download waypoints from the device</source>
        <translation>El comando que se usa para descargar waypoints del receptor</translation>
    </message>
    <message>
        <source>The command that is used to upload tracks to the device</source>
        <translation>El comando que se usa para cargar tracks al receptor</translation>
    </message>
    <message>
        <source>Waypoint upload:</source>
        <translation>Cargar waypoint:</translation>
    </message>
    <message>
        <source>Device name</source>
        <translation>Nombre del dispositivo</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;En las órdenes para descargar y cargar puede haber palabras especiales que serán reemplazadas por QGIS cuando se utilicen estas órdenes. Estas palabras son:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - ruta de GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%dentro&lt;/span&gt; - el nombre del archivo GPX cuando se está cargando o el puerto cuando se está descargando&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%fuera&lt;/span&gt; - el puerto cuando se está cargando o el nombre del archivo GPX cuando se está descargando&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <source>&amp;Gps Tools</source>
        <translation>Herramientas &amp;GPS</translation>
    </message>
    <message>
        <source>&amp;Create new GPX layer</source>
        <translation>&amp;Crear nueva capa GPX</translation>
    </message>
    <message>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Crea una capa GPX nueva y la muestra en la vista del mapa</translation>
    </message>
    <message>
        <source>&amp;Gps</source>
        <translation>&amp;GPS</translation>
    </message>
    <message>
        <source>Save new GPX file as...</source>
        <translation>Guardar nuevo archivo GPX como...</translation>
    </message>
    <message>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>Archivo GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <source>Could not create file</source>
        <translation>No se pudo crear el archivo</translation>
    </message>
    <message>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>No se puede crear un archivo GPX con este nombre. </translation>
    </message>
    <message>
        <source>Try again with another name or in another </source>
        <translation>Inténtelo de nuevo con otro nombre o en otro </translation>
    </message>
    <message>
        <source>directory.</source>
        <translation>directorio.</translation>
    </message>
    <message>
        <source>GPX Loader</source>
        <translation>Cargador de GPX</translation>
    </message>
    <message>
        <source>Unable to read the selected file.
</source>
        <translation>No se puede leer el archivo seleccionado.
</translation>
    </message>
    <message>
        <source>Please reselect a valid file.</source>
        <translation>Por favor, vuelva a seleccionar un archivo válido.</translation>
    </message>
    <message>
        <source>Could not start process</source>
        <translation>No se pudo iniciar el proceso</translation>
    </message>
    <message>
        <source>Could not start GPSBabel!</source>
        <translation>No se pudo iniciar GPSBabel</translation>
    </message>
    <message>
        <source>Importing data...</source>
        <translation>Importando datos...</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>Could not import data from %1!

</source>
        <translation>No se pudieron importar datos de %1

</translation>
    </message>
    <message>
        <source>Error importing data</source>
        <translation>Error al importar datos</translation>
    </message>
    <message>
        <source>Not supported</source>
        <translation>No soportado</translation>
    </message>
    <message>
        <source>This device does not support downloading </source>
        <translation>Este receptor no soporta la descarga </translation>
    </message>
    <message>
        <source>of </source>
        <translation>de </translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation>Descargando datos...</translation>
    </message>
    <message>
        <source>Could not download data from GPS!

</source>
        <translation>No se pudieron descargar datos del GPS

</translation>
    </message>
    <message>
        <source>Error downloading data</source>
        <translation>Error al descargar datos</translation>
    </message>
    <message>
        <source>This device does not support uploading of </source>
        <translation>Este receptor no soporta el cargar datos de </translation>
    </message>
    <message>
        <source>Uploading data...</source>
        <translation>Cargando datos...</translation>
    </message>
    <message>
        <source>Error while uploading data to GPS!

</source>
        <translation>Error al cargar datos al GPS

</translation>
    </message>
    <message>
        <source>Error uploading data</source>
        <translation>Error al cargar datos</translation>
    </message>
    <message>
        <source>Could not convert data from %1!

</source>
        <translation>¡No se pudieron convertir los datos desde %1!</translation>
    </message>
    <message>
        <source>Error converting data</source>
        <translation>Error al convertir datos</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>formato GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <source>Select GPX file</source>
        <translation>Seleccionar archivo GPX</translation>
    </message>
    <message>
        <source>Select file and format to import</source>
        <translation>Seleccionar archivo y formato a importar</translation>
    </message>
    <message>
        <source>Waypoints</source>
        <translation>Waypoints</translation>
    </message>
    <message>
        <source>Routes</source>
        <translation>Rutas</translation>
    </message>
    <message>
        <source>Tracks</source>
        <translation>Tracks</translation>
    </message>
    <message>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation>QGIS puede realizar conversiones de archivos GPX, usando GPSBabel (%1) para hacer las conversiones.</translation>
    </message>
    <message>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation>Esto requiere que tenga instalado GPSBabel donde QGIS lo pueda encontrar</translation>
    </message>
    <message>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation>GPX es el %1, que se usa para guardar información sobre waypoints, rutas y tracks.</translation>
    </message>
    <message>
        <source>GPS eXchange file format</source>
        <translation>Formato de archivo GPS eXchange</translation>
    </message>
    <message>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation>Seleccione un archivo GPX y luego elija el tipo de objetos espaciales que quiere cargar.</translation>
    </message>
    <message>
        <source>This tool will help you download data from a GPS device.</source>
        <translation>Esta herramienta le ayudará a descargar datos de un receptor GPS.</translation>
    </message>
    <message>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation>Seleccione su receptor GPS, el puerto al que está conectado, el tipo de objeto espacial que quiere descargar, un nombre para su nueva capa y el archivo GPX donde quiera guardar los datos.</translation>
    </message>
    <message>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation>Si su receptor no está en la lista o si quiere cambiar alguna configuración, también puede editar los receptores.</translation>
    </message>
    <message>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation>Esta herramienta usa el programa GPSBabel (%1) para transferir los datos.</translation>
    </message>
    <message>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation>Esta herramienta le ayudará a cargar datos de una capa GPX a un receptor GPS.</translation>
    </message>
    <message>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation>Seleccione la capa que quiere cargar , el receptor en el que la quiere cargar y el puerto al que está conectado.</translation>
    </message>
    <message>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation>QGIS sólo puede cargar archivos GPX por si mismo, pero otros muchos formatos se pueden convertir a GPX usando GPSBabel (%1).</translation>
    </message>
    <message>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation>No todos los formatos de archivo pueden guardar waypoints, rutas y tracks, por lo que algunos tipos de objeto espacial pueden estar deshabilitados para algunos formatos de archivo.</translation>
    </message>
    <message>
        <source>Choose a file name to save under</source>
        <translation>Seleccione un nombre de archivo bajo el que guardar</translation>
    </message>
    <message>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX file name that you want to save the converted file as, and a name for the new layer.</source>
        <translation>Seleccione un formato de archivo de GPS y el archivo que quiere importar, el tipo de objeto espacial que quiere usar, un nombre de archivo con el que quiera guardar el archivo convertido a GPX y un nombre para la nueva capa.</translation>
    </message>
    <message>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX file name that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation>Seleccione un nombre de archivo GPX de entrada, el tipo de conversión que quiere realizar, un nombre de archivo GPX con el que quiera guardar el archivo convertido y un nombre para la nueva capa creada a partir del resultado.</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <source>GPS Tools</source>
        <translation>Herramientas GPS</translation>
    </message>
    <message>
        <source>Load GPX file</source>
        <translation>Cargar archivo GPX</translation>
    </message>
    <message>
        <source>File:</source>
        <translation>Archivo:</translation>
    </message>
    <message>
        <source>Feature types:</source>
        <translation>Tipo de objetos espaciales:</translation>
    </message>
    <message>
        <source>Waypoints</source>
        <translation>Waypoints</translation>
    </message>
    <message>
        <source>Routes</source>
        <translation>Rutas</translation>
    </message>
    <message>
        <source>Tracks</source>
        <translation>Tracks</translation>
    </message>
    <message>
        <source>Import other file</source>
        <translation>Importar otro archivo</translation>
    </message>
    <message>
        <source>Layer name:</source>
        <translation>Nombre de la capa:</translation>
    </message>
    <message>
        <source>GPX output file:</source>
        <translation>Archivo de salida GPX:</translation>
    </message>
    <message>
        <source>Feature type:</source>
        <translation>Tipo de objeto espacial:</translation>
    </message>
    <message>
        <source>File to import:</source>
        <translation>Archivo a importar:</translation>
    </message>
    <message>
        <source>Download from GPS</source>
        <translation>Descargar desde GPS</translation>
    </message>
    <message>
        <source>Port:</source>
        <translation>Puerto:</translation>
    </message>
    <message>
        <source>Output file:</source>
        <translation>Archivo de salida:</translation>
    </message>
    <message>
        <source>GPS device:</source>
        <translation>Receptor GPS:</translation>
    </message>
    <message>
        <source>Edit devices</source>
        <translation>Editar receptores</translation>
    </message>
    <message>
        <source>Upload to GPS</source>
        <translation>Cargar al GPS</translation>
    </message>
    <message>
        <source>Data layer:</source>
        <translation>Capa de datos:</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Guardar como...</translation>
    </message>
    <message>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation>(Nota: ¡Seleccionar el tipo de archivo correcto en el diálogo de exploración es importante!)</translation>
    </message>
    <message>
        <source>GPX Conversions</source>
        <translation>Conversiones GPX</translation>
    </message>
    <message>
        <source>Conversion:</source>
        <translation>Conversión:</translation>
    </message>
    <message>
        <source>GPX input file:</source>
        <translation>Archivo GPX de entrada:</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Edit devices...</source>
        <translation>Editar dispositivos...</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>URI errónea - necesita especificar el tipo de objeto espacial.</translation>
    </message>
    <message>
        <source>GPS eXchange file</source>
        <translation>Archivo GPS eXchange</translation>
    </message>
    <message>
        <source>Digitized in QGIS</source>
        <translation>Digitalizado en QGIS</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelector</name>
    <message>
        <source>Define this layer&apos;s projection:</source>
        <translation>Definir la proyección de esta capa:</translation>
    </message>
    <message>
        <source>This layer appears to have no projection specification.</source>
        <translation>Parece que esta capa no tiene no tiene especificada la proyección.</translation>
    </message>
    <message>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation>Por omisión, se utilizará la misma que para el proyecto, pero puede ignorarla seleccionado una proyección diferente a continuación.</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelectorBase</name>
    <message>
        <source>Projection Selector</source>
        <translation>Selector de proyección</translation>
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
        <translation>Entero</translation>
    </message>
    <message>
        <source>String</source>
        <translation>Cadena</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialogBase</name>
    <message>
        <source>New Vector Layer</source>
        <translation>Nueva capa vectorial</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Línea</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Polígono</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>File format</source>
        <translation>Formato de archivo</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Atributos</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Delete selected attribute</source>
        <translation>Borrar el atributo seleccionado</translation>
    </message>
    <message>
        <source>Add attribute</source>
        <translation>Añadir atributo</translation>
    </message>
</context>
<context>
    <name>QgsGeorefDescriptionDialogBase</name>
    <message>
        <source>Description georeferencer</source>
        <translation>Georreferenciador de descripción</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Descripciónn&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;&quot;&gt;Este complemento puede generar archivos de referenciación (world files) para imágenes ráster. Seleccione puntos en el ráster e introduzca sus coordenadas y el complemento procesará los parámetros del archivo de referenciación. Cuantas más coordenadas pueda proporcionar, mejor será el resultado&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPlugin</name>
    <message>
        <source>&amp;Georeferencer</source>
        <translation>Geo&amp;rreferenciador</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <source>Choose a raster file</source>
        <translation>Seleccionar un archivo ráster</translation>
    </message>
    <message>
        <source>Raster files (*.*)</source>
        <translation>Archivos ráster (*.*)</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <source>The selected file is not a valid raster file.</source>
        <translation>El archivo seleccionado no es un archivo ráster válido.</translation>
    </message>
    <message>
        <source>World file exists</source>
        <translation>El archivo de referenciación (world file) ya existe</translation>
    </message>
    <message>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation>&lt;p&gt;Parece que el archivo seleccionado ya tiene un </translation>
    </message>
    <message>
        <source>world file! Do you want to replace it with the </source>
        <translation>archivo de referenciación (world file). ¿Quiere reemplazarlo con el </translation>
    </message>
    <message>
        <source>new world file?&lt;/p&gt;</source>
        <translation>nuevo archivo de referenciación (world file)?&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <source>Georeferencer</source>
        <translation>Georreferenciador</translation>
    </message>
    <message>
        <source>Raster file:</source>
        <translation>Archivo ráster:</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <source>Arrange plugin windows</source>
        <translation>Ajustar ventanas de complementos</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Description...</source>
        <translation>Descripción...</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialogBase</name>
    <message>
        <source>Warp options</source>
        <translation>Opciones de combado</translation>
    </message>
    <message>
        <source>Resampling method:</source>
        <translation>Método de remuestreo:</translation>
    </message>
    <message>
        <source>Nearest neighbour</source>
        <translation>Vecino más próximo</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Lineal</translation>
    </message>
    <message>
        <source>Cubic</source>
        <translation>Cúbica</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <source>Use 0 for transparency when needed</source>
        <translation>Usar 0 para transparencia cuando sea necesario</translation>
    </message>
    <message>
        <source>Compression:</source>
        <translation>Compresión:</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <source>Equal Interval</source>
        <translation>Intervalo igual</translation>
    </message>
    <message>
        <source>Quantiles</source>
        <translation>Cuantiles</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>Vacío</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <source>graduated Symbol</source>
        <translation>Símbolo graduado</translation>
    </message>
    <message>
        <source>Delete class</source>
        <translation>Borrar clase</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Clasificar</translation>
    </message>
    <message>
        <source>Classification field</source>
        <translation>Falló la clasificación</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Modo</translation>
    </message>
    <message>
        <source>Number of classes</source>
        <translation>Número de clases</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>Column</source>
        <translation>Columna</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Valor</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <source>ERROR</source>
        <translation>ERROR</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Capa</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <source>GRASS Attributes</source>
        <translation>Atributos de GRASS</translation>
    </message>
    <message>
        <source>Tab 1</source>
        <translation>Tab 1</translation>
    </message>
    <message>
        <source>result</source>
        <translation>resultado</translation>
    </message>
    <message>
        <source>Update database record</source>
        <translation>Actualizar registro de la base de datos</translation>
    </message>
    <message>
        <source>Update</source>
        <translation>Actualizar</translation>
    </message>
    <message>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Añadir nueva categoría utilizando las especificaciones de la caja de herramientas de edición de GRASS</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <source>Delete selected category</source>
        <translation>Borrar la categoría seleccionada</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <source>Tools</source>
        <translation>Herramientas</translation>
    </message>
    <message>
        <source>Add selected map to canvas</source>
        <translation>Añadir el mapa seleccionado a la vista del mapa</translation>
    </message>
    <message>
        <source>Copy selected map</source>
        <translation>Copiar el mapa seleccionado</translation>
    </message>
    <message>
        <source>Rename selected map</source>
        <translation>Cambiar nombre del mapa seleccionado</translation>
    </message>
    <message>
        <source>Delete selected map</source>
        <translation>Borrar el mapa seleccionado</translation>
    </message>
    <message>
        <source>Set current region to selected map</source>
        <translation>Establecer la región actual al mapa seleccionado</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>Cannot copy map </source>
        <translation>No se puede copiar el mapa </translation>
    </message>
    <message>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt;comando: </translation>
    </message>
    <message>
        <source>Cannot rename map </source>
        <translation>No se puede cambiar el nombre del mapa</translation>
    </message>
    <message>
        <source>Delete map &lt;b&gt;</source>
        <translation>Borrar mapa &lt;b&gt;</translation>
    </message>
    <message>
        <source>Cannot delete map </source>
        <translation>No se puede borrar el mapa </translation>
    </message>
    <message>
        <source>Cannot write new region</source>
        <translation>No se puede guardar una nueva región</translation>
    </message>
    <message>
        <source>New name</source>
        <translation>Nuevo nombre</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>No es propietario del directorio de mapas, no puede abrir el vectorial para editarlo.</translation>
    </message>
    <message>
        <source>Cannot open vector for update.</source>
        <translation>No se puede abrir el vectorial para actualizar.</translation>
    </message>
    <message>
        <source>Edit tools</source>
        <translation>Herramientas de edición</translation>
    </message>
    <message>
        <source>New point</source>
        <translation>Nuevo punto</translation>
    </message>
    <message>
        <source>New line</source>
        <translation>Nueva línea</translation>
    </message>
    <message>
        <source>New boundary</source>
        <translation>Nuevo contorno</translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation>Nuevo centroide</translation>
    </message>
    <message>
        <source>Move vertex</source>
        <translation>Mover vértice</translation>
    </message>
    <message>
        <source>Add vertex</source>
        <translation>Añadir vértice</translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation>Borrar vértice</translation>
    </message>
    <message>
        <source>Move element</source>
        <translation>Mover elemento</translation>
    </message>
    <message>
        <source>Split line</source>
        <translation>Dividir línea</translation>
    </message>
    <message>
        <source>Delete element</source>
        <translation>Borrar elemento</translation>
    </message>
    <message>
        <source>Edit attributes</source>
        <translation>Editar atributos</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <source>Info</source>
        <translation>Info</translation>
    </message>
    <message>
        <source>The table was created</source>
        <translation>La tabla ha sido creada</translation>
    </message>
    <message>
        <source>Tool not yet implemented.</source>
        <translation>Herramienta no implementada todavía.</translation>
    </message>
    <message>
        <source>Cannot check orphan record: </source>
        <translation>No se puede comprobar registro huérfano: </translation>
    </message>
    <message>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation>El registro huérfano se dejó en la tabla de atributos. &lt;br&gt;¿Borrar el registro?</translation>
    </message>
    <message>
        <source>Cannot delete orphan record: </source>
        <translation>No se puede borrar el registro huérfano: </translation>
    </message>
    <message>
        <source>Cannot describe table for field </source>
        <translation>No se puede describir la tabla por el campo </translation>
    </message>
    <message>
        <source>Left: </source>
        <translation>Izquierda: </translation>
    </message>
    <message>
        <source>Middle: </source>
        <translation>Medio: </translation>
    </message>
    <message>
        <source>Background</source>
        <translation>Fondo</translation>
    </message>
    <message>
        <source>Highlight</source>
        <translation>Resaltado</translation>
    </message>
    <message>
        <source>Dynamic</source>
        <translation>Dinámico</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Línea</translation>
    </message>
    <message>
        <source>Boundary (no area)</source>
        <translation>Contorno (sin área)</translation>
    </message>
    <message>
        <source>Boundary (1 area)</source>
        <translation>Contorno (1 área)</translation>
    </message>
    <message>
        <source>Boundary (2 areas)</source>
        <translation>Contorno (2 áreas)</translation>
    </message>
    <message>
        <source>Centroid (in area)</source>
        <translation>Centroide (en el área)</translation>
    </message>
    <message>
        <source>Centroid (outside area)</source>
        <translation>Centroide (fuera del área)</translation>
    </message>
    <message>
        <source>Centroid (duplicate in area)</source>
        <translation>Centroide (duplicado en el área)</translation>
    </message>
    <message>
        <source>Node (1 line)</source>
        <translation>Nodo (1 línea)</translation>
    </message>
    <message>
        <source>Node (2 lines)</source>
        <translation>Nodo (2 líneas)</translation>
    </message>
    <message>
        <source>Next not used</source>
        <translation>La siguiente sin usar</translation>
    </message>
    <message>
        <source>Manual entry</source>
        <translation>Entrada manual</translation>
    </message>
    <message>
        <source>No category</source>
        <translation>Ninguna categoría</translation>
    </message>
    <message>
        <source>Right: </source>
        <translation>Derecha: </translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <source>GRASS Edit</source>
        <translation>Edición de GRASS</translation>
    </message>
    <message>
        <source>Category</source>
        <translation>Categoría</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Modo</translation>
    </message>
    <message>
        <source>Settings</source>
        <translation>Configuración</translation>
    </message>
    <message>
        <source>Snapping in screen pixels</source>
        <translation>Autoensamblado en píxeles de la pantalla</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Simbología</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation>Ancho de línea</translation>
    </message>
    <message>
        <source>Marker size</source>
        <translation>Tamaño de marcador</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabla</translation>
    </message>
    <message>
        <source>Add Column</source>
        <translation>Añadir columna</translation>
    </message>
    <message>
        <source>Create / Alter Table</source>
        <translation>Crear / modificar tabla</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Capa</translation>
    </message>
    <message>
        <source>Disp</source>
        <translation type="unfinished">Título</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Color</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <source>Index</source>
        <translation>Índice</translation>
    </message>
    <message>
        <source>Column</source>
        <translation>Columna</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Longitud</translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;¡Introduzca un nombre!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;¡Este es el nombre del origen!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;¡Existe!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation>Sobrescribir</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <source>Mapcalc tools</source>
        <translation>Herramientas Mapcalc</translation>
    </message>
    <message>
        <source>Add map</source>
        <translation>Añadir mapa</translation>
    </message>
    <message>
        <source>Add constant value</source>
        <translation>Añadir constante</translation>
    </message>
    <message>
        <source>Add operator or function</source>
        <translation>Añadir operador o función</translation>
    </message>
    <message>
        <source>Add connection</source>
        <translation>Añadir conexión</translation>
    </message>
    <message>
        <source>Select item</source>
        <translation>Seleccionar elemento</translation>
    </message>
    <message>
        <source>Delete selected item</source>
        <translation>Borrar elemento seleccionado</translation>
    </message>
    <message>
        <source>Open</source>
        <translation>Abrir</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Guardar</translation>
    </message>
    <message>
        <source>Save as</source>
        <translation>Guardar como</translation>
    </message>
    <message>
        <source>Addition</source>
        <translation>Suma</translation>
    </message>
    <message>
        <source>Subtraction</source>
        <translation>Resta</translation>
    </message>
    <message>
        <source>Multiplication</source>
        <translation>Multiplicación</translation>
    </message>
    <message>
        <source>Division</source>
        <translation>División</translation>
    </message>
    <message>
        <source>Modulus</source>
        <translation>Módulo</translation>
    </message>
    <message>
        <source>Exponentiation</source>
        <translation>Exponencial</translation>
    </message>
    <message>
        <source>Equal</source>
        <translation>Igual</translation>
    </message>
    <message>
        <source>Not equal</source>
        <translation>Distinto</translation>
    </message>
    <message>
        <source>Greater than</source>
        <translation>Mayor que</translation>
    </message>
    <message>
        <source>Greater than or equal</source>
        <translation>Mayor o igual que</translation>
    </message>
    <message>
        <source>Less than</source>
        <translation>Menor que</translation>
    </message>
    <message>
        <source>Less than or equal</source>
        <translation>Menor o igual que</translation>
    </message>
    <message>
        <source>And</source>
        <translation>Y</translation>
    </message>
    <message>
        <source>Or</source>
        <translation>O</translation>
    </message>
    <message>
        <source>Absolute value of x</source>
        <translation>Valor absoluto de x</translation>
    </message>
    <message>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>Inverso de la tangente de x (resultado en grados)</translation>
    </message>
    <message>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>Inverso de la tangente de y/x (resultado en grados)</translation>
    </message>
    <message>
        <source>Current column of moving window (starts with 1)</source>
        <translation>Columna actual de la ventana movible (empieza con 1)</translation>
    </message>
    <message>
        <source>Cosine of x (x is in degrees)</source>
        <translation>Coseno de x (x en grados)</translation>
    </message>
    <message>
        <source>Convert x to double-precision floating point</source>
        <translation>Convertir x a coma flotante (precisión doble)</translation>
    </message>
    <message>
        <source>Current east-west resolution</source>
        <translation>Resolución actual Este-Oeste</translation>
    </message>
    <message>
        <source>Exponential function of x</source>
        <translation>Elevado a x</translation>
    </message>
    <message>
        <source>x to the power y</source>
        <translation>x elevado a y</translation>
    </message>
    <message>
        <source>Convert x to single-precision floating point</source>
        <translation>Convertir x a coma flotante (precisión simple)</translation>
    </message>
    <message>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>Decisión: 1 si x no es cero, 0 para el resto</translation>
    </message>
    <message>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>Decisión: a si x no es cero, 0 para el resto</translation>
    </message>
    <message>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>Decisión: a si x no es cero, b para el resto</translation>
    </message>
    <message>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>Decisión: a si x &gt; 0, b si x es cero, c si x &lt; 0</translation>
    </message>
    <message>
        <source>Convert x to integer [ truncates ]</source>
        <translation>Convertir x a entero [ truncar ]</translation>
    </message>
    <message>
        <source>Check if x = NULL</source>
        <translation>Comprobar si x = NULO</translation>
    </message>
    <message>
        <source>Natural log of x</source>
        <translation>Logaritmo neperiano de x</translation>
    </message>
    <message>
        <source>Log of x base b</source>
        <translation>Logaritmo de x en base b</translation>
    </message>
    <message>
        <source>Largest value</source>
        <translation>Máximo</translation>
    </message>
    <message>
        <source>Median value</source>
        <translation>Mediana</translation>
    </message>
    <message>
        <source>Smallest value</source>
        <translation>Mínimo</translation>
    </message>
    <message>
        <source>Mode value</source>
        <translation>Moda</translation>
    </message>
    <message>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1 si x = 0, 0 para el resto</translation>
    </message>
    <message>
        <source>Current north-south resolution</source>
        <translation>Resolución Norte-Sur actual</translation>
    </message>
    <message>
        <source>NULL value</source>
        <translation>Valor NULO</translation>
    </message>
    <message>
        <source>Random value between a and b</source>
        <translation>Valor aleatorio entre a y b</translation>
    </message>
    <message>
        <source>Round x to nearest integer</source>
        <translation>Redondear x al número entero más próximo</translation>
    </message>
    <message>
        <source>Current row of moving window (Starts with 1)</source>
        <translation>Fila actual de la ventana movible (empieza con 1)</translation>
    </message>
    <message>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation>Seno de x (x en grados)</translation>
    </message>
    <message>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>Raíz cuadrada de x</translation>
    </message>
    <message>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation>Tangente de x (x en grados)</translation>
    </message>
    <message>
        <source>Current x-coordinate of moving window</source>
        <translation>Coordenada X de la ventana movible</translation>
    </message>
    <message>
        <source>Current y-coordinate of moving window</source>
        <translation>Coordenada Y de la ventana movible</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation>No se puede estimar la región actual</translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation>No se puede comprobar la región del mapa </translation>
    </message>
    <message>
        <source>Cannot get region of map </source>
        <translation>No se puede obtener la región del mapa </translation>
    </message>
    <message>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>Actualmente no hay mapas ráster de GRASS en QGIS</translation>
    </message>
    <message>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>No se puede crear el directorio &quot;mapcalc&quot; en el directorio de mapas actual.</translation>
    </message>
    <message>
        <source>New mapcalc</source>
        <translation>Nuevo mapcalc</translation>
    </message>
    <message>
        <source>Enter new mapcalc name:</source>
        <translation>Introduzca el nombre del nuevo mapcalc:</translation>
    </message>
    <message>
        <source>Enter vector name</source>
        <translation>Introducir nombre del vectorial</translation>
    </message>
    <message>
        <source>The file already exists. Overwrite? </source>
        <translation>El archivo ya existe. ¿Desea sobrescribirlo?</translation>
    </message>
    <message>
        <source>Save mapcalc</source>
        <translation>Guardar mapcalc</translation>
    </message>
    <message>
        <source>File name empty</source>
        <translation>Nombre de archivo vacío</translation>
    </message>
    <message>
        <source>Cannot open mapcalc file</source>
        <translation>No se puede abrir el archivo mapcalc</translation>
    </message>
    <message>
        <source>The mapcalc schema (</source>
        <translation>El esquema del mapcalc (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) no se ha encontrado.</translation>
    </message>
    <message>
        <source>Cannot open mapcalc schema (</source>
        <translation>No se puede abrir el esquema del mapcalc (</translation>
    </message>
    <message>
        <source>Cannot read mapcalc schema (</source>
        <translation>No se puede leer el esquema del mapcalc (</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
en la línea </translation>
    </message>
    <message>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Salida</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <source>MainWindow</source>
        <translation>Ventana principal</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Salida</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <source>Run</source>
        <translation>Ejecutar</translation>
    </message>
    <message>
        <source>Stop</source>
        <translation>Detener</translation>
    </message>
    <message>
        <source>Module</source>
        <translation>Módulo</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>The module file (</source>
        <translation>El archivo del módulo (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) no se ha encontrado.</translation>
    </message>
    <message>
        <source>Cannot open module file (</source>
        <translation>No se puede abrir el archivo del módulo (</translation>
    </message>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>Cannot read module file (</source>
        <translation>No se puede leer el archivo del módulo (</translation>
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
en la línea </translation>
    </message>
    <message>
        <source>Module </source>
        <translation>Módulo </translation>
    </message>
    <message>
        <source> not found</source>
        <translation> no encontrado</translation>
    </message>
    <message>
        <source>Cannot find man page </source>
        <translation>No se puede encontrar la página del manual </translation>
    </message>
    <message>
        <source>Not available, cannot open description (</source>
        <translation>No disponible, no se puede abrir la descripción (</translation>
    </message>
    <message>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <source>Not available, incorrect description (</source>
        <translation>No disponible, descripción incorrecta (</translation>
    </message>
    <message>
        <source>Cannot get input region</source>
        <translation>No se puede obtener la región de entrada</translation>
    </message>
    <message>
        <source>Use Input Region</source>
        <translation>Usar región de entrada</translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation>No se puede encontrar el módulo </translation>
    </message>
    <message>
        <source>Cannot start module: </source>
        <translation>No se puede iniciar el módulo: </translation>
    </message>
    <message>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation>&lt;B&gt;Finalizado correctamente&lt;/B&gt;</translation>
    </message>
    <message>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation>&lt;B&gt;Finalizado con error&lt;/B&gt;</translation>
    </message>
    <message>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation>&lt;B&gt;Módulo finalizado o matado&lt;/B&gt;</translation>
    </message>
    <message>
        <source>Not available, description not found (</source>
        <translation>No disponible, no se encuentra la descripción (</translation>
    </message>
    <message>
        <source>Please ensure you have the GRASS documentation installed.</source>
        <translation>Por favor, asegúrese de que tiene instalada la documentación de GRASS.</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <source>GRASS Module</source>
        <translation>Módulo de GRASS</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opciones</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Salida</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation>Manual</translation>
    </message>
    <message>
        <source>Run</source>
        <translation>Ejecutar</translation>
    </message>
    <message>
        <source>View output</source>
        <translation>Ver salida</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation>EtiquetaDeTexto</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <source>Attribute field</source>
        <translation>Campo de atributos</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <source>File</source>
        <translation>Archivo</translation>
    </message>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;falta el valor</translation>
    </message>
    <message>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation>:&amp;nbsp;el directorio no existe</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>Cannot find layeroption </source>
        <translation>No se puede encontrar la opción capa </translation>
    </message>
    <message>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation>El controlador PostGIS de OGR no soporta esquemas.&lt;br&gt;Sólo se usará el nombre de la tabla.&lt;br&gt;Esto puede dar como resultado entradas incorrectas si hay más tablas con el mismo nombre&lt;br&gt;en la base de datos.</translation>
    </message>
    <message>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;ninguna entrada</translation>
    </message>
    <message>
        <source>Cannot find whereoption </source>
        <translation>No se puede encontrar la opción dónde </translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>Cannot find typeoption </source>
        <translation>No se puede encontrar la opción tipo </translation>
    </message>
    <message>
        <source>Cannot find values for typeoption </source>
        <translation>No se pueden encontrar valores para la opción tipo </translation>
    </message>
    <message>
        <source>Cannot find layeroption </source>
        <translation>No se puede encontrar la opción capa </translation>
    </message>
    <message>
        <source>GRASS element </source>
        <translation>Elemento de GRASS </translation>
    </message>
    <message>
        <source> not supported</source>
        <translation> no soportado</translation>
    </message>
    <message>
        <source>Use region of this map</source>
        <translation>Usar la región de este mapa</translation>
    </message>
    <message>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;ninguna entrada</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;falta el valor</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <source>Attribute field</source>
        <translation>Campo de atributo</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation>No se puede encontrar el módulo </translation>
    </message>
    <message>
        <source>Cannot start module </source>
        <translation>No se puede iniciar el módulo </translation>
    </message>
    <message>
        <source>Cannot read module description (</source>
        <translation>No se puede leer la descripción del módulo (</translation>
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
en la línea </translation>
    </message>
    <message>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <source>Cannot find key </source>
        <translation>No se puede encontrar la clave </translation>
    </message>
    <message>
        <source>Item with id </source>
        <translation>Ítem con ID </translation>
    </message>
    <message>
        <source> not found</source>
        <translation> no encontrado</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation>No se puede obtener la región actual</translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation>No se puede comprobar la región del mapa </translation>
    </message>
    <message>
        <source>Cannot set region of map </source>
        <translation>No se puede establecer la región del mapa </translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <source>Database</source>
        <translation>Base de datos</translation>
    </message>
    <message>
        <source>Location 2</source>
        <translation>Localización 2</translation>
    </message>
    <message>
        <source>User&apos;s mapset</source>
        <translation>Directorio de mapas del usuario</translation>
    </message>
    <message>
        <source>System mapset</source>
        <translation>Directorio de mapas del sistema</translation>
    </message>
    <message>
        <source>Location 1</source>
        <translation>Localización 1</translation>
    </message>
    <message>
        <source>Enter path to GRASS database</source>
        <translation>Introduzca la ruta a la base de datos de GRASS</translation>
    </message>
    <message>
        <source>The directory doesn&apos;t exist!</source>
        <translation>¡El directorio no existe!</translation>
    </message>
    <message>
        <source>No writable locations, the database not writable!</source>
        <translation>¡Ninguna localización que se pueda escribir, la base de datos no se puede escribir!</translation>
    </message>
    <message>
        <source>Enter location name!</source>
        <translation>¡Introduzca el nombre de la localización!</translation>
    </message>
    <message>
        <source>The location exists!</source>
        <translation>¡La localización existe!</translation>
    </message>
    <message>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>¡La proyección seleccionada no está soportada por GRASS!</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>Cannot create projection.</source>
        <translation>No se puede crear la proyección.</translation>
    </message>
    <message>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation>No se puede reproyectar la región establecida previamente, se establece la región predeterminada.</translation>
    </message>
    <message>
        <source>North must be greater than south</source>
        <translation>El Norte debe ser mayor que el Sur</translation>
    </message>
    <message>
        <source>East must be greater than west</source>
        <translation>El Este debe ser mayor que el Oeste</translation>
    </message>
    <message>
        <source>Regions file (</source>
        <translation>Archivo de regiones (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) no encontrado.</translation>
    </message>
    <message>
        <source>Cannot open locations file (</source>
        <translation>No se puede abrir el archivo de localizaciones (</translation>
    </message>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>Cannot read locations file (</source>
        <translation>No se puede leer el archivo de localizaciones (</translation>
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
en la línea </translation>
    </message>
    <message>
        <source> column </source>
        <translation> columna </translation>
    </message>
    <message>
        <source>Cannot reproject selected region.</source>
        <translation>No se puede reproyectar la región seleccionada.</translation>
    </message>
    <message>
        <source>Cannot reproject region</source>
        <translation>No se puede reproyectar la región</translation>
    </message>
    <message>
        <source>Enter mapset name.</source>
        <translation>Introduzca el nombre del directorio de mapas.</translation>
    </message>
    <message>
        <source>The mapset already exists</source>
        <translation>El directorio de mapas ya existe</translation>
    </message>
    <message>
        <source>Database: </source>
        <translation>Base de datos: </translation>
    </message>
    <message>
        <source>Location: </source>
        <translation>Localización: </translation>
    </message>
    <message>
        <source>Mapset: </source>
        <translation>Directorio de mapas: </translation>
    </message>
    <message>
        <source>Create location</source>
        <translation>Crear localización</translation>
    </message>
    <message>
        <source>Cannot create new location: </source>
        <translation>No se puede crear la nueva localización: </translation>
    </message>
    <message>
        <source>Create mapset</source>
        <translation>Crear directorio de mapas</translation>
    </message>
    <message>
        <source>Cannot open DEFAULT_WIND</source>
        <translation>No se puede abrir DEFAULT_WIND</translation>
    </message>
    <message>
        <source>Cannot open WIND</source>
        <translation>No se puede abrir WIND</translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation>Nuevo directorio de mapas</translation>
    </message>
    <message>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>El nuevo directorio de mapas de creó correctamente, pero no se puede abrir: </translation>
    </message>
    <message>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>El nuevo directorio de mapas se creó correctamente y se estableció como el directorio de mapas de trabajo actual.</translation>
    </message>
    <message>
        <source>Cannot create new mapset directory</source>
        <translation>No se puede crear el nuevo directorio de mapas</translation>
    </message>
    <message>
        <source>Cannot create QgsCoordinateReferenceSystem</source>
        <translation>No se puede crear QgsCoordinateReferenceSystem</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <source>Example directory tree:</source>
        <translation>Ejemplo de árbol de directorios:</translation>
    </message>
    <message>
        <source>Database Error</source>
        <translation>Error de la base de datos</translation>
    </message>
    <message>
        <source>Database:</source>
        <translation>Base de datos:</translation>
    </message>
    <message>
        <source>Select existing directory or create a new one:</source>
        <translation>Seleccionar un directorio existente o crear uno nuevo:</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Localización</translation>
    </message>
    <message>
        <source>Select location</source>
        <translation>Seleccionar localización</translation>
    </message>
    <message>
        <source>Create new location</source>
        <translation>Crear nueva localización</translation>
    </message>
    <message>
        <source>Location Error</source>
        <translation>Error en la localización</translation>
    </message>
    <message>
        <source>Projection Error</source>
        <translation>Error de la proyección</translation>
    </message>
    <message>
        <source>Coordinate system</source>
        <translation>Sistema de coordenadas</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation>Proyección</translation>
    </message>
    <message>
        <source>Not defined</source>
        <translation>Sin definir</translation>
    </message>
    <message>
        <source>Set current QGIS extent</source>
        <translation>Establecer la extensión actual de QGIS</translation>
    </message>
    <message>
        <source>Set</source>
        <translation>Establecer</translation>
    </message>
    <message>
        <source>Region Error</source>
        <translation>Error de la región</translation>
    </message>
    <message>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <source>W</source>
        <translation>O</translation>
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
        <translation>Nuevo directorio de mapas:</translation>
    </message>
    <message>
        <source>Mapset Error</source>
        <translation>Error del directorio de mapas</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;center&quot;&gt;Directorios de mapas existentes&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Location:</source>
        <translation>Localización:</translation>
    </message>
    <message>
        <source>Mapset:</source>
        <translation>Directorio de mapas:</translation>
    </message>
    <message>
        <source>New Mapset</source>
        <translation>Nuevo directorio de mapas</translation>
    </message>
    <message>
        <source>GRASS Database</source>
        <translation>Base de datos de GRASS</translation>
    </message>
    <message>
        <source>Tree</source>
        <translation>Árbol</translation>
    </message>
    <message>
        <source>Comment</source>
        <translation>Comentario</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Los datos de GRASS se guardan en una estructura de directorios en árbol. La base de datos de GRASS es el directorio superior en esta estructura en árbol.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
    <message>
        <source>GRASS Location</source>
        <translation>Localización de GRASS</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;La localización de GRASS es una colección de mapas de un territorio o proyecto en particular.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Default GRASS Region</source>
        <translation>Región predeterminada de GRASS</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;La región de GRASS define un entorno de trabajo para los módulos ráster. La región predeterminada es válida para una localización. Es posible establecer una región diferente en cada directorio de mapas. Se puede cambiar la región predeterminada más tarde.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation>Directorio de mapa</translation>
    </message>
    <message>
        <source>Owner</source>
        <translation>Propietario</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;El directorio de mapas de GRASS es una colección de mapas utilizados por un usuario. Un usuario puede leer mapas de todos los directorios de mapas de la localización, pero sólo puede abrir para editar su directorio de mapas.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Create New Mapset</source>
        <translation>Crear nuevo directorio de mapas</translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <source>Open mapset</source>
        <translation>Abrir directorio de mapas</translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation>Nuevo directorio de mapas</translation>
    </message>
    <message>
        <source>Close mapset</source>
        <translation>Cerrar directorio de mapas</translation>
    </message>
    <message>
        <source>Add GRASS vector layer</source>
        <translation>Añadir capa vectorial de GRASS</translation>
    </message>
    <message>
        <source>Add GRASS raster layer</source>
        <translation>Añadir capa ráster de GRASS</translation>
    </message>
    <message>
        <source>Open GRASS tools</source>
        <translation>Abrir herramientas de GRASS</translation>
    </message>
    <message>
        <source>Display Current Grass Region</source>
        <translation>Mostrar región actual de Grass</translation>
    </message>
    <message>
        <source>Edit Current Grass Region</source>
        <translation>Editar la región actual de Grass</translation>
    </message>
    <message>
        <source>Edit Grass Vector layer</source>
        <translation>Editar capa vectorial de Grass</translation>
    </message>
    <message>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>Añade una capa vectorial de GRASS a la vista del mapa</translation>
    </message>
    <message>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>Añade una capa ráster de GRASS a la vista del mapa</translation>
    </message>
    <message>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Muestra la región actual de GRASS como un rectángulo en la vista del mapa</translation>
    </message>
    <message>
        <source>Edit the current GRASS region</source>
        <translation>Editar la región actual de GRASS</translation>
    </message>
    <message>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Editar la capa vectorial de GRASS seleccionada actualmente.</translation>
    </message>
    <message>
        <source>&amp;GRASS</source>
        <translation>&amp;GRASS</translation>
    </message>
    <message>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <source>GrassVector</source>
        <translation>VectorialDeGrass</translation>
    </message>
    <message>
        <source>0.1</source>
        <translation>0.1</translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation>Capa de GRASS</translation>
    </message>
    <message>
        <source>Create new Grass Vector</source>
        <translation>Crear nuevo vectorial de Grass</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>GRASS Edit is already running.</source>
        <translation>Le edición de GRASS ya se está ejecutando.</translation>
    </message>
    <message>
        <source>New vector name</source>
        <translation>Nombre del nuevo vectorial</translation>
    </message>
    <message>
        <source>Cannot create new vector: </source>
        <translation>No se puede crear el nuevo vectorial: </translation>
    </message>
    <message>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation>Se creó el nuevo vectorial pero el proveedor de datos no lo puede abrir.</translation>
    </message>
    <message>
        <source>Cannot start editing.</source>
        <translation>No se puede iniciar la edición.</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>BASE DE DATOS, NOMBRE_LOCALIZACIÓN o DIRECTORIO DE MAPAS no están establecidos, no se puede mostrar la región actual.</translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation>No se puede leer la región actual: </translation>
    </message>
    <message>
        <source>Cannot open the mapset. </source>
        <translation>No se abrir el directorio de mapas. </translation>
    </message>
    <message>
        <source>Cannot close mapset. </source>
        <translation>No se puede cerrar el directorio de mapas. </translation>
    </message>
    <message>
        <source>Cannot close current mapset. </source>
        <translation>No se puede cerrar el directorio de mapas actual. </translation>
    </message>
    <message>
        <source>Cannot open GRASS mapset. </source>
        <translation>No se puede abrir el directorio de mapas de GRASS. </translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>BASE DE DATOS, NOMBRE_LOCALIZACIÓN o DIRECTORIO DE MAPAS no están establecidos, no se puede mostrar la región actual.</translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation>No se puede leer la región actual: </translation>
    </message>
    <message>
        <source>Cannot write region</source>
        <translation>No se puede escribir la región</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <source>GRASS Region Settings</source>
        <translation>Configuración de la región de GRASS</translation>
    </message>
    <message>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <source>W</source>
        <translation>O</translation>
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
        <translation>Resolución N-S</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation>Filas</translation>
    </message>
    <message>
        <source>Cols</source>
        <translation>Columnas</translation>
    </message>
    <message>
        <source>E-W Res</source>
        <translation>Resolución E-O</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Color</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Anchura</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <source>Select GRASS Vector Layer</source>
        <translation>Seleccionar capa vectorial de GRASS</translation>
    </message>
    <message>
        <source>Select GRASS Raster Layer</source>
        <translation>Seleccionar capa ráster de GRASS</translation>
    </message>
    <message>
        <source>Select GRASS mapcalc schema</source>
        <translation>Seleccionar esquema mapcalc de GRASS</translation>
    </message>
    <message>
        <source>Select GRASS Mapset</source>
        <translation>Seleccionar directorio de mapas de GRASS</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation>No se puede abrir el vectorial en el nivel 2 (topología no disponible).</translation>
    </message>
    <message>
        <source>Choose existing GISDBASE</source>
        <translation>Elija una base de datos (GISBASE) existente</translation>
    </message>
    <message>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>Base de datos (GISBASE) incorrecta, ninguna localización disponible.</translation>
    </message>
    <message>
        <source>Wrong GISDBASE</source>
        <translation>Base de datos (GISBASE) incorrecta</translation>
    </message>
    <message>
        <source>Select a map.</source>
        <translation>Seleccionar un mapa.</translation>
    </message>
    <message>
        <source>No map</source>
        <translation>Ningún mapa</translation>
    </message>
    <message>
        <source>No layer</source>
        <translation>Ninguna capa</translation>
    </message>
    <message>
        <source>No layers available in this map</source>
        <translation>No hay ninguna capa disponible en este mapa</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <source>Add GRASS Layer</source>
        <translation>Añadir capa de GRASS</translation>
    </message>
    <message>
        <source>Gisdbase</source>
        <translation>Base de datos (Gisdbase)</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Localización</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation>Directorio de mapa</translation>
    </message>
    <message>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Seleccionar o escribir el nombre del mapa (se aceptan los comodines &apos;*&apos; y &apos;?&apos; para los ráster)</translation>
    </message>
    <message>
        <source>Map name</source>
        <translation>Nombre del mapa</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Capa</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Explorar</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <source>GRASS Shell</source>
        <translation>Consola de GRASS</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <source>Browser</source>
        <translation>Explorador</translation>
    </message>
    <message>
        <source>GRASS Tools</source>
        <translation>Herramientas de GRASS</translation>
    </message>
    <message>
        <source>GRASS Tools: </source>
        <translation>Herramientas de GRASS: </translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>Cannot find MSYS (</source>
        <translation>No se puede encontrar MSYS (</translation>
    </message>
    <message>
        <source>GRASS Shell is not compiled.</source>
        <translation>La consola de GRASS no está compilada.</translation>
    </message>
    <message>
        <source>The config file (</source>
        <translation>El archivo de configuración (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) no se ha encontrado.</translation>
    </message>
    <message>
        <source>Cannot open config file (</source>
        <translation>No se puede abrir el archivo de configuración (</translation>
    </message>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>Cannot read config file (</source>
        <translation>No se puede leer el archivo de configuración (</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
en la línea </translation>
    </message>
    <message>
        <source> column </source>
        <translation> columna </translation>
    </message>
</context>
<context>
    <name>QgsGrassToolsBase</name>
    <message>
        <source>Grass Tools</source>
        <translation>Herramientas de GRASS</translation>
    </message>
    <message>
        <source>Modules Tree</source>
        <translation>Árbol de módulos</translation>
    </message>
    <message>
        <source>1</source>
        <translation>1</translation>
    </message>
    <message>
        <source>Modules List</source>
        <translation>Lista de módulos</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPlugin</name>
    <message>
        <source>&amp;Graticule Creator</source>
        <translation>&amp;Generador de cuadrícula</translation>
    </message>
    <message>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation>Crea una cuadricula (grid) y guarda el resultado como un archivo shape</translation>
    </message>
    <message>
        <source>&amp;Graticules</source>
        <translation>&amp;Cuadrículas</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <source>QGIS - Grid Maker</source>
        <translation>QGIS - Creador de cuadrículas</translation>
    </message>
    <message>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>Archivo Shape de ESRI (*.shp)</translation>
    </message>
    <message>
        <source>Please enter the file name before pressing OK!</source>
        <translation>¡Por favor, introduzca el nombre de archivo antes de pulsar Aceptar!</translation>
    </message>
    <message>
        <source>Please enter intervals before pressing OK!</source>
        <translation>¡Por favor, introduzca los intervalos antes de pulsar Aceptar!</translation>
    </message>
    <message>
        <source>Choose a file name to save under</source>
        <translation>Seleccione un nombre de archivo bajo el que guardar</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <source>Graticule Builder</source>
        <translation>Generador de cuadrículas</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Polígono</translation>
    </message>
    <message>
        <source>Origin (lower left)</source>
        <translation>Origen (esquina inferior izquierda)</translation>
    </message>
    <message>
        <source>End point (upper right)</source>
        <translation>Punto final (esquina superior derecha)</translation>
    </message>
    <message>
        <source>Output (shape) file</source>
        <translation>Archivo de salida (shape)</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Guardar como...</translation>
    </message>
    <message>
        <source>QGIS Graticule Creator</source>
        <translation>Generador de cuadrículas de QGIS</translation>
    </message>
    <message>
        <source>Graticle size</source>
        <translation>Tamaño de cuadrícula</translation>
    </message>
    <message>
        <source>Y Interval:</source>
        <translation>Intervalo Y:</translation>
    </message>
    <message>
        <source>X Interval:</source>
        <translation>Intervalo X:</translation>
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
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Este complemento le ayudará a construir un archivo shape de cuadrícula que pueda usar superpuesto en su visor de mapas de QGIS.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;;&quot;&gt;Por favor, introduzca todas las unidades en grados decimales&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <source>This help file does not exist for your language</source>
        <translation>Este archivo de ayuda no existe en su idioma</translation>
    </message>
    <message>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation>Si desea crearlo, contacte con el equipo de desarrollo de QGIS</translation>
    </message>
    <message>
        <source>Quantum GIS Help</source>
        <translation>Ayuda de Quantum GIS</translation>
    </message>
    <message>
        <source>Quantum GIS Help - </source>
        <translation>Ayuda de Quantum GIS - </translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <source>Failed to get the help text from the database</source>
        <translation>No se pudo obtener el texto de ayuda de la base de datos</translation>
    </message>
    <message>
        <source>The QGIS help database is not installed</source>
        <translation>La base de datos de ayuda de QGIS no está instalada</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <source>QGIS Help</source>
        <translation>Ayuda de QGIS</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
    <message>
        <source>&amp;Home</source>
        <translation>&amp;Inicio</translation>
    </message>
    <message>
        <source>Alt+H</source>
        <translation>Alt+I</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>A&amp;delante</translation>
    </message>
    <message>
        <source>Alt+F</source>
        <translation>Alt+D</translation>
    </message>
    <message>
        <source>&amp;Back</source>
        <translation>A&amp;trás</translation>
    </message>
    <message>
        <source>Alt+B</source>
        <translation>Alt+T</translation>
    </message>
</context>
<context>
    <name>QgsHttpTransaction</name>
    <message>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation>El servidor WMS ha respondido de forma inesperada con el código de estado HTTP %1 (%2)</translation>
    </message>
    <message>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>Respuesta HTTP completada, sin embargo ha habido un error: %1</translation>
    </message>
    <message>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>Transacción HTTP completada, sin embargo ha habido un error: %1</translation>
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
        <translation>Diálogo</translation>
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
        <source>Feature</source>
        <translation>Objeto espacial</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Valor</translation>
    </message>
    <message>
        <source>Run action</source>
        <translation>Ejecutar acción</translation>
    </message>
    <message>
        <source>(Derived)</source>
        <translation>(Derivado)</translation>
    </message>
    <message>
        <source>Identify Results - </source>
        <translation>Resultados de la identificación - </translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <source>Identify Results</source>
        <translation>Resultados de la identificación</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialog</name>
    <message>
        <source>Triangular interpolation (TIN)</source>
        <translation>Interpolación triangular (TIN)</translation>
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
        <translation>Complemento de interpolación</translation>
    </message>
    <message>
        <source>Input</source>
        <translation>Entrada</translation>
    </message>
    <message>
        <source>Input vector layer</source>
        <translation>Capa vectorial de entrada</translation>
    </message>
    <message>
        <source>Use z-Coordinate for interpolation</source>
        <translation>Usar coordenada Z para interpolación</translation>
    </message>
    <message>
        <source>Interpolation attribute </source>
        <translation>Atributo de interpolación </translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Salida</translation>
    </message>
    <message>
        <source>Interpolation method</source>
        <translation>Método de interpolación</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Number of columns</source>
        <translation>Número de columnas</translation>
    </message>
    <message>
        <source>Number of rows</source>
        <translation>Número de filas</translation>
    </message>
    <message>
        <source>Output file </source>
        <translation>Archivo de salida </translation>
    </message>
</context>
<context>
    <name>QgsInterpolationPlugin</name>
    <message>
        <source>&amp;Interpolation</source>
        <translation>&amp;Interpolación</translation>
    </message>
</context>
<context>
    <name>QgsLUDialogBase</name>
    <message>
        <source>Enter class bounds</source>
        <translation>Introducir contornos de la clase</translation>
    </message>
    <message>
        <source>Lower value</source>
        <translation>Valor más bajo</translation>
    </message>
    <message>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <source>Upper value</source>
        <translation>Valor más alto</translation>
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
        <source>Font size units</source>
        <translation>Unidades del tamaño de fuente</translation>
    </message>
    <message>
        <source>Map units</source>
        <translation>Unidades del mapa</translation>
    </message>
    <message>
        <source>Points</source>
        <translation>Puntos</translation>
    </message>
    <message>
        <source>Transparency:</source>
        <translation>Transparencia:</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Ubicación</translation>
    </message>
    <message>
        <source>Below Right</source>
        <translation>Abajo derecha</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>Derecha</translation>
    </message>
    <message>
        <source>Below</source>
        <translation>Abajo</translation>
    </message>
    <message>
        <source>Over</source>
        <translation>Sobre</translation>
    </message>
    <message>
        <source>Above</source>
        <translation>Encima de</translation>
    </message>
    <message>
        <source>Left</source>
        <translation>Izquierda</translation>
    </message>
    <message>
        <source>Below Left</source>
        <translation>Abajo izquierda</translation>
    </message>
    <message>
        <source>Above Right</source>
        <translation>Encima derecha</translation>
    </message>
    <message>
        <source>Above Left</source>
        <translation>Abajo izquierda</translation>
    </message>
    <message>
        <source>Buffer</source>
        <translation>Margen</translation>
    </message>
    <message>
        <source>Buffer size units</source>
        <translation>Unidades de tamaño del margen</translation>
    </message>
    <message>
        <source>Size is in map units</source>
        <translation>Tamaño en unidades del mapa</translation>
    </message>
    <message>
        <source>Size is in points</source>
        <translation>El tamaño está en puntos</translation>
    </message>
    <message>
        <source>Size:</source>
        <translation>Tamaño:</translation>
    </message>
    <message>
        <source>Position</source>
        <translation>Posición</translation>
    </message>
    <message>
        <source>Offset units</source>
        <translation>Unidades de desplazamiento</translation>
    </message>
    <message>
        <source>Preview:</source>
        <translation>Previsualizar:</translation>
    </message>
    <message>
        <source>QGIS Rocks!</source>
        <translation>¡QGIS Avanza!</translation>
    </message>
    <message>
        <source>Field containing label</source>
        <translation>Campo que contiene la etiqueta</translation>
    </message>
    <message>
        <source>Default label</source>
        <translation>Etiqueta predeterminada</translation>
    </message>
    <message>
        <source>Data defined style</source>
        <translation>Estilo definido por datos</translation>
    </message>
    <message>
        <source>Data defined alignment</source>
        <translation>Alineación definida por datos</translation>
    </message>
    <message>
        <source>Data defined buffer</source>
        <translation>Buffer definido por datos</translation>
    </message>
    <message>
        <source>Data defined position</source>
        <translation>Posición definida por datos</translation>
    </message>
    <message>
        <source>Font transparency</source>
        <translation>Transparencia de tipo de letra</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Color</translation>
    </message>
    <message>
        <source>Angle (deg)</source>
        <translation>Ángulo (grados)</translation>
    </message>
    <message>
        <source>Buffer labels?</source>
        <translation>¿Hacer buffer de etiquetas?</translation>
    </message>
    <message>
        <source>Buffer size</source>
        <translation>Tamaño de buffer</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Transparencia</translation>
    </message>
    <message>
        <source>X Offset (pts)</source>
        <translation>Desplazamiento X (pts)</translation>
    </message>
    <message>
        <source>Y Offset (pts)</source>
        <translation>Desplazamiento Y (pts)</translation>
    </message>
    <message>
        <source>&amp;Font family</source>
        <translation>&amp;Familia del tipo de letra</translation>
    </message>
    <message>
        <source>&amp;Bold</source>
        <translation>&amp;Negrita</translation>
    </message>
    <message>
        <source>&amp;Italic</source>
        <translation>&amp;Cursiva</translation>
    </message>
    <message>
        <source>&amp;Underline</source>
        <translation>&amp;Subrayado</translation>
    </message>
    <message>
        <source>&amp;Size</source>
        <translation>&amp;Tamaño</translation>
    </message>
    <message>
        <source>Size units</source>
        <translation>Unidades de tamaño</translation>
    </message>
    <message>
        <source>X Coordinate</source>
        <translation>Coordenada X</translation>
    </message>
    <message>
        <source>Y Coordinate</source>
        <translation>Coordenada Y</translation>
    </message>
    <message>
        <source>Multiline labels?</source>
        <translation>¿Etiquetas multilínea?</translation>
    </message>
    <message encoding="UTF-8">
        <source>°</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>General</source>
        <translation type="unfinished">General</translation>
    </message>
    <message>
        <source>Use scale dependent rendering</source>
        <translation type="unfinished">Utilizar represenación dependiente de la escala</translation>
    </message>
    <message>
        <source>Maximum</source>
        <translation type="unfinished">Máximo</translation>
    </message>
    <message>
        <source>Minimum</source>
        <translation type="unfinished">Mínimo</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation type="unfinished">Escala mínima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation type="unfinished">Escala máxima a la que se mostrará esta capa. </translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <source>group</source>
        <translation>grupo</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>E&amp;liminar</translation>
    </message>
    <message>
        <source>&amp;Make to toplevel item</source>
        <translation>&amp;Subir el elemento al nivel superior</translation>
    </message>
    <message>
        <source>Re&amp;name</source>
        <translation>Cambiar &amp;nombre</translation>
    </message>
    <message>
        <source>&amp;Add group</source>
        <translation>&amp;Añadir grupo</translation>
    </message>
    <message>
        <source>&amp;Expand all</source>
        <translation>&amp;Expandir todo</translation>
    </message>
    <message>
        <source>&amp;Collapse all</source>
        <translation>&amp;Comprimir todo</translation>
    </message>
    <message>
        <source>Show file groups</source>
        <translation>Mostrar grupos de archivos</translation>
    </message>
    <message>
        <source>No Layer Selected</source>
        <translation>Ninguna capa no seleccionada</translation>
    </message>
    <message>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Para abrir una tabla de atributos, debe seleccionar una capa vectorial en la leyenda</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Zum a la extensión de la capa</translation>
    </message>
    <message>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation>&amp;Zum a la mejor escala (100%)</translation>
    </message>
    <message>
        <source>&amp;Show in overview</source>
        <translation>Mo&amp;strar en la vista general</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>E&amp;liminar</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Abrir tabla de atributos</translation>
    </message>
    <message>
        <source>Save as shapefile...</source>
        <translation>Guardar como archivo shape...</translation>
    </message>
    <message>
        <source>Save selection as shapefile...</source>
        <translation>Guardar selección como archivo shape...</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation>&amp;Propiedades</translation>
    </message>
    <message>
        <source>Multiple layers</source>
        <translation>Capas múltiples</translation>
    </message>
    <message>
        <source>This item contains multiple layers. Displaying multiple layers in the table is not supported.</source>
        <translation>El elemento contiene múltiples capas. Visualizar múltiples capas en la tabla no está implementado.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <source>Save layer as...</source>
        <translation>Guardar capa como...</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <source>Saving done</source>
        <translation>Guardado terminado</translation>
    </message>
    <message>
        <source>Export to Shapefile has been completed</source>
        <translation>La exportación a archivo shape se ha completado</translation>
    </message>
    <message>
        <source>Driver not found</source>
        <translation>No se ha encontrado el controlador</translation>
    </message>
    <message>
        <source>ESRI Shapefile driver is not available</source>
        <translation>El controlador de archivos shape de ESRI no está disponible</translation>
    </message>
    <message>
        <source>Error creating shapefile</source>
        <translation>Error al crear el archivo shape</translation>
    </message>
    <message>
        <source>The shapefile could not be created (</source>
        <translation>El archivo shape no se ha podido crear (</translation>
    </message>
    <message>
        <source>Layer creation failed</source>
        <translation>Ha fallado la creación de la capa</translation>
    </message>
    <message>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Zum a la extensión de la capa</translation>
    </message>
    <message>
        <source>&amp;Show in overview</source>
        <translation>Mo&amp;strar en la vista general</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>E&amp;liminar</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Abrir tabla de atributos</translation>
    </message>
    <message>
        <source>Save as shapefile...</source>
        <translation>Guardar como archivo shape...</translation>
    </message>
    <message>
        <source>Save selection as shapefile...</source>
        <translation>Guardar selección como archivo shape...</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation>&amp;Propiedades</translation>
    </message>
    <message>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation>La tabla de atributos de la capa contiene tipos de datos no soportados</translation>
    </message>
    <message>
        <source>Select the coordinate reference system for the saved shapefile.</source>
        <translation>Seleccione el sistema de referencia de coordenadas para el archivo shape guardado.</translation>
    </message>
    <message>
        <source>The data points will be transformed from the layer coordinate reference system.</source>
        <translation>Los puntos de datos se transformarán a partir del sistema de referencia de coordenadas de la capa.</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <source>Could not draw</source>
        <translation>No se pudo dibujar</translation>
    </message>
    <message>
        <source>because</source>
        <translation>porque</translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <source>%1 at line %2 column %3</source>
        <translation>%1 en la línea %2 columna %3</translation>
    </message>
    <message>
        <source>User database could not be opened.</source>
        <translation>No se pudo abrir la base de datos del usuario.</translation>
    </message>
    <message>
        <source>The style table could not be created.</source>
        <translation>No se pudo crear la tabla de estilos.</translation>
    </message>
    <message>
        <source>The style %1 was saved to database</source>
        <translation>El estilo %1 se guardó en la base de datos</translation>
    </message>
    <message>
        <source>The style %1 was updated in the database.</source>
        <translation>El estilo %1 se actualizó en la base de datos.</translation>
    </message>
    <message>
        <source>The style %1 could not be updated in the database.</source>
        <translation>El estilo %1 no se pudo actualizar en la base de datos.</translation>
    </message>
    <message>
        <source>The style %1 could not be inserted into database.</source>
        <translation>El estilo %1 no se pudo insertar en la base de datos.</translation>
    </message>
    <message>
        <source>style not found in database</source>
        <translation>estilo no encontrado en la base de datos</translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <source>(clicked coordinate)</source>
        <translation>(coordenada pinchada)</translation>
    </message>
    <message>
        <source>WMS identify result for %1
%2</source>
        <translation>Resultado de identificación WMS para %1
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
        <translation>Error en la división</translation>
    </message>
    <message>
        <source>An error occured during feature splitting</source>
        <translation>Ocurrió un error durante la división de los objetos espaciales</translation>
    </message>
    <message>
        <source>No feature split done</source>
        <translation>No se ha dividido ningún objeto espacial</translation>
    </message>
    <message>
        <source>If there are selected features, the split tool only applies to the selected ones. If you like to split all features under the split line, clear the selection</source>
        <translation>Si hay objetos espaciales seleccionados, la herramienta de división sólo se aplica a aquellos seleccionados. Si quiere dividir todos los objetos espaciales bajo la línea de división, elimine la selección.</translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <source>Snap tolerance</source>
        <translation>Tolerancia de autoensamblado</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>No volver a mostrar este mensaje</translation>
    </message>
    <message>
        <source>Could not snap segment.</source>
        <translation>No se pudo autoensamblar el segmento.</translation>
    </message>
    <message>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>¿Ha establecido la tolerancia de autoensamblado en Configuración &gt; Propiedades del proyecto &gt; General?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <source>Name for the map file</source>
        <translation>Nombre para el archivo del mapa</translation>
    </message>
    <message>
        <source>Choose the QGIS project file</source>
        <translation>Seleccione el archivo de proyecto de QGIS</translation>
    </message>
    <message>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>Archivos de proyectos de QGIS (*.qgs);;Todos los archivos (*.*)</translation>
    </message>
    <message>
        <source>Overwrite File?</source>
        <translation>¿Sobrescribir archivo?</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation> existe. 
¿Quiere sobrescribirlo?</translation>
    </message>
    <message>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>Archivos de mapa de MapServer (*.map);;Todos los archivos (*.*)</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a fileName is prepended to this text, and appears in a dialog box</comment>
        <translation> existe. 
¿Quiere sobrescribirlo?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <source>Export to Mapserver</source>
        <translation>Exportar a MapServer</translation>
    </message>
    <message>
        <source>Map file</source>
        <translation>Archivo de mapa</translation>
    </message>
    <message>
        <source>Export LAYER information only</source>
        <translation>Exportar sólo información de la CAPA</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Ayuda</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;Aceptar</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Unidades</translation>
    </message>
    <message>
        <source>Image type</source>
        <translation>Tipo de imagen</translation>
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
        <translation>definida por el usuario</translation>
    </message>
    <message>
        <source>wbmp</source>
        <translation>wbmp</translation>
    </message>
    <message>
        <source>MinScale</source>
        <translation>Escala mínima</translation>
    </message>
    <message>
        <source>MaxScale</source>
        <translation>Escala máxima</translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation>Prefijo añadido al nombre de los archivos GIF del mapa, la barra de escala y la leyenda creados usando este archivo de mapa. Debe ser corto.</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Anchura</translation>
    </message>
    <message>
        <source>Web Interface Definition</source>
        <translation>Definición del interfaz web</translation>
    </message>
    <message>
        <source>Header</source>
        <translation>Encabezado</translation>
    </message>
    <message>
        <source>Footer</source>
        <translation>Pie de página</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Plantilla</translation>
    </message>
    <message>
        <source>Path to the MapServer template file</source>
        <translation>Ruta al archivo de plantilla de MapServer</translation>
    </message>
    <message>
        <source>dd</source>
        <translation>dd</translation>
    </message>
    <message>
        <source>feet</source>
        <translation>pies</translation>
    </message>
    <message>
        <source>meters</source>
        <translation>metros</translation>
    </message>
    <message>
        <source>miles</source>
        <translation>millas</translation>
    </message>
    <message>
        <source>inches</source>
        <translation>pulgadas</translation>
    </message>
    <message>
        <source>kilometers</source>
        <translation>kilómetros</translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation>Prefijo añadido al nombre de los archivos GIF del mapa, la barra de escala y la leyenda creados usando este archivo de mapa</translation>
    </message>
    <message>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation>Nombre para el archivo de mapa que se va a crear a partir del proyecto de QGIS</translation>
    </message>
    <message>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation>Ruta completa al proyecto de QGIS para exportar al formato de mapa de MapServer</translation>
    </message>
    <message>
        <source>QGIS project file</source>
        <translation>Archivo de proyecto de QGIS</translation>
    </message>
    <message>
        <source>If checked, only the layer information will be processed</source>
        <translation>Si se marca, sólo será procesada la información de la capa</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Guardar como...</translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <source>Measure</source>
        <translation>Medida</translation>
    </message>
    <message>
        <source>Total:</source>
        <translation>Total:</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <source>Cl&amp;ose</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>Segments</source>
        <translation>Segmentos</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <source>Segments (in meters)</source>
        <translation>Segmentos (en metros)</translation>
    </message>
    <message>
        <source>Segments (in feet)</source>
        <translation>Segmentos (en pies)</translation>
    </message>
    <message>
        <source>Segments (in degrees)</source>
        <translation>Segmentos (en grados)</translation>
    </message>
    <message>
        <source>Segments</source>
        <translation>Segmentos</translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <source>Incorrect measure results</source>
        <translation>Resultados de medida incorrectos</translation>
    </message>
    <message>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation>&lt;p&gt;Este mapa está definido con un sistema de coordenadas geográficas (latitud/longitud), pero la extensión del mapa sugiere que en realidad es un sistema de coordenadas proyectado (ej.: Mercator). Si es así, las medidas de líneas o áreas serán incorrectas.&lt;/p&gt;&lt;p&gt;Para corregir esto, establezca de forma explicita un sistema de coordenadas del mapa adecuado usando el menú &lt;tt&gt;Configuración&gt;Propiedades de proyecto&lt;/tt&gt;.</translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <source>QGIS Message</source>
        <translation>Mensaje de QGIS</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>No mostrar este mensaje de nuevo</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <source>Test connection</source>
        <translation>Probar conexión</translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>La conexión ha fallado - Compruebe la configuración y vuelva a intentarlo.

Información extensa sobre el error:
</translation>
    </message>
    <message>
        <source>Connection to %1 was successful</source>
        <translation>La conexión a %1 tuvo éxito</translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <source>Create a New PostGIS connection</source>
        <translation>Crear una nueva conexión a PostGIS</translation>
    </message>
    <message>
        <source>Connection Information</source>
        <translation>Información sobre la conexión</translation>
    </message>
    <message>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation>Buscar sólo en el esquema &quot;público&quot;</translation>
    </message>
    <message>
        <source>Only look in the geometry_columns table</source>
        <translation>Buscar sólo en la tabla de columnas de la geometría</translation>
    </message>
    <message>
        <source>Save Password</source>
        <translation>Guardar contraseña</translation>
    </message>
    <message>
        <source>Test Connect</source>
        <translation>Probar conexión</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>Host</source>
        <translation>Servidor</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>Base de datos</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>Puerto</translation>
    </message>
    <message>
        <source>Username</source>
        <translation>Nombre de usuario</translation>
    </message>
    <message>
        <source>Password</source>
        <translation>Contraseña</translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation>Nombre de la nueva conexión</translation>
    </message>
    <message>
        <source>5432</source>
        <translation>5432</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Restrict the search to the public schema for spatial tables not in the geometry_columns table</source>
        <translation>Restringir la búsqueda al esquema público de las tablas espaciales que no están en la tabla de columnas de la geometría</translation>
    </message>
    <message>
        <source>When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)</source>
        <translation>Cuando se busca en tablas espaciales que no están en las tablas de columnas de la geometría, restringir la búsqueda a tablas que están en el esquema público (en algunas bases de datos esto puede ahorrar mucho tiempo)</translation>
    </message>
    <message>
        <source>Restrict the displayed tables to those that are in the geometry_columns table</source>
        <translation>Restringir las tablas mostradas a aquellas que están en la tabla de columnas de la geometría</translation>
    </message>
    <message>
        <source>Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.</source>
        <translation>Restringe las tablas mostradas a aquellas que están en la tabla de columnas de la geometría. Esto puede acelerar la visualización inicial de las tablas espaciales.</translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <source>Name of the new connection</source>
        <translation>Nombre de la nueva conexión</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>HTTP address of the Web Map Server</source>
        <translation>Dirección HTTP del servidor de mapas web</translation>
    </message>
    <message>
        <source>Create a new WMS connection</source>
        <translation>Crear una nueva conexión WMS</translation>
    </message>
    <message>
        <source>Connection details</source>
        <translation>Detalles de la conexión</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <source>&amp;North Arrow</source>
        <translation>Flecha de &amp;Norte</translation>
    </message>
    <message>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>Crea una flecha de norte que se muestra en la vista del mapa</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Ilustraciones</translation>
    </message>
    <message>
        <source>North arrow pixmap not found</source>
        <translation>No se ha encontrado la imagen de flecha de Norte</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <source>Pixmap not found</source>
        <translation>No se ha encontrado la imagen</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <source>North Arrow Plugin</source>
        <translation>Complemento de flecha de Norte</translation>
    </message>
    <message>
        <source>Properties</source>
        <translation>Propiedades</translation>
    </message>
    <message>
        <source>Angle</source>
        <translation>Ángulo</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Ubicación</translation>
    </message>
    <message>
        <source>Set direction automatically</source>
        <translation>Establecer la dirección automáticamente</translation>
    </message>
    <message>
        <source>Enable North Arrow</source>
        <translation>Activar flecha de Norte</translation>
    </message>
    <message>
        <source>Placement on screen</source>
        <translation>Ubicación en la pantalla</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <source>Preview of north arrow</source>
        <translation>Previsualización de la flecha de Norte</translation>
    </message>
    <message>
        <source>Icon</source>
        <translation>Icono</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <source>Detected active locale on your system: </source>
        <translation>Idioma activo detectado en su sistema: </translation>
    </message>
    <message>
        <source>to vertex</source>
        <translation>a vértice</translation>
    </message>
    <message>
        <source>to segment</source>
        <translation>a segmento</translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation>a vértice y segmento</translation>
    </message>
    <message>
        <source>Semi transparent circle</source>
        <translation>Círculo semitransparente</translation>
    </message>
    <message>
        <source>Cross</source>
        <translation>Cruz</translation>
    </message>
</context>
<context>
    <name>QgsOptionsBase</name>
    <message>
        <source>QGIS Options</source>
        <translation>Opciones de QGIS</translation>
    </message>
    <message>
        <source>Hide splash screen at startup</source>
        <translation>Ocultar la pantalla de bienvenida al iniciar la aplicación</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Nota: &lt;/b&gt;Los cambios de tema tendrán efecto la próxima vez que inicie QGIS</translation>
    </message>
    <message>
        <source>&amp;Rendering</source>
        <translation>&amp;Representación</translation>
    </message>
    <message>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>La vista del mapa se actualizará después de que todos estos objetos espaciales sean leídos de la fuente de datos</translation>
    </message>
    <message>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Por omisión, las nuevas &amp;capas añadidas al mapa se deben visualizar</translation>
    </message>
    <message>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>Hacer que las líneas se muestren menos quebradas a expensas de la calidad del dibujo</translation>
    </message>
    <message>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Solucionar problemas con polígonos rellenados incorrectamente</translation>
    </message>
    <message>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Redibujar el mapa continuamente cuando se desplaza el separador entre mapa y leyenda</translation>
    </message>
    <message>
        <source>&amp;Map tools</source>
        <translation>Herramientas de &amp;mapa</translation>
    </message>
    <message>
        <source>Search radius</source>
        <translation>Radio de búsqueda</translation>
    </message>
    <message>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <source>Measure tool</source>
        <translation>Herramienta de medida</translation>
    </message>
    <message>
        <source>Panning and zooming</source>
        <translation>Desplazar y zum</translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation>Zum</translation>
    </message>
    <message>
        <source>Zoom and recenter</source>
        <translation>Zum y centrar</translation>
    </message>
    <message>
        <source>Nothing</source>
        <translation>Nada</translation>
    </message>
    <message>
        <source>Select Global Default ...</source>
        <translation>Seleccionar proyección global predeterminada...</translation>
    </message>
    <message>
        <source>&amp;General</source>
        <translation>&amp;General</translation>
    </message>
    <message>
        <source>Locale</source>
        <translation>Idioma</translation>
    </message>
    <message>
        <source>Locale to use instead</source>
        <translation>Idioma a usar en su lugar</translation>
    </message>
    <message>
        <source>Additional Info</source>
        <translation>Información adicional</translation>
    </message>
    <message>
        <source>Detected active locale on your system:</source>
        <translation>Idioma activo detectado en su sistema:</translation>
    </message>
    <message>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation>Con esta opción desactivará el &quot;hacer las líneas menos quebradas&quot;</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitalización</translation>
    </message>
    <message>
        <source>Rubberband</source>
        <translation>Banda elástica</translation>
    </message>
    <message>
        <source>Line width in pixels</source>
        <translation>Anchura de la línea en píxeles</translation>
    </message>
    <message>
        <source>Snapping</source>
        <translation>Autoensamblado</translation>
    </message>
    <message>
        <source>Zoom to mouse cursor</source>
        <translation>Zoom al cursor del ratón</translation>
    </message>
    <message>
        <source>Vertex markers</source>
        <translation>Marcadores de vértices</translation>
    </message>
    <message>
        <source>Project files</source>
        <translation>Archivos de proyecto</translation>
    </message>
    <message>
        <source>Prompt to save project changes when required</source>
        <translation>Preguntar si guardar cambios en el proyecto cuando sea necesario</translation>
    </message>
    <message>
        <source>Warn when opening a project file saved with an older version of QGIS</source>
        <translation>Avisar cuando se abra un proyecto guardado con una versión anterior de QGIS</translation>
    </message>
    <message>
        <source>Default Map Appearance (overridden by project properties)</source>
        <translation>Apariencia predeterminada del mapa (anulada por las propiedades del proyecto)</translation>
    </message>
    <message>
        <source>Selection color</source>
        <translation>Color de selección</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Color de fondo</translation>
    </message>
    <message>
        <source>&amp;Application</source>
        <translation>&amp;Aplicación</translation>
    </message>
    <message>
        <source>Icon theme</source>
        <translation>Tema de iconos</translation>
    </message>
    <message>
        <source>Capitalise layer names in legend</source>
        <translation>Comenzar el nombre de las capas con mayúsculas en la leyenda</translation>
    </message>
    <message>
        <source>Display classification attribute names in legend</source>
        <translation>Mostrar nombre de atributos de clasificación en la leyenda</translation>
    </message>
    <message>
        <source>Rendering behavior</source>
        <translation>Comportamiento de la representación</translation>
    </message>
    <message>
        <source>Number of features to draw before updating the display</source>
        <translation>Número de objetos espaciales a dibujar antes de actualizar la visualización</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Use zero to prevent display updates until all features have been rendered</source>
        <translation>&lt;b&gt;Nota:&lt;/b&gt; Usar cero para evitar actualizaciones de la visualización hasta que todos los objetos espaciales se hayan representado</translation>
    </message>
    <message>
        <source>Rendering quality</source>
        <translation>Calidad de representación</translation>
    </message>
    <message>
        <source>Zoom factor</source>
        <translation>Factor de zum</translation>
    </message>
    <message>
        <source>Mouse wheel action</source>
        <translation>Acción de la rueda del ratón</translation>
    </message>
    <message>
        <source>Rubberband color</source>
        <translation>Color de la banda de medida</translation>
    </message>
    <message>
        <source>Ellipsoid for distance calculations</source>
        <translation>Elipsoide para el cálculo de distancias</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Specify the search radius as a percentage of the map width</source>
        <translation>&lt;b&gt;Nota:&lt;/b&gt; Especifique el radio de búsqueda como porcentaje de la anchura del mapa</translation>
    </message>
    <message>
        <source>Search radius for identifying features and displaying map tips</source>
        <translation>Radio de búsqueda para identificar objetos espaciales y mostrar avisos del mapa</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation>Ancho de línea</translation>
    </message>
    <message>
        <source>Line colour</source>
        <translation>Color de línea</translation>
    </message>
    <message>
        <source>Default snap mode</source>
        <translation>Modo de autoensamblado por omisión</translation>
    </message>
    <message>
        <source>Default snapping tolerance in layer units</source>
        <translation>Tolerancia de autoensamblado predeterminada en unidades de la capa</translation>
    </message>
    <message>
        <source>Search radius for vertex edits in layer units</source>
        <translation>Radio de búsqueda para edición de vértices en unidades de la capa</translation>
    </message>
    <message>
        <source>Marker style</source>
        <translation>Estilo de marcadores</translation>
    </message>
    <message>
        <source>Override system locale</source>
        <translation>Ignorar el idioma del sistema</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Enabling / changing overide on local requires an application restart</source>
        <translation>&lt;b&gt;Nota:&lt;/b&gt; Activar / cambiar ignorar el idioma requiere reiniciar la aplicación</translation>
    </message>
    <message>
        <source>Proxy</source>
        <translation>Proxy</translation>
    </message>
    <message>
        <source>Use proxy for web access</source>
        <translation>Usar proxy para acceso web</translation>
    </message>
    <message>
        <source>Host</source>
        <translation>Servidor</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>Puerto</translation>
    </message>
    <message>
        <source>User</source>
        <translation>Usuario</translation>
    </message>
    <message>
        <source>Leave this blank if no proxy username / password are required</source>
        <translation>Dejar en blanco si no se requiere nombre de usuario/contraseña de proxy</translation>
    </message>
    <message>
        <source>Password</source>
        <translation>Contraseña</translation>
    </message>
    <message>
        <source>Open attribute table in a dock window</source>
        <translation>Abrir tabla de atributos en una ventada adosada</translation>
    </message>
    <message>
        <source>CRS</source>
        <translation>SRC</translation>
    </message>
    <message>
        <source>When layer is loaded that has no coordinate reference system (CRS)</source>
        <translation>Cuando se carga una capa que no tiene sistema de referencia de coordenadas (SRC)</translation>
    </message>
    <message>
        <source>Prompt for CRS</source>
        <translation>Preguntar SRC</translation>
    </message>
    <message>
        <source>Project wide default CRS will be used</source>
        <translation>Usar el SRC predeterminado del proyecto</translation>
    </message>
    <message>
        <source>Global default CRS displa&amp;yed below will be used</source>
        <translation>Usar el SRC global predeterminado mostrado debajo</translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <source>Paste Transformations</source>
        <translation>Pegar transformaciones</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation>&lt;b&gt;Nota: Esta función todavía no es útil&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Source</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <source>Destination</source>
        <translation>Destino</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Ayuda</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Add New Transfer</source>
        <translation>Añadir nueva transformación</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;Aceptar</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <source>&amp;Buffer features</source>
        <translation>&amp;Crear buffer de objetos espaciales</translation>
    </message>
    <message>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation>Se crea una nueva capa en la base de datos con el buffer de los objetos espaciales.</translation>
    </message>
    <message>
        <source>&amp;Geoprocessing</source>
        <translation>Geo&amp;procesamiento</translation>
    </message>
    <message>
        <source>Buffer features in layer %1</source>
        <translation>Crear buffer de objetos espaciales de la capa %1</translation>
    </message>
    <message>
        <source>Unable to add geometry column</source>
        <translation>No se puede añadir la columna de la geometría</translation>
    </message>
    <message>
        <source>Unable to add geometry column to the output table </source>
        <translation>No se ha podido añadir la columna de la geomtría a la tabla de salida </translation>
    </message>
    <message>
        <source>Unable to create table</source>
        <translation>No se puede crear la tabla</translation>
    </message>
    <message>
        <source>Failed to create the output table </source>
        <translation>No se ha podido crear la tabla de salida </translation>
    </message>
    <message>
        <source>Error connecting to the database</source>
        <translation>Error al conectarse a la base de datos</translation>
    </message>
    <message>
        <source>No GEOS support</source>
        <translation>No hay soporte para GEOS</translation>
    </message>
    <message>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation>La función buffer requiere soporte GEOS en PostGIS</translation>
    </message>
    <message>
        <source>No Active Layer</source>
        <translation>Ninguna capa activa</translation>
    </message>
    <message>
        <source>You must select a layer in the legend to buffer</source>
        <translation>Debe seleccionar una capa en la leyenda para crear el buffer</translation>
    </message>
    <message>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation>No es una capa PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation>Crear un área de influencia de una capa PostgreSQL. </translation>
    </message>
    <message>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation> no es una capa PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation>Las funciones de geoprocesamiento sólo están disponibles para capas PostgreSQL/PostGIS</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Tabla &lt;b&gt;%1&lt;/b&gt; en la base de datos &lt;b&gt;%2&lt;/b&gt; en el servidor &lt;b&gt;%3&lt;/b&gt;, usuario &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Connection Failed</source>
        <translation>Ha fallado la conexión</translation>
    </message>
    <message>
        <source>Connection to the database failed:</source>
        <translation>Ha fallado la conexión a la base de datos:</translation>
    </message>
    <message>
        <source>Database error</source>
        <translation>Error de la base de datos</translation>
    </message>
    <message>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;No se pudieron obtener muestras de los valores de los campos utilizando SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <source>Query Result</source>
        <translation>Resultados de la consulta</translation>
    </message>
    <message>
        <source>The where clause returned </source>
        <translation>La cláusula &quot;donde&quot; (WHERE) devolvió </translation>
    </message>
    <message>
        <source> rows.</source>
        <translation> filas.</translation>
    </message>
    <message>
        <source>Query Failed</source>
        <translation>Ha fallado la consulta</translation>
    </message>
    <message>
        <source>An error occurred when executing the query:</source>
        <translation>Se ha producido un error mientras se ejecutaba la consulta:</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation>Sin registros</translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>La consulta que ha especificado devuelve cero registros. Las capas válidas de PostgreSQL deben tener al menos un objeto espacial.</translation>
    </message>
    <message>
        <source>No Query</source>
        <translation>Ninguna consulta</translation>
    </message>
    <message>
        <source>You must create a query before you can test it</source>
        <translation>Debe crear una consulta antes de probarlo</translation>
    </message>
    <message>
        <source>Error in Query</source>
        <translation>Error en la consulta</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <source>PostgreSQL Query Builder</source>
        <translation>Constructor de consultas de PostgreSQL</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Limpiar</translation>
    </message>
    <message>
        <source>Test</source>
        <translation>Probar</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>Values</source>
        <translation>Valores</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Todos</translation>
    </message>
    <message>
        <source>Sample</source>
        <translation>Muestra</translation>
    </message>
    <message>
        <source>Fields</source>
        <translation>Campos</translation>
    </message>
    <message>
        <source>Operators</source>
        <translation>Operadores</translation>
    </message>
    <message>
        <source>=</source>
        <translation>=</translation>
    </message>
    <message>
        <source>IN</source>
        <translation>EN</translation>
    </message>
    <message>
        <source>NOT IN</source>
        <translation>NO EN</translation>
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
        <translation>COMO</translation>
    </message>
    <message>
        <source>AND</source>
        <translation>Y</translation>
    </message>
    <message>
        <source>ILIKE</source>
        <translation>DISTINTO DE</translation>
    </message>
    <message>
        <source>OR</source>
        <translation>O</translation>
    </message>
    <message>
        <source>NOT</source>
        <translation>NO</translation>
    </message>
    <message>
        <source>SQL where clause</source>
        <translation>Cláusula &quot;donde&quot; (WHERE) de SQL</translation>
    </message>
    <message>
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
        <source>Datasource</source>
        <translation>Origen de datos</translation>
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
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error reading repository:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>all repositories</source>
        <translation type="unfinished">todos lo repositorios</translation>
    </message>
    <message>
        <source>connected</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished">Instalar/actualizar complemento</translation>
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
        <translation>Instalador de complementos de Python de QGIS</translation>
    </message>
    <message>
        <source>QGIS Plugin Installer</source>
        <translation type="obsolete">Instalador de complementos de QGIS</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation>Complementos</translation>
    </message>
    <message>
        <source>List of available and installed plugins</source>
        <translation>Lista de complementos disponibles e instalados</translation>
    </message>
    <message>
        <source>Filter:</source>
        <translation>Filtrar:</translation>
    </message>
    <message>
        <source>Display only plugins containing this word in their metadata</source>
        <translation>Mostrar sólo complementos que contengan esta palabra en sus metadatos</translation>
    </message>
    <message>
        <source>Display only plugins from given repository</source>
        <translation>Mostrar sólo complementos de un repositorio dado</translation>
    </message>
    <message>
        <source>all repositories</source>
        <translation>todos lo repositorios</translation>
    </message>
    <message>
        <source>Display only plugins with matching status</source>
        <translation>Mostrar sólo complementos con estado concordante</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Estado</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Versión</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Descripción</translation>
    </message>
    <message>
        <source>Author</source>
        <translation>Autor</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation>Repositorio</translation>
    </message>
    <message>
        <source>Install, reinstall or upgrade the selected plugin</source>
        <translation>Instalar, reinstalar o actualizar el complemento seleccionado</translation>
    </message>
    <message>
        <source>Install/upgrade plugin</source>
        <translation>Instalar/actualizar complemento</translation>
    </message>
    <message>
        <source>Uninstall the selected plugin</source>
        <translation>Desinstalar el complemento seleccionado</translation>
    </message>
    <message>
        <source>Uninstall plugin</source>
        <translation>Desinstalar complemento</translation>
    </message>
    <message>
        <source>Repositories</source>
        <translation>Repositorios</translation>
    </message>
    <message>
        <source>List of plugin repositories</source>
        <translation>Lista de repositorios de complementos</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Allow the Installer to look for updates and news in enabled repositories on QGIS startup</source>
        <translation>Permitir al instalador buscar actualizaciones y nuevos en los repositorios activos al inicio de QGIS</translation>
    </message>
    <message>
        <source>Check for updates on startup</source>
        <translation>Comprobar actualizaciones al inicio</translation>
    </message>
    <message>
        <source>Add third party plugin repositories to the list</source>
        <translation>Añadir repositorios de terceros a la lista</translation>
    </message>
    <message>
        <source>Add 3rd party repositories</source>
        <translation>Añadir repositorios de terceros</translation>
    </message>
    <message>
        <source>Add a new plugin repository</source>
        <translation>Añadir un nuevo repositorio de complementos</translation>
    </message>
    <message>
        <source>Add...</source>
        <translation>Añadir...</translation>
    </message>
    <message>
        <source>Edit the selected repository</source>
        <translation>Editar el repositorio seleccionado</translation>
    </message>
    <message>
        <source>Edit...</source>
        <translation>Editar...</translation>
    </message>
    <message>
        <source>Remove the selected repository</source>
        <translation>Eliminar el repositorio seleccionado</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <source>The plugins will be installed to ~/.qgis/python/plugins</source>
        <translation>El complemento se instalará en ~/.qgis/python/plugins</translation>
    </message>
    <message>
        <source>Close the Installer window</source>
        <translation>Cerrar la ventana del instalador</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
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
        <translation type="unfinished">Conectando...</translation>
    </message>
    <message>
        <source>Host connected. Sending request...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation type="unfinished">Descargando datos...</translation>
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
        <translation type="unfinished">Error</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerFetchingDialogBase</name>
    <message>
        <source>Fetching repositories</source>
        <translation>Recopilando repositorios</translation>
    </message>
    <message>
        <source>Overall progress:</source>
        <translation>Progreso global:</translation>
    </message>
    <message>
        <source>Abort fetching</source>
        <translation>Cancelar la recopilación</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation>Repositorio</translation>
    </message>
    <message>
        <source>State</source>
        <translation>Estado</translation>
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
        <translation type="unfinished">Conectando...</translation>
    </message>
    <message>
        <source>Host connected. Sending request...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation type="unfinished">Descargando datos...</translation>
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
        <translation type="unfinished">Error</translation>
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
        <translation>Instaldor de complementos de Python de QGIS</translation>
    </message>
    <message>
        <source>Installing plugin:</source>
        <translation>Instalando complemento:</translation>
    </message>
    <message>
        <source>Connecting...</source>
        <translation>Conectando...</translation>
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
        <translation>Error al cargar el complemento</translation>
    </message>
    <message>
        <source>The plugin seems to be invalid or have unfulfilled dependencies. It has been installed, but can&apos;t be loaded. If you really need this plugin, you can contact its author or &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;QGIS users group&lt;/a&gt; and try to solve the problem. If not, you can just uninstall it. Here is the error message below:</source>
        <translation>El complemento parece no ser válido o tiene dependencias insatisfechas. Se ha instalado, pero no se puede cargar. Si realmente lo necesita, puede contactar con el autor o &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;el grupo se usuarios de QGIS&lt;/a&gt; e intantar solucionar el problema. Si no, puede simplemente desinstalarlo. A continuación está el mensaje de error:</translation>
    </message>
    <message>
        <source>Do you want to uninstall this plugin now? If you&apos;re unsure, probably you would like to do this.</source>
        <translation>¿Quiere desinstalar este complemento ahora? Si no está seguro, probablemente quiera hacerlo.</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerRepositoryDetailsDialogBase</name>
    <message>
        <source>Repository details</source>
        <translation>Detalles del repositorio</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Nombre:</translation>
    </message>
    <message>
        <source>Enter a name for the repository</source>
        <translation>Introduzca un nombre para el repositorio</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <source>Enter the repository URL, beginning with &quot;http://&quot;</source>
        <translation>Introduzca la URL del repositorio, comenzando con &quot;http://&quot;</translation>
    </message>
    <message>
        <source>Enable or disable the repository (disabled repositories will be omitted)</source>
        <translation>Activar o desactivar el repositorio (los repositorios desactivados se omitirán)</translation>
    </message>
    <message>
        <source>Enabled</source>
        <translation>Activado</translation>
    </message>
    <message>
        <source>[place for a warning message]</source>
        <translation type="obsolete">[sitio para un mensaje de aviso]</translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <source>No Plugins</source>
        <translation>No hay complementos</translation>
    </message>
    <message>
        <source>No QGIS plugins found in </source>
        <translation>No se han econtrado complementos de QGIS en </translation>
    </message>
    <message>
        <source>&amp;Select All</source>
        <translation>&amp;Seleccionar todos</translation>
    </message>
    <message>
        <source>&amp;Clear All</source>
        <translation>&amp;Limpiar todos</translation>
    </message>
    <message>
        <source>[ incompatible ]</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <source>QGIS Plugin Manager</source>
        <translation>Administrador de complementos de QGIS</translation>
    </message>
    <message>
        <source>To enable / disable a plugin, click its checkbox or description</source>
        <translation>Para activar /desactivar un complemento, pulse en su casilla o descripción</translation>
    </message>
    <message>
        <source>&amp;Filter</source>
        <translation>&amp;Filtrar</translation>
    </message>
    <message>
        <source>Plugin Directory:</source>
        <translation>Directorio de complementos:</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation>Directorio</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <source>Zoom In</source>
        <translation>Acercar zum</translation>
    </message>
    <message>
        <source>z</source>
        <translation>z</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Alejar zum</translation>
    </message>
    <message>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <source>Zoom To Layer</source>
        <translation>Zum a la capa</translation>
    </message>
    <message>
        <source>Zoom to Layer</source>
        <translation>Hace zum a la capa</translation>
    </message>
    <message>
        <source>Pan Map</source>
        <translation>Mover mapa</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation>Mueve el mapa</translation>
    </message>
    <message>
        <source>Add Point</source>
        <translation>Añadir punto</translation>
    </message>
    <message>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <source>Capture Points</source>
        <translation>Capturar puntos</translation>
    </message>
    <message>
        <source>Delete Point</source>
        <translation>Borrar punto</translation>
    </message>
    <message>
        <source>Delete Selected</source>
        <translation>Borrar lo seleccionado</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Lineal</translation>
    </message>
    <message>
        <source>Helmert</source>
        <translation>Helmert</translation>
    </message>
    <message>
        <source>Choose a name for the world file</source>
        <translation>Seleccionar un nombre para el archivo de referenciación (world file)</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atención</translation>
    </message>
    <message>
        <source>Affine</source>
        <translation>Afinidad</translation>
    </message>
    <message>
        <source>Not implemented!</source>
        <translation>Sin implementar</translation>
    </message>
    <message>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Una transformación por afinidad requiere cambiar el archivo ráster original. Esto todavía no está soportado.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt; La transformación </translation>
    </message>
    <message>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation> aún no está soportada.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <source>Could not write to </source>
        <translation>No se pudo escribir </translation>
    </message>
    <message>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>Actualmente todos los archivos modificados se escribirán en formato TIFF.</translation>
    </message>
    <message>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Una transformacioń Helmert requiere modificaciones en la capa ráster.&lt;/p&gt;&lt;p&gt;El ráster modificado se guardará en un nuevo archivo y se generará un archivo de referenciación para dicho archivo.&lt;/p&gt;&lt;p&gt;¿Está seguro de que es eso lo que quiere?&lt;/p&gt;</translation>
    </message>
    <message>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given file name</comment>
        <translation>-modificado</translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <source>Add points</source>
        <translation>Añadir puntos</translation>
    </message>
    <message>
        <source>Delete points</source>
        <translation>Borrar puntos</translation>
    </message>
    <message>
        <source>Zoom in</source>
        <translation>Acercar zum</translation>
    </message>
    <message>
        <source>Zoom out</source>
        <translation>Alejar zum</translation>
    </message>
    <message>
        <source>Zoom to the raster extents</source>
        <translation>Zum a la extensión del ráster</translation>
    </message>
    <message>
        <source>Pan</source>
        <translation>Mover</translation>
    </message>
    <message>
        <source>Modified raster:</source>
        <translation>Raster modificado:</translation>
    </message>
    <message>
        <source>World file:</source>
        <translation>Archivo de referenciación (world file):</translation>
    </message>
    <message>
        <source>Transform type:</source>
        <translation>Tipo de transformación:</translation>
    </message>
    <message>
        <source>Reference points</source>
        <translation>Puntos de referencia</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Create</source>
        <translation>Crear</translation>
    </message>
    <message>
        <source>Create and load layer</source>
        <translation>Crear y cargar capa</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <source>Unable to access relation</source>
        <translation>No se puede acceder a la relación</translation>
    </message>
    <message>
        <source>Unable to access the </source>
        <translation>No se puede acceder a la relación </translation>
    </message>
    <message>
        <source> relation.
The error message from the database was:
</source>
        <translation> relación.
El mensaje de error de la base de datos fue:
</translation>
    </message>
    <message>
        <source>No suitable key column in table</source>
        <translation>No hay una columna de clave adecuada</translation>
    </message>
    <message>
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
        <source>The unique index on column</source>
        <translation>El índice único en la columna</translation>
    </message>
    <message>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation>no es adecuado porque actualmente Qgis no admite columnas de tipo no-int4 como clave de la tabla.
</translation>
    </message>
    <message>
        <source>and </source>
        <translation>y </translation>
    </message>
    <message>
        <source>The unique index based on columns </source>
        <translation>El índice único basado en columnas </translation>
    </message>
    <message>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation> no es adecuado porque actualmente Qgis no admite columnas múltiples como clave de la tabla.</translation>
    </message>
    <message>
        <source>Unable to find a key column</source>
        <translation>No se puede encontrar una columna de clave</translation>
    </message>
    <message>
        <source> derives from </source>
        <translation> deriva de </translation>
    </message>
    <message>
        <source>and is suitable.</source>
        <translation>y es adecuada.</translation>
    </message>
    <message>
        <source>and is not suitable </source>
        <translation>y no es adecuada </translation>
    </message>
    <message>
        <source>type is </source>
        <translation>es de tipo </translation>
    </message>
    <message>
        <source> and has a suitable constraint)</source>
        <translation> y tiene la reserva adecuada)</translation>
    </message>
    <message>
        <source> and does not have a suitable constraint)</source>
        <translation> y no tiene la reserva adecuada)</translation>
    </message>
    <message>
        <source>Note: </source>
        <translation>Nota: </translation>
    </message>
    <message>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation>inicialmente parecía adecuada pero no contiene datos únicos, por lo que no es aceptable.
</translation>
    </message>
    <message>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation>La vista que ha seleccionado contiene las siguientes columnas, ninguna de las cuales satisface las condiciones anteriores:</translation>
    </message>
    <message>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation>Qgis necesita que la vista tenga una columna que pueda ser utilizada como clave única. Esta columna debería proceder de una columna de una tabla de tipo int4 y ser una clave primaria, tener una reserva única o ser una columna oid de PostgreSQL. Para un mejor funcionamiento la columna también debería estar indexada.</translation>
    </message>
    <message>
        <source>The view </source>
        <translation>La vista </translation>
    </message>
    <message>
        <source>has no column suitable for use as a unique key.
</source>
        <translation>no tiene columnas adecuadas para usar como clave única.
</translation>
    </message>
    <message>
        <source>No suitable key column in view</source>
        <translation>No hay una columna de clave adecuada en la vista</translation>
    </message>
    <message>
        <source>Unknown geometry type</source>
        <translation>Tipo de geometría desconocido</translation>
    </message>
    <message>
        <source>Column </source>
        <translation>Columna </translation>
    </message>
    <message>
        <source> in </source>
        <translation> en </translation>
    </message>
    <message>
        <source> has a geometry type of </source>
        <translation> tiene una geometría de tipo </translation>
    </message>
    <message>
        <source>, which Qgis does not currently support.</source>
        <translation>, que Qgis no soporta actualmente.</translation>
    </message>
    <message>
        <source>. The database communication log was:
</source>
        <translation>. La registro de comunicación de la base de datos fue:
</translation>
    </message>
    <message>
        <source>Unable to get feature type and srid</source>
        <translation>No se ha podido obtener el tipo ni el srid del objeto espacial</translation>
    </message>
    <message>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation>QGIS no pude determinar el tipo y srid de la columna </translation>
    </message>
    <message>
        <source>Unable to determine table access privileges for the </source>
        <translation>No se pueden determinar los privilegios de acceso a tabla para el </translation>
    </message>
    <message>
        <source>Error while adding features</source>
        <translation>Error al añadir objetos espaciales</translation>
    </message>
    <message>
        <source>Error while deleting features</source>
        <translation>Error al borrar objetos espaciales</translation>
    </message>
    <message>
        <source>Error while adding attributes</source>
        <translation>Error al añadir atributos</translation>
    </message>
    <message>
        <source>Error while deleting attributes</source>
        <translation>Error al borrar atributos</translation>
    </message>
    <message>
        <source>Error while changing attributes</source>
        <translation>Error al cambiar atributos</translation>
    </message>
    <message>
        <source>unexpected PostgreSQL error</source>
        <translation>error inesperado de PostgreSQL</translation>
    </message>
    <message>
        <source>Error while changing geometry values</source>
        <translation>Error al cambiar los valores de la geometría</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider::Conn</name>
    <message>
        <source>No GEOS Support!</source>
        <translation>GEOS no está implementado</translation>
    </message>
    <message>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>Su instalación de PostGIS no admite GEOS.
La selección e identificación de objetos espaciales no funcionarán correctamente.
Instale PostGIS con capacidad para GEOS (http://geos.refractions.net)
</translation>
    </message>
</context>
<context>
    <name>QgsProjectPropertiesBase</name>
    <message>
        <source>Project Properties</source>
        <translation>Propiedades del proyecto</translation>
    </message>
    <message>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <source>Default project title</source>
        <translation>Título del proyecto por omisión</translation>
    </message>
    <message>
        <source>Descriptive project name</source>
        <translation>Nombre descriptivo del proyecto</translation>
    </message>
    <message>
        <source>Meters</source>
        <translation>Metros</translation>
    </message>
    <message>
        <source>Feet</source>
        <translation>Pies</translation>
    </message>
    <message>
        <source>Decimal degrees</source>
        <translation>Grados decimales</translation>
    </message>
    <message>
        <source>Precision</source>
        <translation>Precisión</translation>
    </message>
    <message>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>Establece automáticamente el número de decimales en la visualización en la posición del ratón</translation>
    </message>
    <message>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>El número de decimales usado cuando se visualiza en la posición del ratón se establece automaticamente de manera que un movimiento del ratón de un solo píxel cambia la posición de la visualización</translation>
    </message>
    <message>
        <source>Automatic</source>
        <translation>Automática</translation>
    </message>
    <message>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Estable el número de decimales a usar para la visualización en la posición del ratón</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation>Manual</translation>
    </message>
    <message>
        <source>The number of decimal places for the manual option</source>
        <translation>Número de decimales para la opción manual</translation>
    </message>
    <message>
        <source>decimal places</source>
        <translation>lugares decimales</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitalización</translation>
    </message>
    <message>
        <source>Enable topological editing</source>
        <translation>Activar edición topológica</translation>
    </message>
    <message>
        <source>Snapping options...</source>
        <translation>Opciones de autoensamblado...</translation>
    </message>
    <message>
        <source>Avoid intersections of new polygons</source>
        <translation>Evitar intersecciones de nuevos polígonos</translation>
    </message>
    <message>
        <source>Title and colors</source>
        <translation>Título y colores</translation>
    </message>
    <message>
        <source>Project title</source>
        <translation>Título del proyecto</translation>
    </message>
    <message>
        <source>Selection color</source>
        <translation>Color de selección</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Color de fondo</translation>
    </message>
    <message>
        <source>Map units</source>
        <translation>Unidades del mapa</translation>
    </message>
    <message>
        <source>Coordinate Reference System (CRS)</source>
        <translation>Sistema de referencia de coordenadas (SRC)</translation>
    </message>
    <message>
        <source>Enable &apos;on the fly&apos; CRS transformation</source>
        <translation>Activar transformación de SRC al vuelo</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelector</name>
    <message>
        <source>User Defined Coordinate Systems</source>
        <translation>Sistemas de coordenadas definidos por el usuario</translation>
    </message>
    <message>
        <source>Geographic Coordinate Systems</source>
        <translation>Sistemas de coordenadas geográficas</translation>
    </message>
    <message>
        <source>Projected Coordinate Systems</source>
        <translation>Sistemas de coordenadas proyectadas</translation>
    </message>
    <message>
        <source>Resource Location Error</source>
        <translation>Error en la localización de recursos</translation>
    </message>
    <message>
        <source>Error reading database file from: 
 %1
Because of this the projection selector will not work...</source>
        <translation>Error al leer el archivo de base de datos de: 
 %1
Debido a esto no funcionará el selector de proyecciones...</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelectorBase</name>
    <message>
        <source>Search</source>
        <translation>Buscar</translation>
    </message>
    <message>
        <source>Find</source>
        <translation>Encontrar</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>EPSG ID</source>
        <translation>EPSG ID</translation>
    </message>
    <message>
        <source>Coordinate Reference System Selector</source>
        <translation>Selector de sistema de referencia de coordenadas</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Sistema de referencia de coordenadas</translation>
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
        <translation>Consola de Python</translation>
    </message>
    <message>
        <source>&gt;&gt;&gt;</source>
        <translation>&gt;&gt;&gt;</translation>
    </message>
    <message>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation>Para acceder al entorno de Quantum GIS desde esta consola de python use objetos de ámbito global, lo que es una instancia de la clase QgisInterface.&lt;br&gt;Ejemplo de uso: iface.zoomFull()</translation>
    </message>
</context>
<context>
    <name>QgsQuickPrint</name>
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
        <source> miles</source>
        <translation> millas</translation>
    </message>
    <message>
        <source> mile</source>
        <translation> milla</translation>
    </message>
    <message>
        <source> inches</source>
        <translation> pulgadas</translation>
    </message>
    <message>
        <source> foot</source>
        <translation> pie</translation>
    </message>
    <message>
        <source> feet</source>
        <translation> pies</translation>
    </message>
    <message>
        <source> degree</source>
        <translation> grado</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation> grados</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation> desconocido</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <source>and all other files</source>
        <translation>y todos los demás archivos</translation>
    </message>
    <message>
        <source>Not Set</source>
        <translation>No establecido</translation>
    </message>
    <message>
        <source>Driver:</source>
        <translation>Controlador:</translation>
    </message>
    <message>
        <source>Dataset Description</source>
        <translation>Descripción del conjunto de datos</translation>
    </message>
    <message>
        <source>Dimensions:</source>
        <translation>Dimensiones:</translation>
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
        <translation> Bandas: </translation>
    </message>
    <message>
        <source>No Data Value</source>
        <translation>Valor sin datos</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Tipo de datos:</translation>
    </message>
    <message>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte - Entero natural de 8 bits</translation>
    </message>
    <message>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 - Entero natural de 16 bits </translation>
    </message>
    <message>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 - Número entero de 16 bits </translation>
    </message>
    <message>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 - Entero natural de 32 bits </translation>
    </message>
    <message>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 - Número entero de 32 bits </translation>
    </message>
    <message>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 - Número de coma flotante de 32 bits </translation>
    </message>
    <message>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 - Número de coma flotante de 64 bits </translation>
    </message>
    <message>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 - Número complejo Int16 </translation>
    </message>
    <message>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 - Número complejo Int32 </translation>
    </message>
    <message>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 - Número conplejo Float32 </translation>
    </message>
    <message>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 - Número complejo Float64 </translation>
    </message>
    <message>
        <source>Could not determine raster data type.</source>
        <translation>No se pudo determinar el tipo de datos ráster.</translation>
    </message>
    <message>
        <source>Pyramid overviews:</source>
        <translation>Pirámides:</translation>
    </message>
    <message>
        <source>Layer Spatial Reference System: </source>
        <translation>Sistema de referencia espacial de la capa: </translation>
    </message>
    <message>
        <source>Origin:</source>
        <translation>Origen:</translation>
    </message>
    <message>
        <source>Pixel Size:</source>
        <translation>Tamaño de píxel:</translation>
    </message>
    <message>
        <source>Band</source>
        <translation>Banda</translation>
    </message>
    <message>
        <source>Band No</source>
        <translation>Número de banda</translation>
    </message>
    <message>
        <source>No Stats</source>
        <translation>No hay estadísticas</translation>
    </message>
    <message>
        <source>No stats collected yet</source>
        <translation>Todavía no se han recogido estadísticas</translation>
    </message>
    <message>
        <source>Min Val</source>
        <translation>Valor mínimo</translation>
    </message>
    <message>
        <source>Max Val</source>
        <translation>Valor máximo</translation>
    </message>
    <message>
        <source>Range</source>
        <translation>Intervalo</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Media</translation>
    </message>
    <message>
        <source>Sum of squares</source>
        <translation>Suma de cuadrados</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Desviación estándar</translation>
    </message>
    <message>
        <source>Sum of all cells</source>
        <translation>Suma de todas las celdas</translation>
    </message>
    <message>
        <source>Cell Count</source>
        <translation>Número de celdas</translation>
    </message>
    <message>
        <source>Average Magphase</source>
        <translation>Fase magnética (Magphase) media</translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Media</translation>
    </message>
    <message>
        <source>out of extent</source>
        <translation>fuera de la extensión</translation>
    </message>
    <message>
        <source>null (no data)</source>
        <translation>nulo (sin datos)</translation>
    </message>
    <message>
        <source>NoDataValue not set</source>
        <translation>No se ha establecido el valor sin datos (NoDataValue)</translation>
    </message>
    <message>
        <source>Band %1</source>
        <translation>Banda %1</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <source>Grayscale</source>
        <translation>Escala de grises</translation>
    </message>
    <message>
        <source>Pseudocolor</source>
        <translation>Pseudocolor</translation>
    </message>
    <message>
        <source>Freak Out</source>
        <translation>Alucinante</translation>
    </message>
    <message>
        <source>Columns: </source>
        <translation>Columnas: </translation>
    </message>
    <message>
        <source>Rows: </source>
        <translation>Filas: </translation>
    </message>
    <message>
        <source>No-Data Value: </source>
        <translation>Valor sin datos: </translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>n/d</translation>
    </message>
    <message>
        <source>Write access denied</source>
        <translation>Acceso de escritura denegado</translation>
    </message>
    <message>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation>Acceso de escritura denegado. Ajuste los permisos del archivo e inténtelo de nuevo.

</translation>
    </message>
    <message>
        <source>Building pyramids failed.</source>
        <translation>Ha fallado la construcción de pirámides.</translation>
    </message>
    <message>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation>La creación de pirámides no es soportada en este tipo de ráster.</translation>
    </message>
    <message>
        <source>No Stretch</source>
        <translation>Sin estiramiento</translation>
    </message>
    <message>
        <source>Stretch To MinMax</source>
        <translation>Estirar a MinMax</translation>
    </message>
    <message>
        <source>Stretch And Clip To MinMax</source>
        <translation>Estirar y unir a MinMax</translation>
    </message>
    <message>
        <source>Clip To MinMax</source>
        <translation>Unir a MinMax</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Discreto</translation>
    </message>
    <message>
        <source>Equal interval</source>
        <translation>Intervalo igual</translation>
    </message>
    <message>
        <source>Quantiles</source>
        <translation>Cuantiles</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Descripción</translation>
    </message>
    <message>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation>Las capas ráster de elevada resolución pueden ralentizar la navegación en QGIS.</translation>
    </message>
    <message>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation>Creando copias de menor resolución de los datos (pirámides) se puede mejorar el rendimiento de forma considerable, ya que QGIS selecciona la resolución más adecuada dependiendo del nivel de zum.</translation>
    </message>
    <message>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation>Debe tener permiso de escritura en el directorio donde están almacenados los datos originales para construir las pirámides.</translation>
    </message>
    <message>
        <source>Red</source>
        <translation>Rojo</translation>
    </message>
    <message>
        <source>Green</source>
        <translation>Verde</translation>
    </message>
    <message>
        <source>Blue</source>
        <translation>Azul</translation>
    </message>
    <message>
        <source>Percent Transparent</source>
        <translation>Porcentaje transparente</translation>
    </message>
    <message>
        <source>Gray</source>
        <translation>Gris</translation>
    </message>
    <message>
        <source>Indexed Value</source>
        <translation>Valor indexado</translation>
    </message>
    <message>
        <source>User Defined</source>
        <translation>Definido por el usuario</translation>
    </message>
    <message>
        <source>No-Data Value: Not Set</source>
        <translation>Valor sin datos: no establecido</translation>
    </message>
    <message>
        <source>Save file</source>
        <translation>Guardar archivo</translation>
    </message>
    <message>
        <source>Textfile (*.txt)</source>
        <translation>Archivo de texto (*.txt)</translation>
    </message>
    <message>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation>Archivo de exportación de valores de píxel transparente generado por QGIS</translation>
    </message>
    <message>
        <source>Open file</source>
        <translation>Abrir archivo</translation>
    </message>
    <message>
        <source>Import Error</source>
        <translation>Error de importación</translation>
    </message>
    <message>
        <source>The following lines contained errors

</source>
        <translation>Las siguientes líneas contenían errores</translation>
    </message>
    <message>
        <source>Read access denied</source>
        <translation>Acceso de lectura denegado</translation>
    </message>
    <message>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation>Acceso de lectura denegado. Ajuste los permisos de archivo y pruebe de nuevo.</translation>
    </message>
    <message>
        <source>Color Ramp</source>
        <translation>Rampa de colores</translation>
    </message>
    <message>
        <source>Not Set</source>
        <translation>No establecido</translation>
    </message>
    <message>
        <source>Default Style</source>
        <translation>Estilo predeterminado</translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Archivo de estilo de capa de QGIS (*.qml)</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation>Formato de estilo desconocido: </translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation>Mapa de color</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Lineal</translation>
    </message>
    <message>
        <source>Exact</source>
        <translation>Exacto</translation>
    </message>
    <message>
        <source>Please note that building internal pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation>¡Por favor, tenga en cuenta que la construcción de pirámides internas puede alterar el archivo de datos original y una vez creadas no se pueden eliminar!</translation>
    </message>
    <message>
        <source>Please note that building internal pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation>¡Por favor, tenga en cuenta que la construcción de pirámides internas puede estropear su imagen - haga siempre una copia de seguridad antes!</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>Predeterminado</translation>
    </message>
    <message>
        <source>The file was not writeable. Some formats do not support pyramid overviews. Consult the GDAL documentation if in doubt.</source>
        <translation>El archivo no se puede escribir. Algunos formatos no admiten vistas de pirámides. Consulta la documentación de GDAL en caso de duda.</translation>
    </message>
    <message>
        <source>Custom color map entry</source>
        <translation>Entrada de mapa de colores personalizado</translation>
    </message>
    <message>
        <source>QGIS Generated Color Map Export File</source>
        <translation>Archivo de exportación de mapa de colores generado por QGIS</translation>
    </message>
    <message>
        <source>Load Color Map</source>
        <translation>Cargar mapa de colores</translation>
    </message>
    <message>
        <source>Saved Style</source>
        <translation>Estilo guardado</translation>
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
        <translation>Propiedades de la capa ráster</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Simbología</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Total&lt;/p&gt;</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Nada</translation>
    </message>
    <message>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <source>Thumbnail</source>
        <translation>Miniatura</translation>
    </message>
    <message>
        <source>Columns:</source>
        <translation>Columnas:</translation>
    </message>
    <message>
        <source>Rows:</source>
        <translation>Filas:</translation>
    </message>
    <message>
        <source>No Data:</source>
        <translation>Sin datos:</translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Escala máxima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Escala mínima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <source>Metadata</source>
        <translation>Metadatos</translation>
    </message>
    <message>
        <source>Pyramids</source>
        <translation>Pirámides</translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Media</translation>
    </message>
    <message>
        <source>Nearest Neighbour</source>
        <translation>Vecino más próximo</translation>
    </message>
    <message>
        <source>Histogram</source>
        <translation>Histograma</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opciones</translation>
    </message>
    <message>
        <source>Chart Type</source>
        <translation>Tipo de gráfico</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Actualizar</translation>
    </message>
    <message>
        <source>Max</source>
        <translation>Máx</translation>
    </message>
    <message>
        <source>Min</source>
        <translation>Mín</translation>
    </message>
    <message>
        <source> 00%</source>
        <translation> 00%</translation>
    </message>
    <message>
        <source>Render as</source>
        <translation>Representar como</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation>Mapa de color</translation>
    </message>
    <message>
        <source>Delete entry</source>
        <translation>Borrar entrada</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Clasificar</translation>
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
        <source>Estimate (faster)</source>
        <translation>Estimar (más rápido)</translation>
    </message>
    <message>
        <source>Load</source>
        <translation>Cargar</translation>
    </message>
    <message>
        <source>Actual (slower)</source>
        <translation>Real (más lento)</translation>
    </message>
    <message>
        <source>Current</source>
        <translation>Actual</translation>
    </message>
    <message>
        <source>Save current contrast enhancement algorithm as default. This setting will be persistent between QGIS sessions.</source>
        <translation>Guardar el algoritmo actual de mejora del contraste como predeterminado. Esta configuración se mantendrá entre sesiones de QGIS.</translation>
    </message>
    <message>
        <source>Saves current contrast enhancement algorithm as a default. This setting will be persistent between QGIS sessions.</source>
        <translation>Guarda la configuración actual del algoritmo de mejora del contraste como predeterminado. Esta configuración se mantendrá entre sesiones de QGIS.</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>Predeterminado</translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation>EtiquetaDeTexto</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Transparencia</translation>
    </message>
    <message>
        <source>Single band gray</source>
        <translation>Gris de una banda</translation>
    </message>
    <message>
        <source>Three band color</source>
        <translation>Color de tres bandas</translation>
    </message>
    <message>
        <source>RGB mode band selection and scaling</source>
        <translation>Selección de banda del modo RGB y escalado</translation>
    </message>
    <message>
        <source>Red band</source>
        <translation>Banda roja</translation>
    </message>
    <message>
        <source>Green band</source>
        <translation>Banda verde</translation>
    </message>
    <message>
        <source>Blue band</source>
        <translation>Banda azul</translation>
    </message>
    <message>
        <source>Custom min / max values</source>
        <translation>Valores mínimo / máximo personalizados</translation>
    </message>
    <message>
        <source>Red min</source>
        <translation>Mínimo de rojo</translation>
    </message>
    <message>
        <source>Red max</source>
        <translation>Máximo de rojo</translation>
    </message>
    <message>
        <source>Green min</source>
        <translation>Mínimo de verde</translation>
    </message>
    <message>
        <source>Green max</source>
        <translation>Máximo de verde</translation>
    </message>
    <message>
        <source>Blue min</source>
        <translation>Mínimo de azul</translation>
    </message>
    <message>
        <source>Blue max</source>
        <translation>Máximo de azul</translation>
    </message>
    <message>
        <source>Single band properties</source>
        <translation>Propiedades de banda única</translation>
    </message>
    <message>
        <source>Gray band</source>
        <translation>Banda gris</translation>
    </message>
    <message>
        <source>Color map</source>
        <translation>Mapa de color</translation>
    </message>
    <message>
        <source>Invert color map</source>
        <translation>Invertir mapa de color</translation>
    </message>
    <message>
        <source>Use standard deviation</source>
        <translation>Usar desviación estándar</translation>
    </message>
    <message>
        <source>Load min / max values from band</source>
        <translation>Cargar valores min / max de la banda</translation>
    </message>
    <message>
        <source>Contrast enhancement</source>
        <translation>Mejora de contraste</translation>
    </message>
    <message>
        <source>Global transparency</source>
        <translation>Transparencia global</translation>
    </message>
    <message>
        <source>No data value</source>
        <translation>Valor de sin datos</translation>
    </message>
    <message>
        <source>Reset no data value</source>
        <translation>Restablecer valor de sin datos</translation>
    </message>
    <message>
        <source>Custom transparency options</source>
        <translation>Opciones de transparencia personalizada</translation>
    </message>
    <message>
        <source>Transparency band</source>
        <translation>Banda de transparencia</translation>
    </message>
    <message>
        <source>Transparent pixel list</source>
        <translation>Lista de píxeles de transparentes</translation>
    </message>
    <message>
        <source>Add values manually</source>
        <translation>Añadir valores manualmente</translation>
    </message>
    <message>
        <source>Add Values from display</source>
        <translation>Añadir valores desde la visualización</translation>
    </message>
    <message>
        <source>Remove selected row</source>
        <translation>Eliminar la fila seleccionada</translation>
    </message>
    <message>
        <source>Default values</source>
        <translation>Valores predeterminados</translation>
    </message>
    <message>
        <source>Import from file</source>
        <translation>Importar de archivo</translation>
    </message>
    <message>
        <source>Export to file</source>
        <translation>Exporta a archivo</translation>
    </message>
    <message>
        <source>Number of entries</source>
        <translation>Número de entradas</translation>
    </message>
    <message>
        <source>Color interpolation</source>
        <translation>Interpolación de color</translation>
    </message>
    <message>
        <source>Classification mode</source>
        <translation>Modo de clasificación</translation>
    </message>
    <message>
        <source>Scale dependent visibility</source>
        <translation>Visibilidad dependiente de la escala</translation>
    </message>
    <message>
        <source>Maximum</source>
        <translation>Máximo</translation>
    </message>
    <message>
        <source>Minimum</source>
        <translation>Mínimo</translation>
    </message>
    <message>
        <source>Layer source</source>
        <translation>Fuente de la capa</translation>
    </message>
    <message>
        <source>Display name</source>
        <translation>Nombre a visualizar</translation>
    </message>
    <message>
        <source>Pyramid resolutions</source>
        <translation>Resoluciones de pirámides</translation>
    </message>
    <message>
        <source>Resampling method</source>
        <translation>Método de remuestreo</translation>
    </message>
    <message>
        <source>Build pyramids</source>
        <translation>Construir pirámides</translation>
    </message>
    <message>
        <source>Line graph</source>
        <translation>Gráfico de líneas</translation>
    </message>
    <message>
        <source>Bar chart</source>
        <translation>Gráfico de barras</translation>
    </message>
    <message>
        <source>Column count</source>
        <translation>Cuenta de columnas</translation>
    </message>
    <message>
        <source>Out of range OK?</source>
        <translation>¿Aceptar fuera de intervalo?</translation>
    </message>
    <message>
        <source>Allow approximation</source>
        <translation>Permitir aproximación</translation>
    </message>
    <message>
        <source>Restore Default Style</source>
        <translation>Restaurar estilo predeterminado</translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation>Guardar como predeterminado</translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation>Cargar estilo...</translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation>Guardar estilo...</translation>
    </message>
    <message>
        <source>Note:</source>
        <translation>Nota:</translation>
    </message>
    <message>
        <source>Default R:1 G:2 B:3</source>
        <translation>R:1 G:2 B:3 predeterminado</translation>
    </message>
    <message>
        <source>Add entry</source>
        <translation>Añadir entrada</translation>
    </message>
    <message>
        <source>Sort</source>
        <translation>Ordenar</translation>
    </message>
    <message>
        <source>Load color map from band</source>
        <translation>Cargar mapa de color de banda</translation>
    </message>
    <message>
        <source>Load color map from file</source>
        <translation>Cargar mapa de color de archivo</translation>
    </message>
    <message>
        <source>Export color map to file</source>
        <translation>Exportar mapa de color a archivo</translation>
    </message>
    <message>
        <source>Generate new color map</source>
        <translation>Generar nuevo mapa de color</translation>
    </message>
    <message>
        <source>Coordinate reference system</source>
        <translation>Sistema de referencia de coordenadas</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Cambiar...</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Leyenda</translation>
    </message>
    <message>
        <source>Palette</source>
        <translation>Paleta</translation>
    </message>
    <message>
        <source>Notes</source>
        <translation>Notas</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Build pyramids internally if possible</source>
        <translation>Construir pirámides internamente si es posible</translation>
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
        <translation>No se puede ejecutar el comando</translation>
    </message>
    <message>
        <source>Starting</source>
        <translation>Empezando</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Hecho</translation>
    </message>
    <message>
        <source>Action</source>
        <translation>Acción</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <source>Tick Down</source>
        <translation>Marcas abajo</translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation>Marcas arriba</translation>
    </message>
    <message>
        <source>Bar</source>
        <translation>Barra</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Cajetín</translation>
    </message>
    <message>
        <source>&amp;Scale Bar</source>
        <translation>Barra de &amp;escala</translation>
    </message>
    <message>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation>Crea una barra de escala que se muestra en la Vista del mapa</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Ilustraciones</translation>
    </message>
    <message>
        <source> metres/km</source>
        <translation> metros/km</translation>
    </message>
    <message>
        <source> feet</source>
        <translation> pies</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation> grados</translation>
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
        <translation> pie</translation>
    </message>
    <message>
        <source> degree</source>
        <translation> grado</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation> desconocido</translation>
    </message>
    <message>
        <source> feet/miles</source>
        <translation> pies/millas</translation>
    </message>
    <message>
        <source> miles</source>
        <translation> millas</translation>
    </message>
    <message>
        <source> mile</source>
        <translation> milla</translation>
    </message>
    <message>
        <source> inches</source>
        <translation> pulgadas</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <source>Scale Bar Plugin</source>
        <translation>Complemento de Barra de escala</translation>
    </message>
    <message>
        <source>Click to select the colour</source>
        <translation>Pulsar para seleccionar el color</translation>
    </message>
    <message>
        <source>Size of bar:</source>
        <translation>Tamaño de la barra:</translation>
    </message>
    <message>
        <source>Automatically snap to round number on resize</source>
        <translation>Redondear números automáticamente al cambiar de tamaño</translation>
    </message>
    <message>
        <source>Colour of bar:</source>
        <translation>Color de la barra:</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Superior izquierda</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Superior derecha</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Inferior izquierda</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Inferior derecha</translation>
    </message>
    <message>
        <source>Enable scale bar</source>
        <translation>Activar barra de escala</translation>
    </message>
    <message>
        <source>Scale bar style:</source>
        <translation>Estilo de la barra de escala:</translation>
    </message>
    <message>
        <source>Select the style of the scale bar</source>
        <translation>Seleccionar el estilo de la barra de escala</translation>
    </message>
    <message>
        <source>Tick Down</source>
        <translation>Marcas abajo</translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation>Marcas arriba</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Cajetín</translation>
    </message>
    <message>
        <source>Bar</source>
        <translation>Barra</translation>
    </message>
    <message>
        <source>Placement:</source>
        <translation>Ubicación:</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Este complemento dibuja una barra de escala sobre el mapa. Tenga en cuenta que la opción Tamaño indicada abajo es un tamaño &apos;preferido&apos; y puede que tenga que ser alterada por QGIS dependiendo del nivel de zum. El tamaño se establece de acuerdo con las unidades del mapa especificadas en las propiedades del proyecto.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message>
        <source>No matching features found.</source>
        <translation>No se han encontrado objetos espaciales coincidentes.</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation>Resultados de la búsqueda</translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation>Error al analizar la cadena de búsqueda</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation>Ningún registro</translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned.</source>
        <translation>La consulta que especificó no ha devuelto ningún registro.</translation>
    </message>
    <message>
        <source>Search query builder</source>
        <translation>Constructor de consultas de búsqueda</translation>
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
        <translation>¿Está seguro de que quiere eliminar la conexión </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> y toda su configuración?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Confirmar borrado</translation>
    </message>
    <message>
        <source>WMS Provider</source>
        <translation>Proveedor WMS</translation>
    </message>
    <message>
        <source>Could not open the WMS Provider</source>
        <translation>No se pudo conectar al proveedor WMS</translation>
    </message>
    <message>
        <source>Select Layer</source>
        <translation>Seleccionar capa</translation>
    </message>
    <message>
        <source>You must select at least one layer first.</source>
        <translation>Primero debe seleccionar al menos una capa.</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Sistema de referencia de coordenadas</translation>
    </message>
    <message>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation>No hay un sistema de coordenadas de referencia para las capas seleccionadas.</translation>
    </message>
    <message>
        <source>Could not understand the response.  The</source>
        <translation>La respuesta es ininteligible. El proveedor </translation>
    </message>
    <message>
        <source>provider said</source>
        <translation> ha dicho</translation>
    </message>
    <message>
        <source>WMS proxies</source>
        <translation>Proxy del servidor WMS</translation>
    </message>
    <message>
        <source>Several WMS servers have been added to the server list. Note that if you access the internet via a web proxy, you will need to set the proxy settings in the QGIS options dialog.</source>
        <translation>Se han añadido varios servidores WMS a la lista de servidores. Tenga en cuenta que si accede a internet mediante un proxy necesitará configurarlo en el diálogo de opciones de QGIS.</translation>
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
        <translation>Añadir capa(s) de un servidor</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation>Conexiones de servidor</translation>
    </message>
    <message>
        <source>Adds a few example WMS servers</source>
        <translation>Añadir unos cuantos servidores WMS de ejemplo</translation>
    </message>
    <message>
        <source>Add default servers</source>
        <translation>Añadir servidores por omisión</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>Co&amp;nectar</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation>&amp;Nuevo</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Sistema de referencia de coordenadas</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Cambiar...</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation>Preparado</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Añadir</translation>
    </message>
    <message>
        <source>Alt+A</source>
        <translation>Alt+A</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation>Capas</translation>
    </message>
    <message>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Resumen</translation>
    </message>
    <message>
        <source>Image encoding</source>
        <translation>Codificación de la imagen</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>C&amp;lose</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>Alt+L</source>
        <translation>Alt+C</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <source>The database gave an error while executing this SQL:</source>
        <translation>La base de datos dio un error mientras ejecutaba esta SQL:</translation>
    </message>
    <message>
        <source>The error was:</source>
        <translation>El error fue:</translation>
    </message>
    <message>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation>... (cortado el resto de la SQL)</translation>
    </message>
    <message>
        <source>Scanning </source>
        <translation>Escaneando </translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <source>Solid Line</source>
        <translation>Línea continua</translation>
    </message>
    <message>
        <source>Dash Line</source>
        <translation>Línea de rayas</translation>
    </message>
    <message>
        <source>Dot Line</source>
        <translation>Línea de puntos</translation>
    </message>
    <message>
        <source>Dash Dot Line</source>
        <translation>Línea de rayas y puntos</translation>
    </message>
    <message>
        <source>Dash Dot Dot Line</source>
        <translation>Línea de raya punto punto</translation>
    </message>
    <message>
        <source>No Pen</source>
        <translation>Sin dibujo</translation>
    </message>
    <message>
        <source>No Brush</source>
        <translation>Ningún pincel</translation>
    </message>
    <message>
        <source>Solid</source>
        <translation>Sólido</translation>
    </message>
    <message>
        <source>Horizontal</source>
        <translation>Horizontal</translation>
    </message>
    <message>
        <source>Vertical</source>
        <translation>Vertical</translation>
    </message>
    <message>
        <source>Cross</source>
        <translation>Cruz</translation>
    </message>
    <message>
        <source>BDiagonal</source>
        <translation>Diagonal atrás</translation>
    </message>
    <message>
        <source>FDiagonal</source>
        <translation>Diagonal adelante</translation>
    </message>
    <message>
        <source>Diagonal X</source>
        <translation>Diagonal X</translation>
    </message>
    <message>
        <source>Dense1</source>
        <translation>Denso 1</translation>
    </message>
    <message>
        <source>Dense2</source>
        <translation>Denso 2</translation>
    </message>
    <message>
        <source>Dense3</source>
        <translation>Denso 3</translation>
    </message>
    <message>
        <source>Dense4</source>
        <translation>Denso 4</translation>
    </message>
    <message>
        <source>Dense5</source>
        <translation>Denso 5</translation>
    </message>
    <message>
        <source>Dense6</source>
        <translation>Denso 6</translation>
    </message>
    <message>
        <source>Dense7</source>
        <translation>Denso 7</translation>
    </message>
    <message>
        <source>Texture</source>
        <translation>Textura</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialogBase</name>
    <message>
        <source>Single Symbol</source>
        <translation>Símbolo único</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Tamaño</translation>
    </message>
    <message>
        <source>Point Symbol</source>
        <translation>Símbolo de punto</translation>
    </message>
    <message>
        <source>Area scale field</source>
        <translation>Campo de escala de área</translation>
    </message>
    <message>
        <source>Rotation field</source>
        <translation>Campo de rotación</translation>
    </message>
    <message>
        <source>Style Options</source>
        <translation>Opciones de estilo</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Outline style</source>
        <translation>Estilo de línea exterior</translation>
    </message>
    <message>
        <source>Outline color</source>
        <translation>Color de línea exterior</translation>
    </message>
    <message>
        <source>Outline width</source>
        <translation>Anchura de línea exterior</translation>
    </message>
    <message>
        <source>Fill color</source>
        <translation>Color de relleno</translation>
    </message>
    <message>
        <source>Fill style</source>
        <translation>Estilo de relleno</translation>
    </message>
    <message>
        <source>Label</source>
        <translation>Etiqueta</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <source>to vertex</source>
        <translation>a vértice</translation>
    </message>
    <message>
        <source>to segment</source>
        <translation>a segmento</translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation>a vértice y segmento</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <source>Snapping options</source>
        <translation>Opciones de autoensamblado</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Capa</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Modo</translation>
    </message>
    <message>
        <source>Tolerance</source>
        <translation>Tolerancia</translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <source>File Name</source>
        <translation>Nombre de archivo</translation>
    </message>
    <message>
        <source>Feature Class</source>
        <translation>Clase de objetos espaciales</translation>
    </message>
    <message>
        <source>Features</source>
        <translation>Objetos espaciales</translation>
    </message>
    <message>
        <source>DB Relation Name</source>
        <translation>Nombre de la relación de la BD</translation>
    </message>
    <message>
        <source>Schema</source>
        <translation>Esquema</translation>
    </message>
    <message>
        <source>Are you sure you want to remove the [</source>
        <translation>¿Está seguro de que quiere eliminar la conexión [</translation>
    </message>
    <message>
        <source>] connection and all associated settings?</source>
        <translation>] y toda su configuración?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Confirmar borrado</translation>
    </message>
    <message>
        <source>Add Shapefiles</source>
        <translation>Añadir archivos shape</translation>
    </message>
    <message>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation>Archivos shape (*.shp);;Todos los archivos (*.*)</translation>
    </message>
    <message>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>No se han podido cargar los siguientes archivos shape:

</translation>
    </message>
    <message>
        <source>REASON: File cannot be opened</source>
        <translation>MOTIVO: el archivo no se puede abrir</translation>
    </message>
    <message>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>MOTIVO: faltan uno o varios de los archivos del shape (*.dbf, *shx)</translation>
    </message>
    <message>
        <source>General Interface Help:</source>
        <translation>Ayuda de la Interfaz general:</translation>
    </message>
    <message>
        <source>PostgreSQL Connections:</source>
        <translation>Conexiones PostgreSQL:</translation>
    </message>
    <message>
        <source>[New ...] - create a new connection</source>
        <translation>[Nueva...] - crear una conexión nueva</translation>
    </message>
    <message>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation>[Editar...] - editar la conexión seleccionada actualmente</translation>
    </message>
    <message>
        <source>[Remove] - remove the currently selected connection</source>
        <translation>[Eliminar] - eliminar la conexión seleccionada actualmente</translation>
    </message>
    <message>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation>-debe seleccionar una conexión que funcione para poder importar archivos</translation>
    </message>
    <message>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation>-cuando se cambian las conexiones el esquema global cambia en concordancia</translation>
    </message>
    <message>
        <source>Shapefile List:</source>
        <translation>Lista de archivos shape:</translation>
    </message>
    <message>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation>[Añadir...] - abrir un cuadro de diálogo y buscar los archivos a importar</translation>
    </message>
    <message>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation>[Eliminar] - eliminar los archivos seleccionados de la lista</translation>
    </message>
    <message>
        <source>[Remove All] - remove all the files in the list</source>
        <translation>[Eliminar todos] - eliminar todos los archivos de la lista</translation>
    </message>
    <message>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] - ID de referencia para los archivos shape a importar</translation>
    </message>
    <message>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation>[Utilizar (SRID) por omisión] - establecer SRID a -1</translation>
    </message>
    <message>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation>[Nombre de columna de la geometría] - nombre de la columna de la geometría en la base de datos</translation>
    </message>
    <message>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation>[Utilizar [Nombre de columna de la geometría] por omisión] - establecer el nombre de la columna a &quot;the_geom&quot;</translation>
    </message>
    <message>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation>[Esquema global] - establecer el esquema para todos los archivos a importar</translation>
    </message>
    <message>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[Importar] - importar los archivos shape actuales de la lista</translation>
    </message>
    <message>
        <source>[Quit] - quit the program
</source>
        <translation>[Cerrar] - salir del programa
</translation>
    </message>
    <message>
        <source>[Help] - display this help dialog</source>
        <translation>[Ayuda] - mostrar esta ayuda</translation>
    </message>
    <message>
        <source>Import Shapefiles</source>
        <translation>Importar archivos shape</translation>
    </message>
    <message>
        <source>You need to specify a Connection first</source>
        <translation>Primero debe especificar una conexión</translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again</source>
        <translation>La conexión ha fallado - Comprobar la configuración y probar de nuevo</translation>
    </message>
    <message>
        <source>PostGIS not available</source>
        <translation>PostGIS no está disponible</translation>
    </message>
    <message>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;La base de datos seleccionada no tiene instalado PostGIS, lo cual es necesario para almacenar datos espaciales.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>You need to add shapefiles to the list first</source>
        <translation>Primero debe añadir archivos shape a la lista</translation>
    </message>
    <message>
        <source>Importing files</source>
        <translation>Importando archivos</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>Progress</source>
        <translation>Progreso</translation>
    </message>
    <message>
        <source>Problem inserting features from file:</source>
        <translation>Problemas al insertar objetos espaciales del archivo:</translation>
    </message>
    <message>
        <source>Invalid table name.</source>
        <translation>Nombre de la tabla no válido.</translation>
    </message>
    <message>
        <source>No fields detected.</source>
        <translation>No se han detectado campos.</translation>
    </message>
    <message>
        <source>The following fields are duplicates:</source>
        <translation>Los siguientes campos están duplicados:</translation>
    </message>
    <message>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Error al ejecutar la SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation>&lt;/p&gt;&lt;p&gt;La base de datos ha dicho:</translation>
    </message>
    <message>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>Importar archivos shape - La relación existe</translation>
    </message>
    <message>
        <source>The Shapefile:</source>
        <translation>El archivo shape:</translation>
    </message>
    <message>
        <source>will use [</source>
        <translation>utilizará la relación [</translation>
    </message>
    <message>
        <source>] relation for its data,</source>
        <translation>] para sus datos,</translation>
    </message>
    <message>
        <source>which already exists and possibly contains data.</source>
        <translation>que ya existe y posiblemente contenga datos.</translation>
    </message>
    <message>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>Para evitar la pérdida de datos cambie el &quot;Nombre de la relación de la BD&quot;</translation>
    </message>
    <message>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation>para este archivo shape en la lista de archivos del diálogo principal.</translation>
    </message>
    <message>
        <source>Do you want to overwrite the [</source>
        <translation>¿Quiere sobrescribir la relación [</translation>
    </message>
    <message>
        <source>] relation?</source>
        <translation>]?</translation>
    </message>
    <message>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation>No se pudieron importar %1 de %2 archivos shape.</translation>
    </message>
    <message>
        <source>Password for </source>
        <translation>Contraseña para </translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation>Por favor, introduzca su contraseña:</translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT - Herramienta para importar archivos shape a PostGIS</translation>
    </message>
    <message>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Añadir un archivo shape a la lista de archivos a importar</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <source>Remove the selected shapefile from the import list</source>
        <translation>Eliminar el archivo shape seleccionado de la lista de importación</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
    <message>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Eliminar todos los archivos shape de la lista de importación</translation>
    </message>
    <message>
        <source>Remove All</source>
        <translation>Eliminar todos</translation>
    </message>
    <message>
        <source>Set the SRID to the default value</source>
        <translation>Establecer el SRID al valor por omisión</translation>
    </message>
    <message>
        <source>Set the geometry column name to the default value</source>
        <translation>Establecer el nombre de la columna de la geometría al valor por omisión</translation>
    </message>
    <message>
        <source>Global Schema</source>
        <translation>Esquema global</translation>
    </message>
    <message>
        <source>PostgreSQL Connections</source>
        <translation>Conexiones PostgreSQL</translation>
    </message>
    <message>
        <source>Create a new PostGIS connection</source>
        <translation>Crear una nueva conexión a PostGIS</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nueva</translation>
    </message>
    <message>
        <source>Remove the current PostGIS connection</source>
        <translation>Eliminar la conexión actual a PostGIS</translation>
    </message>
    <message>
        <source>Edit the current PostGIS connection</source>
        <translation>Editar la conexión actual a PostGIS</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <source>Import options and shapefile list</source>
        <translation>Opciones de importación y lista de archivos shape</translation>
    </message>
    <message>
        <source>Use Default SRID or specify here</source>
        <translation>Usar SRID por omisión o especificar aquí</translation>
    </message>
    <message>
        <source>Use Default Geometry Column Name or specify here</source>
        <translation>Usar nombre de columna de geometría por omisión o especificar aquí</translation>
    </message>
    <message>
        <source>Primary Key Column Name</source>
        <translation>Nombre de la columna de clave primaria</translation>
    </message>
    <message>
        <source>Connect to PostGIS</source>
        <translation>Conectar a PostGIS</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Conectar</translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>&amp;Importar archivos shape a PostgreSQL</translation>
    </message>
    <message>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>Importar archivos shape a una base de datos PostgreSQL-PostGIS. El esquema y los nombres de los campos se pueden personalizar al importar</translation>
    </message>
    <message>
        <source>&amp;Spit</source>
        <translation>I&amp;mportar (Spit)</translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialog</name>
    <message>
        <source>Linear interpolation</source>
        <translation>Interpolación lineal</translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialogBase</name>
    <message>
        <source>Triangle based interpolation</source>
        <translation>Interpolación basada en triángulos</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This interpolator provides different methods for interpolation in a triangular irregular network (TIN).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Este interpolador proporciona diferentes métodos para la interpolación en una red triangular irregular (TIN).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Interpolation method:</source>
        <translation>Método de interpolación:</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialog</name>
    <message>
        <source>Confirm Delete</source>
        <translation>Confirmar borrado</translation>
    </message>
    <message>
        <source>The classification field was changed from &apos;%1&apos; to &apos;%2&apos;.
Should the existing classes be deleted before classification?</source>
        <translation>El campo de clasificación se cambió de &apos;%1&apos; a &apos;%2&apos;.
¿Deben borrarse las clases existentes antes de la clasificación?</translation>
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
        <translation>Clasificar</translation>
    </message>
    <message>
        <source>Classification field</source>
        <translation>Campo de clasificación</translation>
    </message>
    <message>
        <source>Add class</source>
        <translation>Añadir clase</translation>
    </message>
    <message>
        <source>Delete classes</source>
        <translation>Borrar clases</translation>
    </message>
    <message>
        <source>Randomize Colors</source>
        <translation>Colores aleatorios</translation>
    </message>
    <message>
        <source>Reset Colors</source>
        <translation>Restablecer colores</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <source>ERROR: no provider</source>
        <translation>ERROR: ningún proveedor</translation>
    </message>
    <message>
        <source>ERROR: layer not editable</source>
        <translation>ERROR: capa no editable</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attributes added.</source>
        <translation>ÉXITO: se han añadido %1 atributos.</translation>
    </message>
    <message>
        <source>ERROR: %1 new attributes not added</source>
        <translation>ERROR: no se han añadido %1 atributos nuevos</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attributes deleted.</source>
        <translation>ÉXITO: se han borrado %1 atributos.</translation>
    </message>
    <message>
        <source>ERROR: %1 attributes not deleted.</source>
        <translation>ERROR: no se han borrado %1 atributos.</translation>
    </message>
    <message>
        <source>SUCCESS: attribute %1 was added.</source>
        <translation>ÉXITO: se añadió el atributo %1.</translation>
    </message>
    <message>
        <source>ERROR: attribute %1 not added</source>
        <translation>ERROR: no se añadió el atributo %1</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attribute values changed.</source>
        <translation>ÉXITO: se cambiaron los valores de %1 atributos.</translation>
    </message>
    <message>
        <source>ERROR: %1 attribute value changes not applied.</source>
        <translation>ERROR: no se aplicaron los cambios en el valor del atributo %1.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 features added.</source>
        <translation>ÉXITO: se han añadido %1 objetos espaciales.</translation>
    </message>
    <message>
        <source>ERROR: %1 features not added.</source>
        <translation>ERROR: no se han añadido %1 objetos espaciales.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 geometries were changed.</source>
        <translation>ÉXITO: se cambiaron %1 geometrías.</translation>
    </message>
    <message>
        <source>ERROR: %1 geometries not changed.</source>
        <translation>ERROR: no se cambiaron %1 geometrías.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 features deleted.</source>
        <translation>ÉXITO: se borraron %1 objetos espaciales.</translation>
    </message>
    <message>
        <source>ERROR: %1 features not deleted.</source>
        <translation>ERROR: no se borraron %1 objetos espaciales.</translation>
    </message>
    <message>
        <source>No renderer object</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Classification field not found</source>
        <translation>No se ha encontrado el campo de clasificación</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <source>Transparency: </source>
        <translation>Transparencia: </translation>
    </message>
    <message>
        <source>Single Symbol</source>
        <translation>Símbolo único</translation>
    </message>
    <message>
        <source>Graduated Symbol</source>
        <translation>Símbolo graduado</translation>
    </message>
    <message>
        <source>Continuous Color</source>
        <translation>Color graduado</translation>
    </message>
    <message>
        <source>Unique Value</source>
        <translation>Valor único</translation>
    </message>
    <message>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation>Este botón abre el constructor de consultas de PostgreSQL y permite crear un subconjunto de objetos espaciales para mostrar en la vista del mapa, en vez de mostrarlos todos</translation>
    </message>
    <message>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>Aquí se muestra la consulta usada para limitar los objetos espaciales de las capas. Esto actualmente sólo está soportado para capas PostgreSQL. Para introducir o modificar la consulta, pulse el botón Constructor de consultas</translation>
    </message>
    <message>
        <source>Spatial Index</source>
        <translation>Índice espacial</translation>
    </message>
    <message>
        <source>Creation of spatial index failed</source>
        <translation>Ha fallado la creación del índice espacial</translation>
    </message>
    <message>
        <source>General:</source>
        <translation>General:</translation>
    </message>
    <message>
        <source>Storage type of this layer : </source>
        <translation>Tipo de almacenamiento de esta capa: </translation>
    </message>
    <message>
        <source>Source for this layer : </source>
        <translation>Fuente de esta capa: </translation>
    </message>
    <message>
        <source>Geometry type of the features in this layer : </source>
        <translation>Tipo de geometría de los objetos espaciales en esta capa: </translation>
    </message>
    <message>
        <source>The number of features in this layer : </source>
        <translation>Número de objetos espaciales en esta capa: </translation>
    </message>
    <message>
        <source>Editing capabilities of this layer : </source>
        <translation>Posibilidades de edición de esta capa: </translation>
    </message>
    <message>
        <source>Extents:</source>
        <translation>Extensión:</translation>
    </message>
    <message>
        <source>In layer spatial reference system units : </source>
        <translation>En unidades del sistema espacial de referencia de la capa: </translation>
    </message>
    <message>
        <source>xMin,yMin </source>
        <translation>xMín,yMín </translation>
    </message>
    <message>
        <source> : xMax,yMax </source>
        <translation> : xMáx,yMáx </translation>
    </message>
    <message>
        <source>In project spatial reference system units : </source>
        <translation>En unidades del sistema espacial de referencia del proyecto: </translation>
    </message>
    <message>
        <source>Layer Spatial Reference System:</source>
        <translation>Sistema de referencia espacial de la capa:</translation>
    </message>
    <message>
        <source>Attribute field info:</source>
        <translation>Información del campo del atributo:</translation>
    </message>
    <message>
        <source>Field</source>
        <translation>Campo</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Tamaño</translation>
    </message>
    <message>
        <source>Precision</source>
        <translation>Precisión</translation>
    </message>
    <message>
        <source>Layer comment: </source>
        <translation>Comentario de la capa: </translation>
    </message>
    <message>
        <source>Comment</source>
        <translation>Comentario</translation>
    </message>
    <message>
        <source>Default Style</source>
        <translation>Estilo predeterminado</translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Archivo de estilo de capa de QGIS (*.qml)</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation>Formato de estilo desconocido: </translation>
    </message>
    <message>
        <source>id</source>
        <translation>ID</translation>
    </message>
    <message>
        <source>name</source>
        <translation>nombre</translation>
    </message>
    <message>
        <source>type</source>
        <translation>tipo</translation>
    </message>
    <message>
        <source>length</source>
        <translation>longitud</translation>
    </message>
    <message>
        <source>precision</source>
        <translation>precisión</translation>
    </message>
    <message>
        <source>comment</source>
        <translation>comentario</translation>
    </message>
    <message>
        <source>edit widget</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>values</source>
        <translation>valores</translation>
    </message>
    <message>
        <source>line edit</source>
        <translation>edición de línea</translation>
    </message>
    <message>
        <source>unique values</source>
        <translation>valores únicos</translation>
    </message>
    <message>
        <source>unique values (editable)</source>
        <translation>valores únicos (editable)</translation>
    </message>
    <message>
        <source>value map</source>
        <translation>mapa de valor</translation>
    </message>
    <message>
        <source>classification</source>
        <translation>clasificación</translation>
    </message>
    <message>
        <source>range (editable)</source>
        <translation>intervalo (editable)</translation>
    </message>
    <message>
        <source>range (slider)</source>
        <translation>intervalo (deslizador)</translation>
    </message>
    <message>
        <source>file name</source>
        <translation>nombre de archivo</translation>
    </message>
    <message>
        <source>Name conflict</source>
        <translation>Conflicto de nombres</translation>
    </message>
    <message>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation>No se pudo insertar el atributo. El nombre ya existe en la tabla.</translation>
    </message>
    <message>
        <source>Creation of spatial index successful</source>
        <translation>Creación de índice espacial correcta</translation>
    </message>
    <message>
        <source>Saved Style</source>
        <translation>Estilo guardado</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <source>Layer Properties</source>
        <translation>Propiedades de la capa</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Simbología</translation>
    </message>
    <message>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <source>Display name</source>
        <translation>Mostrar el nombre</translation>
    </message>
    <message>
        <source>Display field for the Identify Results dialog box</source>
        <translation>Campo para mostrar en el cuadro de diálogo de resultados de la identificación</translation>
    </message>
    <message>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation>Establece el campo que se mostrará con la herramienta de identificación</translation>
    </message>
    <message>
        <source>Display field</source>
        <translation>Mostrar campo</translation>
    </message>
    <message>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation>Use este control para indicar el campo que se situará en el nivel superior del cuadro de diálogo de resultados de la identificación.</translation>
    </message>
    <message>
        <source>Use scale dependent rendering</source>
        <translation>Utilizar represenación dependiente de la escala</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Escala mínima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Escala máxima a la que se mostrará esta capa. </translation>
    </message>
    <message>
        <source>Create Spatial Index</source>
        <translation>Crear índice espacial</translation>
    </message>
    <message>
        <source>Subset</source>
        <translation>Subconjunto</translation>
    </message>
    <message>
        <source>Query Builder</source>
        <translation>Constructor de consultas</translation>
    </message>
    <message>
        <source>Metadata</source>
        <translation>Metadatos</translation>
    </message>
    <message>
        <source>Labels</source>
        <translation>Etiquetas</translation>
    </message>
    <message>
        <source>Display labels</source>
        <translation>Mostrar etiquetas</translation>
    </message>
    <message>
        <source>Actions</source>
        <translation>Acciones</translation>
    </message>
    <message>
        <source>Restore Default Style</source>
        <translation>Restaurar estilo predeterminado</translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation>Guardar como predeterminado</translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation>Cargar estilo...</translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation>Guardar estilo...</translation>
    </message>
    <message>
        <source>Legend type</source>
        <translation>Tipo de leyenda</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Transparencia</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opciones</translation>
    </message>
    <message>
        <source>Maximum</source>
        <translation>Máximo</translation>
    </message>
    <message>
        <source>Minimum</source>
        <translation>Mínimo</translation>
    </message>
    <message>
        <source>Change CRS</source>
        <translation>Cambiar SRC</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Atributos</translation>
    </message>
    <message>
        <source>New column</source>
        <translation>Nueva columna</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <source>Delete column</source>
        <translation>Borrar columna</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <source>Toggle editing mode</source>
        <translation>Conmutar el modo edición</translation>
    </message>
    <message>
        <source>Click to toggle table editing</source>
        <translation>Pulsar para conmutar la edición de la tabla</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <source>&amp;Add WFS layer</source>
        <translation>&amp;Añadir capa WFS</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <source>unknown</source>
        <translation>desconocido</translation>
    </message>
    <message>
        <source>received %1 bytes from %2</source>
        <translation>recibidos %1 bytes de %2</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>¿Está seguro de que quiere eleminar la conexión </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> y toda su configuración?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Confirmar borrado</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Resumen</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Sistema de referencia de coordenadas</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Cambiar...</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation>Conexiones de servidor</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation>&amp;Nuevo</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>Co&amp;nectar</translation>
    </message>
    <message>
        <source>Add WFS Layer from a Server</source>
        <translation>Añadir capa WFS desde un servidor</translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <source>Tried URL: </source>
        <translation>URL probada: </translation>
    </message>
    <message>
        <source>HTTP Exception</source>
        <translation>Excepción HTTP</translation>
    </message>
    <message>
        <source>WMS Service Exception</source>
        <translation>Excepción del servicio WMS</translation>
    </message>
    <message>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>No se pudieron obtener las capacidades del WMS: %1 en la línea %2 columna %3</translation>
    </message>
    <message>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>Probablemente se deba a una URL incorrecta del servidor WMS.</translation>
    </message>
    <message>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>No se pudieron obtener las capacidades del WMS en el formato esperado (DTD): no se ha encontrado %1 o %2</translation>
    </message>
    <message>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>No se pudo obtener una excepción del servicio WMS en %1: %2 en la línea %3 columna %4</translation>
    </message>
    <message>
        <source>Request contains a Format not offered by the server.</source>
        <translation>La solicitud contiene un formato no ofrecido por el servidor.</translation>
    </message>
    <message>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>La solicitud contiene un CRS no ofrecido por el servidor para una o más de las capas solicitadas.</translation>
    </message>
    <message>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>La solicitud contiene un SRS no ofrecido por el servidor para una o más de las capas solicitadas.</translation>
    </message>
    <message>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation>La solicitud de obtención de mapa (GetMap) es para una capa no ofrecida por el servidor o la solicitud de información del tema (GetFeatureInfo) es para una capa que no mostrada en el mapa.</translation>
    </message>
    <message>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>La solicitud es para una capa en un estilo no ofrecido por el servidor.</translation>
    </message>
    <message>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>La solicitud de información del tema (GetFeatureInfo) se aplica a una capa no declarada consultable.</translation>
    </message>
    <message>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>La solicitud de información del tema (GetFeatureInfo) contiene valores no válidos de X o Y.</translation>
    </message>
    <message>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation>El valor del parámetro (opcional) UpdateSequence en la consulta GetCapabilities es igual al valor actual del número de secuencia del servicio de actualización de los metadatos.</translation>
    </message>
    <message>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation>El valor del parámetro (opcional) UpdateSequence en la consulta GetCapabilities es mayor que el valor actual del número de secuencia del servicio de actualización de los metadatos.</translation>
    </message>
    <message>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation>La solicitud no incluye un valor de muestra para la dimensión y el servidor no ha declarado un valor por omisión para esa dimensión.</translation>
    </message>
    <message>
        <source>Request contains an invalid sample dimension value.</source>
        <translation>La solicitud contiene un valor de muestra para la dimensión no válido.</translation>
    </message>
    <message>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>La solicitud es para una operación opcional no soportada por el servidor.</translation>
    </message>
    <message>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(Código de error desconocido de un servidor post-1.3 WMS)</translation>
    </message>
    <message>
        <source>The WMS vendor also reported: </source>
        <translation>El productor WMS también informó: </translation>
    </message>
    <message>
        <source>Server Properties:</source>
        <translation>Propiedades del servidor:</translation>
    </message>
    <message>
        <source>Property</source>
        <translation>Propiedad</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Valor</translation>
    </message>
    <message>
        <source>WMS Version</source>
        <translation>Versión WMS</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Resumen</translation>
    </message>
    <message>
        <source>Keywords</source>
        <translation>Palabras clave</translation>
    </message>
    <message>
        <source>Online Resource</source>
        <translation>Recursos en línea</translation>
    </message>
    <message>
        <source>Contact Person</source>
        <translation>Persona de contacto</translation>
    </message>
    <message>
        <source>Fees</source>
        <translation>Cuotas</translation>
    </message>
    <message>
        <source>Access Constraints</source>
        <translation>Restricciones de acceso</translation>
    </message>
    <message>
        <source>Image Formats</source>
        <translation>Formatos de imagen</translation>
    </message>
    <message>
        <source>Identify Formats</source>
        <translation>Formatos de identificación</translation>
    </message>
    <message>
        <source>Layer Count</source>
        <translation>Número de capas</translation>
    </message>
    <message>
        <source>Layer Properties: </source>
        <translation>Propiedades de la capa: </translation>
    </message>
    <message>
        <source>Selected</source>
        <translation>Seleccionado</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Sí</translation>
    </message>
    <message>
        <source>No</source>
        <translation>No</translation>
    </message>
    <message>
        <source>Visibility</source>
        <translation>Visibilidad</translation>
    </message>
    <message>
        <source>Visible</source>
        <translation>Visible</translation>
    </message>
    <message>
        <source>Hidden</source>
        <translation>Oculta</translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>n/d</translation>
    </message>
    <message>
        <source>Can Identify</source>
        <translation>Se puede identificar</translation>
    </message>
    <message>
        <source>Can be Transparent</source>
        <translation>Puede ser transparente</translation>
    </message>
    <message>
        <source>Can Zoom In</source>
        <translation>Se puede acercar el zum</translation>
    </message>
    <message>
        <source>Cascade Count</source>
        <translation>Cuenta en cascada</translation>
    </message>
    <message>
        <source>Fixed Width</source>
        <translation>Anchura fija</translation>
    </message>
    <message>
        <source>Fixed Height</source>
        <translation>Altura fija</translation>
    </message>
    <message>
        <source>WGS 84 Bounding Box</source>
        <translation>Marco de la WGS 84</translation>
    </message>
    <message>
        <source>Available in CRS</source>
        <translation>Disponible en CRS</translation>
    </message>
    <message>
        <source>Available in style</source>
        <translation>Disponible en estilo</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>Layer cannot be queried.</source>
        <translation>La capa no se puede consultar.</translation>
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
        <translation>Formato de documento portátil (*.pdf)</translation>
    </message>
    <message>
        <source>quickprint</source>
        <translation>impresión rápida</translation>
    </message>
    <message>
        <source>Unknown format: </source>
        <translation>Formato desconocido: </translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <source>QGIS Quick Print Plugin</source>
        <translation>Complemento de impresión rápida de QGIS</translation>
    </message>
    <message>
        <source>Quick Print</source>
        <translation>Impresión rápida</translation>
    </message>
    <message>
        <source>Map Title e.g. ACME inc.</source>
        <translation>Título del mapa, ej. ACME inc.</translation>
    </message>
    <message>
        <source>Map Name e.g. Water Features</source>
        <translation>Nombre del mapa, ej. Elementos acuáticos</translation>
    </message>
    <message>
        <source>Copyright</source>
        <translation>Copyright</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Salida</translation>
    </message>
    <message>
        <source>Use last filename but incremented.</source>
        <translation>Usar el último nombre de archivo, pero incrementado.</translation>
    </message>
    <message>
        <source>last used filename but incremented will be shown here</source>
        <translation>el último nombre de archivo usado incrementado se mostrará aquí</translation>
    </message>
    <message>
        <source>Prompt for file name</source>
        <translation>Preguntar nombre de archivo</translation>
    </message>
    <message>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation>Nota: si quiere más control sobre la disposición del mapa, por favor use la función Diseñador de mapas de QGIS.</translation>
    </message>
    <message>
        <source>Page Size</source>
        <translation>Tamaño de página</translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <source>Quick Print</source>
        <translation>Impresión rápida</translation>
    </message>
    <message>
        <source>&amp;Quick Print</source>
        <translation>Impresión &amp;rápida</translation>
    </message>
    <message>
        <source>Provides a way to quickly produce a map with minimal user input.</source>
        <translation>Proporciona una forma de producir rápidamente un mapa con intervención mínima del usuario.</translation>
    </message>
</context>
<context>
    <name>[pluginname]GuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation>Plantilla de complementos de QGIS</translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation>Plantilla de complementos</translation>
    </message>
</context>
<context>
    <name>dxf2shpConverter</name>
    <message>
        <source>Converts DXF files in Shapefile format</source>
        <translation>Convierte archivos DXF a formato SHP</translation>
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
        <translation>Importador DXF</translation>
    </message>
    <message>
        <source>Input Dxf file</source>
        <translation>Archivo dxf de entrada</translation>
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
        <translation>Tipo de archivo de salida</translation>
    </message>
    <message>
        <source>Polyline</source>
        <translation>Polilínea</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Polígono</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punto</translation>
    </message>
    <message>
        <source>Export text labels</source>
        <translation>Exportar etiquetas de texto</translation>
    </message>
    <message>
        <source>Choose a file name to save to</source>
        <translation type="obsolete">Seleccione un nombre de archivo en el que guardar</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <source>[menuitemname]</source>
        <translation>[nombredeelementodemenu]</translation>
    </message>
    <message>
        <source>&amp;[menuname]</source>
        <translation>&amp;{nombremenu]</translation>
    </message>
    <message>
        <source>Replace this with a short description of what the plugin does</source>
        <translation>Sustituya esto por una breve descripción de lo que hace el complemento</translation>
    </message>
</context>
</TS>
