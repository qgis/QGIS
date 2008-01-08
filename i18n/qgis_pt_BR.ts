<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1">
<context>
    <name>@default</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1642"/>
        <source>OGR Driver Manager</source>
        <translation>Driver gerenciador OGR</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1642"/>
        <source>unable to get OGRDriverManager</source>
        <translation>Impossível pegar o Gerenciador de driver OGR</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="13"/>
        <source>QGIS Plugin Installer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="23"/>
        <source>Retrieve the list of available plugins, select one and install it</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="36"/>
        <source>Get List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="52"/>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="57"/>
        <source>Version</source>
        <translation type="unfinished">Versão</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="62"/>
        <source>Description</source>
        <translation type="unfinished">Descrição</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="67"/>
        <source>Author</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="77"/>
        <source>Name of plugin to install</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="87"/>
        <source>Install Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="98"/>
        <source>The plugin will be installed to ~/.qgis/python/plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="111"/>
        <source>Done</source>
        <translation type="unfinished">Feito</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="55"/>
        <source>Welcome to your automatically generated plugin!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="56"/>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="57"/>
        <source>Documentation:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="58"/>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="59"/>
        <source>In particular look at the following classes:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="62"/>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="63"/>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="64"/>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="65"/>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="66"/>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="67"/>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="68"/>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="69"/>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="70"/>
        <source>This file contains the documentation you are reading now!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="71"/>
        <source>Getting developer help:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="72"/>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="73"/>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="74"/>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="75"/>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="13"/>
        <source>Enter map coordinates</source>
        <translation>Entre as coordenadas do mapa</translation>
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
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="28"/>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>Entre com as coordenadas X e Y que correspondem ao ponto selecionado na imagem. Alternativamente, clique no ícone com um lápis e, então, clique num ponto correspondente na superfície do mapa para preencher as cooredenadas para aquele ponto.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="137"/>
        <source> from map canvas</source>
        <translation> da superfície do mapa</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2907"/>
        <source>QGis files (*.qgs)</source>
        <translation>Arquivos QGis (*.qgs)</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="87"/>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation>Nenhum plugin de acesso a dados encontrado</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="89"/>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>Nenhuma camada vetorial pôde ser carregada. Verifique sua instalação do QGIS</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="92"/>
        <source>No Data Providers</source>
        <translation>Nenhum mecanismo de acesso a dados</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="251"/>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>Nenhum plugin de acesso a dados disponível. Nenhuma camada vetorial pôde ser carregada</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="769"/>
        <source> at line </source>
        <translation>na linha</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="770"/>
        <source> column </source>
        <translation>coluna</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="776"/>
        <source> for file </source>
        <translation>para arquivo</translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="289"/>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>Coluna referenciada não foi encontrada: </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="293"/>
        <source>Division by zero.</source>
        <translation>Divisão por zero.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="75"/>
        <source>No active layer</source>
        <translation type="unfinished">Nenhuma camada ativa</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="153"/>
        <source>Band</source>
        <translation type="unfinished">Banda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="332"/>
        <source>action</source>
        <translation type="unfinished">ação</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="341"/>
        <source> features found</source>
        <translation> feições encontradas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="345"/>
        <source> 1 feature found</source>
        <translation> 1 feição encontrada</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="351"/>
        <source>No features found</source>
        <translation type="unfinished">Nenhuma feição encontrada</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="351"/>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation type="unfinished">Nenhuma feição encontrada no ponto clicado da camada ativa</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="451"/>
        <source>Could not identify objects on</source>
        <translation>Impossível identificar objetos em</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="451"/>
        <source>because</source>
        <translation>porque</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="944"/>
        <source>Unable to save to file </source>
        <translation>Impossível salvar o arquivo </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="72"/>
        <source>New centroid</source>
        <translation type="unfinished">Novo centróide</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>New point</source>
        <translation type="unfinished">Novo ponto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="134"/>
        <source>New vertex</source>
        <translation type="unfinished">Novo vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>Undo last point</source>
        <translation type="unfinished">Desfazer último ponto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>Close line</source>
        <translation type="unfinished">Fechar linha</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="543"/>
        <source>Select vertex</source>
        <translation type="unfinished">Selecione vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="296"/>
        <source>Select new position</source>
        <translation type="unfinished">Selecione nova posição</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="427"/>
        <source>Select line segment</source>
        <translation type="unfinished">Selecione segmento de linha</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="414"/>
        <source>New vertex position</source>
        <translation type="unfinished">Nova posição do vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="414"/>
        <source>Release</source>
        <translation type="unfinished">Desmarcar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="530"/>
        <source>Delete vertex</source>
        <translation>Excluir vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="530"/>
        <source>Release vertex</source>
        <translation type="unfinished">Desmarcar vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="784"/>
        <source>Select element</source>
        <translation type="unfinished">Selecione elemento</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="597"/>
        <source>New location</source>
        <translation type="unfinished">Nova localidade</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="673"/>
        <source>Release selected</source>
        <translation type="unfinished">Desmarcar selecionado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="673"/>
        <source>Delete selected / select next</source>
        <translation type="unfinished">Excluir selecionado / selecione próximo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="736"/>
        <source>Select position on line</source>
        <translation type="unfinished">Selecione posição na linha</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="754"/>
        <source>Split the line</source>
        <translation type="unfinished">Quebrar linha</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="754"/>
        <source>Release the line</source>
        <translation type="unfinished">Desmarcar a linha</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="768"/>
        <source>Select point on line</source>
        <translation type="unfinished">Selecione ponto na linha</translation>
    </message>
    <message>
        <location filename="../src/core/qgslabelattributes.cpp" line="58"/>
        <source>Label</source>
        <translation type="unfinished">Rótulo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="319"/>
        <source>Length</source>
        <translation type="unfinished">Tamanho</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="325"/>
        <source>Area</source>
        <translation>Área</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Could not snap segment. Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation type="obsolete">Impossível quebrar o segmento. Você configurou a tolerância em Configurações &gt; Propriedades dos Projeto &gt; Geral?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="219"/>
        <source>Could not snap vertex. Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>Impossível quebrar o vértice. Você configurou a tolerância em Configurações &gt; Propriedades dos Projeto &gt; Geral?</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="769"/>
        <source>Project file read error: </source>
        <translation>Erro na leitura do arquivo de projeto: </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="32"/>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>Extender para uma tranformação linear requer no mínimo 2 pontos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="71"/>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Extender para uma transformação Helmet requer um mínimo de 2 pontos.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="123"/>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Estender para uma transformação afim requer no mínimo 4 pontos.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="330"/>
        <source>Couldn&apos;t open the data source: </source>
        <translation>Não foi possível abrir a fonte de dados:</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="352"/>
        <source>Parse error at line </source>
        <translation>Erro de análise na linha </translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="53"/>
        <source>GPS eXchange format provider</source>
        <translation>Provedor de formato de troca GPS</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="311"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="402"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="156"/>
        <source>GRASS plugin</source>
        <translation>Plugin do GRASS</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="129"/>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>QGIS não encontrou sua instalação do GRASS. 
Você gostaria de especificar um caminhos (GISBASE) para sua instalação GRASS?</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="143"/>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Escolha o caminho de instalação do GRASS (GISBASE)</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="157"/>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>Dados do GRASS não serão habilitados se o GISBASE não for especificado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="51"/>
        <source>CopyrightLabel</source>
        <translation>Etiqueta de Copyright </translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="52"/>
        <source>Draws copyright information</source>
        <translation>Infomações do copyright do desenho</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="30"/>
        <source>Version 0.1</source>
        <translation>Versão 0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="44"/>
        <source>Version 0.2</source>
        <translation>Versão 0.2</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="45"/>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation>Carrega e mostra arquivos de texto delimitados contendo coordenadas x e y</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="161"/>
        <source>Add Delimited Text Layer</source>
        <translation>Adiciona uma camada de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="57"/>
        <source>Georeferencer</source>
        <translation>Georreferenciador</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="58"/>
        <source>Adding projection info to rasters</source>
        <translation>Adicionando informação de projeção para os rasters</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="54"/>
        <source>GPS Tools</source>
        <translation type="unfinished">Ferramentas de GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="56"/>
        <source>Tools for loading and importing GPS data</source>
        <translation>Ferramentas para carregar e descarregar dados de GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="842"/>
        <source>GRASS</source>
        <translation type="unfinished">GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="848"/>
        <source>GRASS layer</source>
        <translation>camada do GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="43"/>
        <source>Graticule Creator</source>
        <translation>Criador de Grade</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="44"/>
        <source>Builds a graticule</source>
        <translation>Constrói uma grade</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="58"/>
        <source>NorthArrow</source>
        <translation>Seta Norte</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="59"/>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Mostra uma seta Norte sobreposta ao mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="38"/>
        <source>[menuitemname]</source>
        <translation>{nomedoitemdomenu]</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="39"/>
        <source>[plugindescription]</source>
        <translation>[descriçãodoplugin]</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="62"/>
        <source>ScaleBar</source>
        <translation>Barra de escala</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="63"/>
        <source>Draws a scale bar</source>
        <translation>Desenha uma barra de escala</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="37"/>
        <source>SPIT</source>
        <translation>SPIT</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="38"/>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>Ferramenta para importar um Shapefile para PostgreSQL/PostGIS </translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="28"/>
        <source>WFS plugin</source>
        <translation>Plugin WFS</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="29"/>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>Adiciona uma camada WFS para a tela do QGIS</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Version 0.0001</source>
        <translation type="obsolete">Versão 0.0001</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="391"/>
        <source>Not a vector layer</source>
        <translation>Não é uma camada vetorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="392"/>
        <source>The current layer is not a vector layer</source>
        <translation>A camada atual não é uma camada vetorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="69"/>
        <source>Layer cannot be added to</source>
        <translation>Camada não pode ser adicionada para</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="70"/>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>O provedor de dados para esta camada não suporta a adição de feições.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="405"/>
        <source>Layer not editable</source>
        <translation type="unfinished">A camada não pode ser editada</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="51"/>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>Camada bloqueada para edição. Para tornar editável você precisa clicar com o botão direito do mause e habilitar &apos;Permitir edição&apos;.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="76"/>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Para selecionar feições você precisa escolher uma camada clicando em seu nome na legenda</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="400"/>
        <source>Python error</source>
        <translation>Erro Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="57"/>
        <source>Couldn&apos;t load SIP module.
Python support will be disabled.</source>
        <translation>Não consegui carregar o módulo SIP. 
O suporte Python será desabilitado.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="67"/>
        <source>Couldn&apos;t load PyQt bindings.
Python support will be disabled.</source>
        <translation>Couldn&apos;t load PyQt bindings.
O suporte Python será desabilitado.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="78"/>
        <source>Couldn&apos;t load QGIS bindings.
Python support will be disabled.</source>
        <translation>Couldn&apos;t load QGIS bindings. 
O suporte Python será desabilitado.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="380"/>
        <source>Couldn&apos;t load plugin </source>
        <translation>Não consegui carregar o plugin </translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="369"/>
        <source> due an error when calling its classFactory() method</source>
        <translation> devido um erro when calling its classFactory() method</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="381"/>
        <source> due an error when calling its initGui() method</source>
        <translation> devido a um erro ao chamar seu método Gui()</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="401"/>
        <source>Error while unloading plugin </source>
        <translation>Erro enquanto descarregava o plugin</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="56"/>
        <source>2.5D shape type not supported</source>
        <translation>Shape tipo 2.5D não suportado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="56"/>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>Adicionar feições para shapes tipo 2.5D ainda não suportada</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="194"/>
        <source>Wrong editing tool</source>
        <translation>Ferramanta de edição errada</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="92"/>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>Impossível aplicar a ferramenta &apos;ponto de captura&apos; nesta camada vetorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="65"/>
        <source>Coordinate transform error</source>
        <translation>Erro na transformação da coordenada</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="66"/>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>Impossível transformar o ponto para o sistema de coordenadas da camada</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="187"/>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>Impossível aplicar a ferramenta &apos;capturar linha&apos; nesta camada vetoria</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="195"/>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>Impossível aplicar a ferramenta &apos;capturar polígono&apos; nesta camada vetorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="218"/>
        <source>Error</source>
        <translation>Erro</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="395"/>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>Impossível adicionar feição. Tipo WKB desconhecido</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdisland.cpp" line="110"/>
        <source>Error, could not add island</source>
        <translation>Erro, impossível adicionar ilha</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="90"/>
        <source>A problem with geometry type occured</source>
        <translation>Ocorreu um problema com o tipo de geometria</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="94"/>
        <source>The inserted Ring is not closed</source>
        <translation>O Anel inserido não está fechado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="98"/>
        <source>The inserted Ring is not a valid geometry</source>
        <translation>O Anel inserido não é uma geometria válida</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="102"/>
        <source>The inserted Ring crosses existing rings</source>
        <translation>O Anel inserido cruza anéis existentes</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="106"/>
        <source>The inserted Ring is not contained in a feature</source>
        <translation>O Anel inserido não está contido na feição</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="110"/>
        <source>An unknown error occured</source>
        <translation>Ocorreu um erro desconhecido</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="112"/>
        <source>Error, could not add ring</source>
        <translation>Erro, não posso adicionar anel</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="398"/>
        <source>Change geometry</source>
        <translation>Mudar geometria</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="399"/>
        <source>Data provider of the current layer doesn&apos;t allow changing geometries</source>
        <translation>O provedor de dados da camada atual não permite mudar geometrias</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="406"/>
        <source>Cannot edit the vector layer. Use &apos;Start editing&apos; in the legend item menu</source>
        <translation>Impossível editar a camada vetorial. Use &apos;Iniciar edição&apos; no menu da legenda</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="648"/>
        <source> km2</source>
        <translation> km2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="653"/>
        <source> ha</source>
        <translation> ha</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="658"/>
        <source> m2</source>
        <translation> m2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="686"/>
        <source> m</source>
        <translation> m</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="671"/>
        <source> km</source>
        <translation type="unfinished"> km</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="676"/>
        <source> mm</source>
        <translation type="unfinished"> mm</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="681"/>
        <source> cm</source>
        <translation type="unfinished"> cm</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="695"/>
        <source> sq mile</source>
        <translation> sq mile</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="700"/>
        <source> sq ft</source>
        <translation> sq ft</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="707"/>
        <source> mile</source>
        <translation> milha</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="713"/>
        <source> foot</source>
        <translation type="unfinished"> pés</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="715"/>
        <source> feet</source>
        <translation type="unfinished">pés</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="722"/>
        <source> sq.deg.</source>
        <translation> sq deg.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="727"/>
        <source> degree</source>
        <translation type="unfinished"> graus</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="729"/>
        <source> degrees</source>
        <translation type="unfinished"> graus</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="733"/>
        <source> unknown</source>
        <translation type="unfinished"> desconhecido</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="269"/>
        <source>Received %1 of %2 bytes</source>
        <translation>Recebidos %1 de %2 bytes</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="275"/>
        <source>Received %1 bytes (total unknown)</source>
        <translation>Recebidos %1 bytes (total desconhecido)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="386"/>
        <source>Not connected</source>
        <translation>Não conectado</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="392"/>
        <source>Looking up &apos;%1&apos;</source>
        <translation>Olhando para &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="399"/>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>Conectando a &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="406"/>
        <source>Sending request &apos;%1&apos;</source>
        <translation>Enviando requisição &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="413"/>
        <source>Receiving reply</source>
        <translation>Recebendo resposta</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="419"/>
        <source>Response is complete</source>
        <translation>Resposta completa</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="425"/>
        <source>Closing down connection</source>
        <translation>Fechando a conexão</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="751"/>
        <source>Unable to open </source>
        <translation>Impossível abrir</translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="253"/>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation>Expressões regulares nos valores numéricos não têm senso. Ao invés disso, use comparação.</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>PostgresSQL Geoprocessing</source>
        <translation type="obsolete">Geoprocessando POstgresSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="48"/>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Funções de geoprocessamento para trabalhar com camadas PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="136"/>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Local: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="136"/>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Maset: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="140"/>
        <source>Location: </source>
        <translation>Local: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="140"/>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Mapset: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="145"/>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation>&lt;b&gt;Raster&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="154"/>
        <source>Cannot open raster header</source>
        <translation>Impossível abrir cabaçalho raster</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="158"/>
        <source>Rows</source>
        <translation type="unfinished">linhas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="159"/>
        <source>Columns</source>
        <translation>Colunas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="160"/>
        <source>N-S resolution</source>
        <translation>Resolução N-S</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="161"/>
        <source>E-W resolution</source>
        <translation>Resolução E-W</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="299"/>
        <source>North</source>
        <translation>Norte</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="301"/>
        <source>South</source>
        <translation>Sul</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="303"/>
        <source>East</source>
        <translation>Leste</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="305"/>
        <source>West</source>
        <translation>Oeste</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="188"/>
        <source>Format</source>
        <translation>Formatar</translation>
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
        <translation>Fonte de dados</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="216"/>
        <source>Data description</source>
        <translation>Decrição de dados</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="225"/>
        <source>Comments</source>
        <translation>Comentários</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="240"/>
        <source>Categories</source>
        <translation>Categorias</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="346"/>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;Vetor&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="273"/>
        <source>Points</source>
        <translation type="unfinished">Pontos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="274"/>
        <source>Lines</source>
        <translation>Linhas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="275"/>
        <source>Boundaries</source>
        <translation>Limites</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="276"/>
        <source>Centroids</source>
        <translation>Centróides</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="279"/>
        <source>Faces</source>
        <translation>Faces</translation>
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
        <translation>Ilhas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="308"/>
        <source>Top</source>
        <translation>Topo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="309"/>
        <source>Bottom</source>
        <translation>Base</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="312"/>
        <source>yes</source>
        <translation>sim</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="312"/>
        <source>no</source>
        <translation>não</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="319"/>
        <source>History&lt;br&gt;</source>
        <translation>Histórico&lt;br&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="347"/>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Camada&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="366"/>
        <source>Features</source>
        <translation type="unfinished">Feições</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="375"/>
        <source>Driver</source>
        <translation>Driver</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="376"/>
        <source>Database</source>
        <translation type="unfinished">Banco de Dados</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="377"/>
        <source>Table</source>
        <translation type="unfinished">Tabela</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="378"/>
        <source>Key column</source>
        <translation>Coluna chave</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="413"/>
        <source>GISBASE is not set.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="418"/>
        <source> is not a GRASS mapset.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="440"/>
        <source>Cannot start </source>
        <translation>Impossível iniciar</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="457"/>
        <source>Mapset is already in use.</source>
        <translation>Mapset já em uso. </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="472"/>
        <source>Temporary directory </source>
        <translation>Diretório temporário </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="472"/>
        <source> exist but is not writable</source>
        <translation> existe mas não é gravável</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="478"/>
        <source>Cannot create temporary directory </source>
        <translation>Impossível criar diretório temporário </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="494"/>
        <source>Cannot create </source>
        <translation>Impossível criar</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="567"/>
        <source>Cannot remove mapset lock: </source>
        <translation>Impossível remover mapset bloqueado: </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1007"/>
        <source>Warning</source>
        <translation>Aviso</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="955"/>
        <source>Cannot read raster map region</source>
        <translation>Impossível ler região do mapa raster</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="972"/>
        <source>Cannot read vector map region</source>
        <translation>Impossível ler região do mapa vetorial</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1008"/>
        <source>Cannot read region</source>
        <translation>Impossível ler região</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2337"/>
        <source>Where is &apos;</source>
        <translation>Onde está &apos;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2337"/>
        <source>original location: </source>
        <translation>local original: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="122"/>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Para identificar feições, você precisa escolher uma camada ativa na legenda clicando no seu nome</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="47"/>
        <source>PostgreSQL Geoprocessing</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="331"/>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS -</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1409"/>
        <source>Version </source>
        <translation>Versão</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1416"/>
        <source> with PostgreSQL support</source>
        <translation>com suporte a PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1419"/>
        <source> (no PostgreSQL support)</source>
        <translation>(sem suporte a PostgreSQL)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1436"/>
        <source>Version</source>
        <translation>Versão</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1476"/>
        <source>Available Data Provider Plugins</source>
        <translation>Plugins disponíveis de acesso a dados</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1987"/>
        <source>is not a valid or recognized data source</source>
        <translation>não é uma fonte de dados válida ou conhecida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5040"/>
        <source>Invalid Data Source</source>
        <translation>Fonte de Dados Inválida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2081"/>
        <source>Invalid Layer</source>
        <translation>Camada Inválida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2081"/>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>%1 é uma camada inválida e não pode ser carregada.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3361"/>
        <source>No Layer Selected</source>
        <translation>Nenhuma Camada Selecionada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3851"/>
        <source>Error Loading Plugin</source>
        <translation>Erro Carregando Plugin</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3851"/>
        <source>There was an error loading %1.</source>
        <translation>Ocorreu um erro ao carregar %1.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3887"/>
        <source>No MapLayer Plugins</source>
        <translation>Nenhum plugin MapLayer encontrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3887"/>
        <source>No MapLayer plugins in ../plugins/maplayer</source>
        <translation>Nenhum plugin MapLayer encontrado em ../plugins/maplayer</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3964"/>
        <source>No Plugins</source>
        <translation>Nenhum plugin encontrado</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>No plugins found in ../plugins. To test plugins, start qgis &gt;from the src directory</source>
        <translation type="obsolete">Nenhum plugin encontrado em ../plugins. Para testar os plugins, inicie o qgis a partir do diretório src</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3999"/>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3999"/>
        <source>Plugin %1 is named %2</source>
        <translation>Plugin %1 chama-se %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4016"/>
        <source>Plugin Information</source>
        <translation>Informação sobre o plugin</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4017"/>
        <source>QGis loaded the following plugin:</source>
        <translation>QGIS carregou o seguinte plugins:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4017"/>
        <source>Name: %1</source>
        <translation>Nome: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4017"/>
        <source>Version: %1</source>
        <translation>Versão: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4018"/>
        <source>Description: %1</source>
        <translation>Descrição: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4036"/>
        <source>Unable to Load Plugin</source>
        <translation>Não foi possível carregar o plugin</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4037"/>
        <source>QGIS was unable to load the plugin from: %1</source>
        <translation>QGIS não conseguiu carregar o plugin a partir de: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4093"/>
        <source>There is a new version of QGIS available</source>
        <translation>Existe uma nova versão do QGIS disponível</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4099"/>
        <source>You are running a development version of QGIS</source>
        <translation>Você está executando uma versão de desenvolvimento do QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4103"/>
        <source>You are running the current version of QGIS</source>
        <translation>Você está rodando a versão atual do QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4108"/>
        <source>Would you like more information?</source>
        <translation>Gostaria de obter mais informações?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4155"/>
        <source>QGIS Version Information</source>
        <translation>Informações sobre a versão do QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4127"/>
        <source>Unable to get current version information from server</source>
        <translation>Impossível obter informações sobre a versão atual</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4141"/>
        <source>Connection refused - server may be down</source>
        <translation>Conexão recusada - o servidor pode estar indisponível</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4144"/>
        <source>QGIS server was not found</source>
        <translation>Servidor do QGIS não foi encontrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3188"/>
        <source>Saved map image to</source>
        <translation>Imagem de mapa salva em</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3146"/>
        <source>Choose a filename to save the map image as</source>
        <translation>Escolha um nome do arquivo para salvar a imagem do mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4589"/>
        <source>Extents: </source>
        <translation>Extensão:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3391"/>
        <source>Problem deleting features</source>
        <translation>Problema ao excluir feições</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3392"/>
        <source>A problem occured during deletion of features</source>
        <translation>Um problema ocorreu durante a exclusão das feições</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3369"/>
        <source>No Vector Layer Selected</source>
        <translation>Nenhuma camada vetorial selecionada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3370"/>
        <source>Deleting features only works on vector layers</source>
        <translation>Apagar feições funciona apenas em camadas vetoriais</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3362"/>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation>Para excluir feições. você precisa selecionar uma camada vetorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1432"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation>Quantum GIS está sob a licença GNU General Public License</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1434"/>
        <source>http://www.gnu.org/licenses</source>
        <translation>http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1288"/>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>A legenda do mapa mostra todas as camadas na área do mapa. Clique na caixa para ativar ou desativar a camada. Duplo clique em uma camada em sua legenda serve para customizar sua aparência e ajustar outras propriedades.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1255"/>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Visão geral da área do mapa. Esta área pode ser usado para mostrar um localizador na área total do mapa. A extensão atual é mostrada como um retângulo vermelho. Qualquer camada do mapa pode ser adicionada para a visão geral.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1210"/>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation>Área do mapa. Onde camadas vetoriais e raster são exibidas quando adicionadas ao mapa.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="898"/>
        <source>&amp;Plugins</source>
        <translation>&amp;Plugins</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1034"/>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>A barra de progresso mostra o status da renderização das camadas e outras operações que levam muito tempo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1059"/>
        <source>Displays the current map scale</source>
        <translation>Exibe a escala atual do mapa</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Shows the map coordinates at the current cursor postion. The display is continuously updated as the mouse is moved.</source>
        <translation type="obsolete">Mostra as coordenadas do mapa na posição atual do cursor. O mostrador é atualizado continuamente com o deslocamento do mouse.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1074"/>
        <source>Render</source>
        <translation>Desenhar</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1077"/>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Quando marcadas, as camadas do mapa são renderezidas em resposta aos comandos de navegação pelo mapa e outros eventos. Quando não marcadas, nenhuma renderização será feita. Isso permite que você adicione um grande número de camadas e as altere antes de renderizá-las.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2957"/>
        <source>Unable to save project</source>
        <translation>Incapaz de salvar projeto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2958"/>
        <source>Unable to save project to </source>
        <translation>Incapaz de salvar projeto em</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1078"/>
        <source>Toggle map rendering</source>
        <translation>Ativa a renderização do mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1093"/>
        <source>This icon shows whether on the fly projection is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation>Este ícone mostra se as projeções em tempo real estão ativadas ou não. Clique neste ícone e visualize as propriedades da projeção e altere o seu comportamento.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1094"/>
        <source>Projection status - Click to open projection dialog</source>
        <translation>Situação da projeção - Clique para abrir a janela de projeção</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1853"/>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>Abrir uma camada vetorial OGR suportada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2776"/>
        <source>QGIS Project Read Error</source>
        <translation>Erro de leitura do projeto QGIS </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2778"/>
        <source>Try to find missing layers?</source>
        <translation>Tento encontrar as camadas que estão faltando?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4925"/>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>Abrir uma fonte de dados raster GDAL suportada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="306"/>
        <source>Reading settings</source>
        <translation>Lendo características</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="310"/>
        <source>Setting up the GUI</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="300"/>
        <source>Checking database</source>
        <translation>Checando base de dados</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="359"/>
        <source>Restoring loaded plugins</source>
        <translation>Restaurar plugins carregados</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="363"/>
        <source>Initializing file filters</source>
        <translation>Iniciar filtros de arquivo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="388"/>
        <source>Restoring window state</source>
        <translation>Restaurar estado da janela</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="392"/>
        <source>QGIS Ready!</source>
        <translation>QGIS pronto!</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="454"/>
        <source>&amp;New Project</source>
        <translation type="unfinished">&amp;Novo Projeto</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Ctrl+N</source>
        <comment>







New Project</comment>
        <translation type="obsolete">Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="456"/>
        <source>New Project</source>
        <translation type="unfinished">Novo Projeto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="459"/>
        <source>&amp;Open Project...</source>
        <translation>&amp;Abrir Projeto...</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Ctrl+O</source>
        <comment>







Open a Project</comment>
        <translation type="obsolete">Ctrl+O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="461"/>
        <source>Open a Project</source>
        <translation>Abre um Projeto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="464"/>
        <source>&amp;Save Project</source>
        <translation type="unfinished">Salvar &amp;Projeto</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Ctrl+S</source>
        <comment>







Save Project</comment>
        <translation type="obsolete">Ctrl+S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="466"/>
        <source>Save Project</source>
        <translation type="unfinished">Salvar Projeto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="469"/>
        <source>Save Project &amp;As...</source>
        <translation type="unfinished">Salvar Projeto &amp;Como...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="470"/>
        <source>Ctrl+A</source>
        <comment>Save Project under a new name</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="471"/>
        <source>Save Project under a new name</source>
        <translation>Salva Projeto com um novo nome</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="474"/>
        <source>&amp;Print...</source>
        <translation type="unfinished">&amp;Imprimir</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Ctrl+P</source>
        <comment>







Print</comment>
        <translation type="obsolete">Ctrl+P</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="476"/>
        <source>Print</source>
        <translation type="unfinished">Imprimir</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="479"/>
        <source>Save as Image...</source>
        <translation>Salva como imagem...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="480"/>
        <source>Ctrl+I</source>
        <comment>Save map as image</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="481"/>
        <source>Save map as image</source>
        <translation>Salva mapa como imagem</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="484"/>
        <source>Export to MapServer Map...</source>
        <translation>Exportar para MapServer Map...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="485"/>
        <source>M</source>
        <comment>Export as MapServer .map file</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="486"/>
        <source>Export as MapServer .map file</source>
        <translation>Exporta como arquifo MapServer (.map) </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="489"/>
        <source>Exit</source>
        <translation type="unfinished">Sair</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="490"/>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="491"/>
        <source>Exit QGIS</source>
        <translation>Sair do QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="496"/>
        <source>Add a Vector Layer...</source>
        <translation>Addicionar uma camada vetorial</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>V</source>
        <comment>







Add a Vector Layer</comment>
        <translation type="obsolete">V</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="498"/>
        <source>Add a Vector Layer</source>
        <translation>Adiciona uma Camada Vetorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="501"/>
        <source>Add a Raster Layer...</source>
        <translation>Addicionar uma camada Raster</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>R</source>
        <comment>







Add a Raster Layer</comment>
        <translation type="obsolete">R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="503"/>
        <source>Add a Raster Layer</source>
        <translation type="unfinished">Adicionar Camada Raster</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="506"/>
        <source>Add a PostGIS Layer...</source>
        <translation>Adicionar uma camada PostGIS</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>D</source>
        <comment>







Add a PostGIS Layer</comment>
        <translation type="obsolete">D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="508"/>
        <source>Add a PostGIS Layer</source>
        <translation>Adiciona uma Camada PostGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="518"/>
        <source>New Vector Layer...</source>
        <translation>Nova camada vetorial</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>N</source>
        <comment>







Create a New Vector Layer</comment>
        <translation type="obsolete">N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="520"/>
        <source>Create a New Vector Layer</source>
        <translation>Criar .uma nova camada vetorial</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="523"/>
        <source>Remove Layer</source>
        <translation type="unfinished">Remover camada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="524"/>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="525"/>
        <source>Remove a Layer</source>
        <translation>Remover uma camada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="528"/>
        <source>Add All To Overview</source>
        <translation type="unfinished">Adicionar tudo para o Overview</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>+</source>
        <comment>







Show all layers in the overview map</comment>
        <translation type="obsolete">+</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="530"/>
        <source>Show all layers in the overview map</source>
        <translation>Mostrar todas as camadas no &apos;overview map&apos;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="533"/>
        <source>Remove All From Overview</source>
        <translation type="unfinished">Remover tudo do Overview</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>-</source>
        <comment>







Remove all layers from overview map</comment>
        <translation type="obsolete">-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="535"/>
        <source>Remove all layers from overview map</source>
        <translation>Remover todas as camadas do &apos;overview map&apos;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="538"/>
        <source>Show All Layers</source>
        <translation type="unfinished">Mostrar Todas as Camadas</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>S</source>
        <comment>







Show all layers</comment>
        <translation type="obsolete">S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="540"/>
        <source>Show all layers</source>
        <translation type="unfinished">Exibir todas camadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="543"/>
        <source>Hide All Layers</source>
        <translation type="unfinished">Ocultar Todas as Camadas</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>H</source>
        <comment>







Hide all layers</comment>
        <translation type="obsolete">H</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="545"/>
        <source>Hide all layers</source>
        <translation>Oculta todas as camadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="550"/>
        <source>Project Properties...</source>
        <translation>Propriedades do projeto...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="551"/>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="552"/>
        <source>Set project properties</source>
        <translation>Gerencie as propriedades do projeto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="555"/>
        <source>Options...</source>
        <translation>Opções...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="557"/>
        <source>Change various QGIS options</source>
        <translation>Modificar várias opções do QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="560"/>
        <source>Custom Projection...</source>
        <translation>Projeção personalizada...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="562"/>
        <source>Manage custom projections</source>
        <translation>Gerencia projeções personalizadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="567"/>
        <source>Help Contents</source>
        <translation>Conteúdo da Ajuda</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>F1</source>
        <comment>







Help Documentation</comment>
        <translation type="obsolete">F1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="573"/>
        <source>Help Documentation</source>
        <translation>Documentação da ajuda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="576"/>
        <source>Qgis Home Page</source>
        <translation>Página do QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="578"/>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="580"/>
        <source>QGIS Home Page</source>
        <translation type="unfinished">Página do QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="583"/>
        <source>About</source>
        <translation type="unfinished">Sobre</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="584"/>
        <source>About QGIS</source>
        <translation>Sobreo o QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="587"/>
        <source>Check Qgis Version</source>
        <translation>Checar a versão do QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="588"/>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Checar se seu QGIS é a versão atual (requer acesso a internet)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="593"/>
        <source>Refresh</source>
        <translation type="unfinished">Atualizar</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="594"/>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="595"/>
        <source>Refresh Map</source>
        <translation>Atualizar mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="600"/>
        <source>Zoom In</source>
        <translation>Aproximar</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="599"/>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="605"/>
        <source>Zoom Out</source>
        <translation type="unfinished">Afastar</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="604"/>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="608"/>
        <source>Zoom Full</source>
        <translation>Ver tudo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="609"/>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="610"/>
        <source>Zoom to Full Extents</source>
        <translation>Ver a toda a extensão</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="613"/>
        <source>Zoom To Selection</source>
        <translation>Ver a seleção</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="614"/>
        <source>Ctrl+F</source>
        <comment>Zoom to selection</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="615"/>
        <source>Zoom to selection</source>
        <translation>Ver a selação</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="618"/>
        <source>Pan Map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="619"/>
        <source>Pan the map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="622"/>
        <source>Zoom Last</source>
        <translation>Última vizualização</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="624"/>
        <source>Zoom to Last Extent</source>
        <translation>Ver extenção anterior</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="627"/>
        <source>Zoom To Layer</source>
        <translation>Ver a camada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="629"/>
        <source>Zoom to Layer</source>
        <translation>Ver a camada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="632"/>
        <source>Identify Features</source>
        <translation>Identifica feições</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="633"/>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="634"/>
        <source>Click on features to identify them</source>
        <translation>Clique nas feições para identificá-las</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="638"/>
        <source>Select Features</source>
        <translation>Selecionar feições</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="644"/>
        <source>Open Table</source>
        <translation>Abrir Tabela</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="648"/>
        <source>Measure Line </source>
        <translation>Medir linha</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Ctrl+M</source>
        <comment>







Measure a Line</comment>
        <translation type="obsolete">Ctrl+M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="650"/>
        <source>Measure a Line</source>
        <translation>Mede uma linha</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="653"/>
        <source>Measure Area</source>
        <translation>Medir Área</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="654"/>
        <source>Ctrl+J</source>
        <comment>Measure an Area</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="655"/>
        <source>Measure an Area</source>
        <translation>Mede uma área</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="660"/>
        <source>Show Bookmarks</source>
        <translation>Mostra Favoritos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="659"/>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="675"/>
        <source>New Bookmark...</source>
        <translation>Novo Favorito</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Ctrl+B</source>
        <comment>







New Bookmark</comment>
        <translation type="obsolete">Ctrl+B</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5303"/>
        <source>New Bookmark</source>
        <translation>Novo Favorito</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="680"/>
        <source>Add WMS Layer...</source>
        <translation>Adiciona camada WMS</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>W</source>
        <comment>







Add Web Mapping Server Layer</comment>
        <translation type="obsolete">W</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="682"/>
        <source>Add Web Mapping Server Layer</source>
        <translation>Adiciona camada Web Mapping Server</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="685"/>
        <source>In Overview</source>
        <translation>No Overview</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="686"/>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="687"/>
        <source>Add current layer to overview map</source>
        <translation>Adiciona a camada ativa ao overview map</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="692"/>
        <source>Plugin Manager...</source>
        <translation>Gerenciador de Plugin...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="694"/>
        <source>Open the plugin manager</source>
        <translation>Abrir o gerenciador de plugin</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="711"/>
        <source>Capture Point</source>
        <translation type="unfinished">Capturar Ponto</translation>
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
        <translation>Captura pontos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="717"/>
        <source>Capture Line</source>
        <translation type="unfinished">Capturar linha</translation>
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
        <translation>Captura linhas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="723"/>
        <source>Capture Polygon</source>
        <translation type="unfinished">Capturar polígono</translation>
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
        <translation>Captura polígonos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="730"/>
        <source>Delete Selected</source>
        <translation>Excluir seleção</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="735"/>
        <source>Add Vertex</source>
        <translation>Adicionar Vértice</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="740"/>
        <source>Delete Vertex</source>
        <translation>Exclui vértice</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="745"/>
        <source>Move Vertex</source>
        <translation>Mover vértice</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="833"/>
        <source>&amp;File</source>
        <translation type="unfinished">&amp;Arquivo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="836"/>
        <source>&amp;Open Recent Projects</source>
        <translation>&amp;Abrir projetos recentes</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="853"/>
        <source>&amp;View</source>
        <translation type="unfinished">&amp;Exibir</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="871"/>
        <source>&amp;Layer</source>
        <translation type="unfinished">&amp;Camada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="891"/>
        <source>&amp;Settings</source>
        <translation type="unfinished">&amp;Configurações</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="914"/>
        <source>&amp;Help</source>
        <translation type="unfinished">&amp;Ajuda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="931"/>
        <source>File</source>
        <translation>Arquivo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="942"/>
        <source>Manage Layers</source>
        <translation>Gerenciar camadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="960"/>
        <source>Help</source>
        <translation>Ajuda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="967"/>
        <source>Digitizing</source>
        <translation>Digitalizar</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="985"/>
        <source>Map Navigation</source>
        <translation>Navegar no mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="998"/>
        <source>Attributes</source>
        <translation>Atributos</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1010"/>
        <source>Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1098"/>
        <source>Ready</source>
        <translation>Pronto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1438"/>
        <source>New features</source>
        <translation>Novas feições</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2564"/>
        <source>Save As</source>
        <translation>Salvar como</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2665"/>
        <source>Choose a QGIS project file to open</source>
        <translation type="unfinished">Escolha um projeto QGIS para abrir</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2794"/>
        <source>Unable to open project</source>
        <translation>Impossível abrir projeto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2833"/>
        <source>Choose a QGIS project file</source>
        <translation type="unfinished">Escolha um projeto do QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2950"/>
        <source>Saved project to:</source>
        <translation>Projeto salvo em:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2964"/>
        <source>Unable to save project </source>
        <translation>Impossível salvar projeto </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2906"/>
        <source>Choose a filename to save the QGIS project file as</source>
        <translation>Escolha o nome do arquivo para salvar o projeto do QGIS como</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3022"/>
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: Impossível carregar projeto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3023"/>
        <source>Unable to load project </source>
        <translation>Impossível carregar projeto </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4115"/>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS - Mudificações no SVN desde o último lançamento</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4234"/>
        <source>QGIS Browser Selection</source>
        <translation>Seleção de Navegador para o QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4235"/>
        <source>Enter the name of a web browser to use (eg. konqueror).
</source>
        <translation>Entre com o nome do navegador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4236"/>
        <source>Enter the full path if the browser is not in your PATH.
</source>
        <translation>Entre com o caminho completo se o seu navegador não está neste local.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4237"/>
        <source>You can change this option later by selecting Options from the Settings menu (Help Browser tab).</source>
        <translation>Você pode modificar esta opção mais tarde selecionando Oções no menu Configurações</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5138"/>
        <source>Layer is not valid</source>
        <translation>Camada não é válida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5139"/>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>A camada não é uma camada válida e não pode ser adicionada ao mapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4402"/>
        <source>Save?</source>
        <translation>Salvar?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4876"/>
        <source>Clipboard contents set to: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5039"/>
        <source> is not a valid or recognized raster data source</source>
        <translation>é uma fonte de dados raster não reconhecida ou inválida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5217"/>
        <source> is not a supported raster data source</source>
        <translation>é uma fonte de dados raster não suportada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5218"/>
        <source>Unsupported Data Source</source>
        <translation>Fonte de dados não suportada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5304"/>
        <source>Enter a name for the new bookmark:</source>
        <translation>Entre com o nome para o novo favorito: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5321"/>
        <source>Error</source>
        <translation>Erro</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5321"/>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>Impossível criar o favorito. Seu usuário pode estar perdido ou corrompido</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="569"/>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="665"/>
        <source>Show most toolbars</source>
        <translation>Mostrar mais barras de ferramentas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="671"/>
        <source>Hide most toolbars</source>
        <translation>Ocultar mais barras de ferramentas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="759"/>
        <source>Cut Features</source>
        <translation>cortar feições</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="760"/>
        <source>Cut selected features</source>
        <translation>Corta feições selecionadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="764"/>
        <source>Copy Features</source>
        <translation>Copiar feições</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="765"/>
        <source>Copy selected features</source>
        <translation>Copia feições elecionadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="769"/>
        <source>Paste Features</source>
        <translation>Cola feições</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="770"/>
        <source>Paste selected features</source>
        <translation>Cola feições selecionadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1421"/>
        <source>
Compiled against Qt </source>
        <translation>
Compilado com Qt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1422"/>
        <source>, running against Qt </source>
        <translation>, rodando contra Qt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4147"/>
        <source>Network error while communicating with server</source>
        <translation>Erro na comunicação enquanto comunica com o servidor</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4150"/>
        <source>Unknown network socket error</source>
        <translation>Erro de encaixe de rede desconhecido</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4155"/>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>Impossível a comunicação com esta versão do QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="664"/>
        <source>T</source>
        <comment>Show most toolbars</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Ctrl+T</source>
        <comment>







Hide most toolbars</comment>
        <translation type="obsolete">Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="344"/>
        <source>Checking provider plugins</source>
        <translation>Verificando provedor de plugins</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="349"/>
        <source>Starting Python</source>
        <translation>Iniciando Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="775"/>
        <source>Python console</source>
        <translation>Console Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1575"/>
        <source>Python error</source>
        <translation>Erro Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1575"/>
        <source>Error when reading metadata of plugin </source>
        <translation>Erro ao ler metadados do plugin</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3376"/>
        <source>Provider does not support deletion</source>
        <translation type="unfinished">O provedor não suporta apagar o arquivo</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3377"/>
        <source>Data provider does not support deleting features</source>
        <translation type="unfinished">O provedor de dados não suporta apagar feições</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3383"/>
        <source>Layer not editable</source>
        <translation type="unfinished">A camada não pode ser editada</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3384"/>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation>A camada atual não é editável. Escolha &apos;Iniciar edição&apos; na barra da ferramentas de digitalização</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="705"/>
        <source>Toggle editing</source>
        <translation>Alternar edição</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="706"/>
        <source>Toggles the editing state of the current layer</source>
        <translation>Alterna o estado de edição da camada ativa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="750"/>
        <source>Add Ring</source>
        <translation>Adiciona anel</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="754"/>
        <source>Add Island</source>
        <translation>Adiciona ilha</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="755"/>
        <source>Add Island to multipolygon</source>
        <translation>Adiciona ilha ao multipolígono</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1020"/>
        <source>Toolbar Visibility...</source>
        <translation>Visibilidade da barra de ferramentas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1047"/>
        <source>Scale </source>
        <translation>Escala</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1060"/>
        <source>Current map scale (formatted as x:y)</source>
        <translation>Escala do mapa atual (formatada como x:y)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1071"/>
        <source>Map coordinates at mouse cursor position</source>
        <translation>Coordenadas onde o cursor do mouse se encontra</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3616"/>
        <source>Invalid scale</source>
        <translation>escala inválida</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4403"/>
        <source>Do you want to save the current project?</source>
        <translation>Você quer salvar o projeto atual?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1445"/>
        <source>Python bindings - This is the major focus of this release it is now possible to create plugins using python. It is also possible to create GIS enabled applications written in python that use the QGIS libraries.</source>
        <translation>Ligações Python - Possível criar aplicações GIS escritas em Python usando bibliotecas do QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1448"/>
        <source>Removed automake build system - QGIS now needs CMake for compilation.</source>
        <translation>Sistema de construção automático removido - Agora o QGIS necessita do CMake para compilação.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1451"/>
        <source>Many new GRASS tools added (with thanks to http://faunalia.it/)</source>
        <translation>Muitas ferramentas GRASS foram adicionadas (obrigado ao http://faunalia.it/)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1454"/>
        <source>Map Composer updates</source>
        <translation>Compositor de mapas atualizado</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1457"/>
        <source>Crash fix for 2.5D shapefiles</source>
        <translation>Quebra fixa para arquivos SHP 2.5D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1460"/>
        <source>The QGIS libraries have been refactored and better organised.</source>
        <translation>As bibliotecas QGIS foram reconstruídas e estão melhor organizadas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1463"/>
        <source>Improvements to the GeoReferencer</source>
        <translation>Melhorias no Georreferenciador</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="455"/>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation type="unfinished">Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="460"/>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation type="unfinished">Ctrl+O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="465"/>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation type="unfinished">Ctrl+S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="475"/>
        <source>Ctrl+P</source>
        <comment>Print</comment>
        <translation type="unfinished">Ctrl+P</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="497"/>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation type="unfinished">V</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="502"/>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation type="unfinished">R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="507"/>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation type="unfinished">D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="519"/>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation type="unfinished">N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="529"/>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation type="unfinished">+</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="534"/>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation type="unfinished">-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="539"/>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation type="unfinished">S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="544"/>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation type="unfinished">H</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="571"/>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation type="unfinished">F1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="649"/>
        <source>Ctrl+M</source>
        <comment>Measure a Line</comment>
        <translation type="unfinished">Ctrl+M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="670"/>
        <source>Ctrl+T</source>
        <comment>Hide most toolbars</comment>
        <translation type="unfinished">Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="676"/>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation type="unfinished">Ctrl+B</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="681"/>
        <source>W</source>
        <comment>Add Web Mapping Server Layer</comment>
        <translation type="unfinished">W</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1070"/>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1466"/>
        <source>Added locale options to options dialog.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3965"/>
        <source>No plugins found in ../plugins. To test plugins, start qgis from the src directory</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation>JanelaPrincipal</translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="102"/>
        <source>Legend</source>
        <translation>Legenda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="135"/>
        <source>Map View</source>
        <translation>Visão do mapa</translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="13"/>
        <source>About Quantum GIS</source>
        <translation>Sobre o Quantum GIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="300"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="32"/>
        <source>About</source>
        <translation>Sobre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="86"/>
        <source>Version</source>
        <translation>Versão</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="132"/>
        <source>What&apos;s New</source>
        <translation>O que há de Novo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="76"/>
        <source>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="111"/>
        <source>QGIS Home Page</source>
        <translation>Página do QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="121"/>
        <source>Subscribe to the QGIS-User mailing list</source>
        <translation>Increva-se na lista de discussão do QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="237"/>
        <source>Providers</source>
        <translation>Provedores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="161"/>
        <source>Developers</source>
        <translation>Desenvolvedores</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="229"/>
        <source>&lt;h2&gt;QGIS Developers&lt;/h2&gt;</source>
        <translation type="unfinished"></translation>
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
&lt;p align=&quot;center&quot; style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Quantum GIS é licenciado sobre a GNU General Public License&lt;/p&gt;
&lt;p align=&quot;center&quot; style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;http://www.gnu.org/licenses&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="253"/>
        <source>Sponsors</source>
        <translation>Patrocinadores</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="111"/>
        <source>QGIS Sponsors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="114"/>
        <source>The following have sponsored QGIS by contributing money to fund development and other project costs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="118"/>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="118"/>
        <source>Website</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="246"/>
        <source>QGIS Browser Selection</source>
        <translation type="unfinished">Seleção de Navegador para o QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="250"/>
        <source>Enter the name of a web browser to use (eg. konqueror).
Enter the full path if the browser is not in your PATH.
You can change this option later by selection Options from the Settings menu (Help Browser tab).</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAddAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="13"/>
        <source>Add Attribute</source>
        <translation>Adicionar atributo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="100"/>
        <source>Name:</source>
        <translation>Nome:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="87"/>
        <source>Type:</source>
        <translation>Tipo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="52"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="59"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="57"/>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="58"/>
        <source>Action</source>
        <translation type="unfinished">Ação</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="59"/>
        <source>Capture</source>
        <translation type="unfinished">Capture</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="142"/>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation>Seleciona uma ação</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="21"/>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="87"/>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>Esta lista contém todas ações que podem ser definidas para a camada atual. Adicione ações entrando com detalhes nos controles abaixo e então, pressione o botão de inserção. Ações podem ser editadas aqui através de um duplo clique.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="72"/>
        <source>Move up</source>
        <translation>Mover para cima</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="69"/>
        <source>Move the selected action up</source>
        <translation>Mover a ação selecionada para cima</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="62"/>
        <source>Move down</source>
        <translation>Mover para baixo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="59"/>
        <source>Move the selected action down</source>
        <translation>Mover a ação selecionada para baixo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="52"/>
        <source>Remove</source>
        <translation>Remover</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="49"/>
        <source>Remove the selected action</source>
        <translation>Remover a ação selecionada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="231"/>
        <source>Name:</source>
        <translation>Nome:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="228"/>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>Entre com o nome da ação aqui. O nome deve ser único (QGIS fará único se necessário).</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="218"/>
        <source>Enter the action name here</source>
        <translation>Entre com o nome da ação aqui</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="208"/>
        <source>Action:</source>
        <translation>Ação:</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation type="obsolete">Entre a ação aqui. Isto pode ser qualquer programa, script ou comando que estiver disponível no seu sistema. Quando a ação é executada qualquer caractere que começe com % e que tenha o nome do campo, terá seu nome trocado pelo valor do campo. O caractere especial %% será substituído pelo valor do campo que foi selecionado. Aspas duplas marcam grupos de texto em argumentos únicos para um programa, script ou comando. Aspas duplas serão ignoradas se precedidas por uma contra-barra</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="195"/>
        <source>Enter the action command here</source>
        <translation>Entre com o comando de ação aqui</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="129"/>
        <source>Browse</source>
        <translation>Exibir</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="126"/>
        <source>Browse for action commands</source>
        <translation>Visualise o comando de ação</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="175"/>
        <source>Insert action</source>
        <translation>Inserir ação</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="172"/>
        <source>Inserts the action into the list above</source>
        <translation>Insere a ação na lista acima</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="165"/>
        <source>Update action</source>
        <translation>Atualiza ação</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="162"/>
        <source>Update the selected action</source>
        <translation>Atualiza a ação selecionada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="139"/>
        <source>Insert field</source>
        <translation>Inserir campo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="136"/>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation>Insere o campo selecionado na ação, precedido por um %</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="119"/>
        <source>The valid attribute names for this layer</source>
        <translation>Os nomes de atributos válidos para esta camada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="188"/>
        <source>Capture output</source>
        <translation>Capture emissor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="182"/>
        <source>Captures any output from the action</source>
        <translation>Captura qualquer saída da ação</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="185"/>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation>Captura a saída padrão ou o erro gerado pela ação, e mostra isso em uma caixa de diálogo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="205"/>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="16"/>
        <source>Enter Attribute Values</source>
        <translation>Entre com os valores de atributos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="32"/>
        <source>1</source>
        <translation type="unfinished">1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="37"/>
        <source>Attribute</source>
        <translation>Atributo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="42"/>
        <source>Value</source>
        <translation type="unfinished">Valor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="50"/>
        <source>&amp;OK</source>
        <translation type="unfinished">&amp;OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="57"/>
        <source>&amp;Cancel</source>
        <translation type="unfinished">&amp;Cancelar</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="280"/>
        <source>Run action</source>
        <translation>Rodar ação</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="13"/>
        <source>Attribute Table</source>
        <translation>Tabela de atributos </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="175"/>
        <source>Start editing</source>
        <translation>Iniciar edição</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="279"/>
        <source>&amp;Close</source>
        <translation>&amp;Fechar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="282"/>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="152"/>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="136"/>
        <source>Ctrl+N</source>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="77"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="68"/>
        <source>Invert selection</source>
        <translation>Inverter seleção</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="61"/>
        <source>Ctrl+T</source>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="52"/>
        <source>Move selected to top</source>
        <translation>Mover selecionado para cima</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="36"/>
        <source>Remove selection</source>
        <translation>Remover seleção</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="84"/>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Copiar linhas selecionadas para a área de transferência (Ctrl+C)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="87"/>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Copia a linha selecionada para a área de transferência</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="96"/>
        <source>Ctrl+C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="182"/>
        <source>Stop editin&amp;g</source>
        <translation>Parar edição</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="185"/>
        <source>Alt+G</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="224"/>
        <source>Search for:</source>
        <translation>Pesquisar por:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="237"/>
        <source>in</source>
        <translation>no</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="259"/>
        <source>Search</source>
        <translation type="unfinished">Procurar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="269"/>
        <source>Adva&amp;nced...</source>
        <translation>Ava&amp;nçado...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="272"/>
        <source>Alt+N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="214"/>
        <source>&amp;Help</source>
        <translation type="unfinished">&amp;Ajuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="127"/>
        <source>New column</source>
        <translation>Nova coluna</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="143"/>
        <source>Delete column</source>
        <translation>Excluir coluna</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="103"/>
        <source>Zoom map to the selected rows (Ctrl-F)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="106"/>
        <source>Zoom map to the selected rows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="112"/>
        <source>Ctrl+F</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="94"/>
        <source>select</source>
        <translation>seleciona</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="95"/>
        <source>select and bring to top</source>
        <translation>seleciona e coloca no topo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="96"/>
        <source>show only matching</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="321"/>
        <source>Search string parsing error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="373"/>
        <source>Search results</source>
        <translation>Pesquisar resultados</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="327"/>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="361"/>
        <source>Error during search</source>
        <translation>Erro durante a pesquisa</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="372"/>
        <source>No matching features found.</source>
        <translation>Nehuma característica produrada encontrada.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="144"/>
        <source>Name conflict</source>
        <translation>Conflito de nomes</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>The attribute could not be inserted. The name already exists in the table</source>
        <translation type="obsolete">O atributo não pôde ser inserido. O nome já existe na tabela</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="193"/>
        <source>Stop editing</source>
        <translation type="unfinished">Parar edição</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="194"/>
        <source>Do you want to save the changes?</source>
        <translation type="unfinished">Você quer salvar as alterações?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="200"/>
        <source>Error</source>
        <translation>Erro</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="200"/>
        <source>Could not commit changes</source>
        <translation>Impossível modificar</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="144"/>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="370"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="140"/>
        <source>Really Delete?</source>
        <translation>Quer mesmo excluir?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="141"/>
        <source>Are you sure you want to delete the </source>
        <translation>Certeza em querer excluir o </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="141"/>
        <source> bookmark?</source>
        <translation> favorito?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="157"/>
        <source>Error deleting bookmark</source>
        <translation>Erro ao excluir favorito</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="159"/>
        <source>Failed to delete the </source>
        <translation>Falha ao excluir o </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="161"/>
        <source> bookmark from the database. The database said:
</source>
        <translation> favorito da base de dados. O base de dados disse:</translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="16"/>
        <source>Geospatial Bookmarks</source>
        <translation>Favoritos Geoespaciais</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="45"/>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="50"/>
        <source>Project</source>
        <translation>Projeto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="55"/>
        <source>Extent</source>
        <translation>Extensão</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="60"/>
        <source>Id</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="98"/>
        <source>Zoom To</source>
        <translation>Zoom para</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="95"/>
        <source>Zoom to the currently selected bookmark</source>
        <translation>Zoom para o bookmark atualmente selecionado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="88"/>
        <source>Delete</source>
        <translation>Excluir</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="85"/>
        <source>Delete the currently selected bookmark</source>
        <translation>Excluir o bookmark atualmente selecionado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="78"/>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="75"/>
        <source>Close the dialog</source>
        <translation>Fechar a janela</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="68"/>
        <source>Help</source>
        <translation>Ajuda</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="715"/>
        <source>Choose a filename to save the map image as</source>
        <translation>Escolha o nome do arquivo com a imagem do mapa atual</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="807"/>
        <source>Choose a filename to save the map as</source>
        <translation>Escolha um nome de arquivo para salvar o mapa</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="500"/>
        <source> for read/write</source>
        <translation> para ler/escrever</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="579"/>
        <source>Error in Print</source>
        <translation>Erro em imprimir</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="557"/>
        <source>Cannot seek</source>
        <translation>Impossível procurar</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="482"/>
        <source>Cannot overwrite BoundingBox</source>
        <translation>Impossível sobrescrever CaixaLimite</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="490"/>
        <source>Cannot find BoundingBox</source>
        <translation>Impossível procurar CaixaLimite</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="573"/>
        <source>Cannot overwrite translate</source>
        <translation>Impossível sobrescrever tradução</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="579"/>
        <source>Cannot find translate</source>
        <translation>Impossível achar tradução</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="587"/>
        <source>File IO Error</source>
        <translation>Erro de IO no arquivo</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="597"/>
        <source>Paper does not match</source>
        <translation>Papel sem correspondência</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="598"/>
        <source>The selected paper size does not match the composition size</source>
        <translation>O papel selecionado não tem correspondência com o tamanho da composição</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="661"/>
        <source>Big image</source>
        <translation>Imagem grande</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="662"/>
        <source>To create image </source>
        <translation>Para criar uma imagem</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="665"/>
        <source> requires circa </source>
        <translation> requer cerca de </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="665"/>
        <source> MB of memory</source>
        <translation> MB de memória</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="51"/>
        <source>QGIS - print composer</source>
        <translation>QGIS - compositor de impressão</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="73"/>
        <source>Map 1</source>
        <translation>Mapa 1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="500"/>
        <source>Couldn&apos;t open </source>
        <translation>Impossível abrir</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="692"/>
        <source>format</source>
        <translation>formatar</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="787"/>
        <source>SVG warning</source>
        <translation>Advertência SVG</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="788"/>
        <source>Don&apos;t show this message again</source>
        <translation>Não mostra esta mensagem novamente</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="802"/>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the Qt4 svg code. Of note, text does not appear in the SVG file and there are problems with the map bounding box clipping other items such as the legend or scale bar.&lt;/p&gt;If you require a vector-based output file from Qgis it is suggested that you try printing to PostScript if the SVG output is not satisfactory.&lt;/p&gt;</source>
        <translation>&lt;p&gt;A função de exportação SVG do QGIS tem sério problemas devido a bugs e deficiências do código svg do Qt4.&lt;/p&gt;Se você precisa de um arquivo de saída vetorial sugere-se imprimir para um arquivo PostScript se o SVG exportado não foi satisfatório.&lt;/p&gt;</translation>
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
        <location filename="../src/ui/qgscomposerbase.ui" line="62"/>
        <source>General</source>
        <translation>Geral</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="107"/>
        <source>Composition</source>
        <translation>Composição</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="115"/>
        <source>Item</source>
        <translation>Item</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="208"/>
        <source>&amp;Open Template ...</source>
        <translation>&amp;Abrir Modelo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="216"/>
        <source>Save Template &amp;As...</source>
        <translation>Salvar modelo &amp;como</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="224"/>
        <source>&amp;Print...</source>
        <translation>&amp;Imprimir</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="256"/>
        <source>Add new map</source>
        <translation>Adicionar novo mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="264"/>
        <source>Add new label</source>
        <translation>Adicionar novo rótulo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="272"/>
        <source>Add new vect legend</source>
        <translation>Adicionar nova legenda vetorial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="280"/>
        <source>Select/Move item</source>
        <translation>Selecionar/mover item</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="288"/>
        <source>Export as image</source>
        <translation>Exportar como imagem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="296"/>
        <source>Export as SVG</source>
        <translation>Exportar como SVG</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="304"/>
        <source>Add new scalebar</source>
        <translation>Adicionar nova barra de escala</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="312"/>
        <source>Refresh view</source>
        <translation>Atualizar visão</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation>JanelaPrincipal</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="232"/>
        <source>Zoom All</source>
        <translation>Ver tudo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="240"/>
        <source>Zoom In</source>
        <translation>Aproximar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="248"/>
        <source>Zoom Out</source>
        <translation type="unfinished">Afastar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="320"/>
        <source>Add Image</source>
        <translation>Adicionar imagem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="170"/>
        <source>Close</source>
        <translation type="unfinished">Fechar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="150"/>
        <source>Help</source>
        <translation>Ajuda</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="21"/>
        <source>Label Options</source>
        <translation>Opções de rótulo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="48"/>
        <source>Font</source>
        <translation>Fonte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="55"/>
        <source>Box</source>
        <translation>Caixa</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="96"/>
        <source>Extent (calculate scale)</source>
        <translation>Estender (calcula escala)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="97"/>
        <source>Scale (calculate extent)</source>
        <translation>Escala (calcula extensão)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="74"/>
        <source>Map %1</source>
        <translation>Mapa %1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="104"/>
        <source>Cache</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="105"/>
        <source>Render</source>
        <translation type="unfinished">Desenhar</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="106"/>
        <source>Rectangle</source>
        <translation>Retângulo</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapBase</name>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="21"/>
        <source>Map options</source>
        <translation>Opções de mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="173"/>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation>&lt;b&gt;Mapa&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="147"/>
        <source>Set</source>
        <translation>Marcar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="196"/>
        <source>Width</source>
        <translation>Largura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="180"/>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Scale</source>
        <translation type="obsolete">Escala</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>1 :</source>
        <translation type="obsolete">1 :</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="72"/>
        <source>Set Extent</source>
        <translation>Estender</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="69"/>
        <source>Set map extent to current extent in QGIS map canvas</source>
        <translation>Dimensionar a extensão para tela</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="212"/>
        <source>Line width scale</source>
        <translation>Escala da largura da linha</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="116"/>
        <source>Width of one unit in millimeters</source>
        <translation>Largura da unidade em milímetros</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="225"/>
        <source>Symbol scale</source>
        <translation>Escala do símbolo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="238"/>
        <source>Font size scale</source>
        <translation>Escala da fonte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="251"/>
        <source>Frame</source>
        <translation>Moldura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="258"/>
        <source>Preview</source>
        <translation>Pré-visualização</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="79"/>
        <source>1:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="97"/>
        <source>Scale:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerPicture</name>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="401"/>
        <source>Warning</source>
        <translation>Advertência</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="402"/>
        <source>Cannot load picture.</source>
        <translation>Impossível carregar figura.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="485"/>
        <source>Choose a file</source>
        <translation>Escolha um arquivo</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="468"/>
        <source>Pictures (</source>
        <translation>Figuras (</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureBase</name>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="21"/>
        <source>Picture Options</source>
        <translation>Opções de figura</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Picture</source>
        <translation type="obsolete">Figura</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="197"/>
        <source>Frame</source>
        <translation type="unfinished">Moldura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="161"/>
        <source>Angle</source>
        <translation type="unfinished">Ângulo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="119"/>
        <source>Width</source>
        <translation type="unfinished">Largura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="140"/>
        <source>Height</source>
        <translation type="unfinished">Altura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="58"/>
        <source>Browse</source>
        <translation type="unfinished">Exibir</translation>
    </message>
</context>
<context>
    <name>QgsComposerScalebarBase</name>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="21"/>
        <source>Barscale Options</source>
        <translation>Opções da barra de escala</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="140"/>
        <source>Segment size</source>
        <translation>Tamanho do segmento</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="172"/>
        <source>Number of segments</source>
        <translation>Número de segmentos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="159"/>
        <source>Map units per scalebar unit</source>
        <translation>Unidades do mapa por unidades na barra de escala</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="88"/>
        <source>Unit label</source>
        <translation>Unidade de rótulo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="127"/>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="195"/>
        <source>Font</source>
        <translation>Fonte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="41"/>
        <source>Line width</source>
        <translation>Largura da linha</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegend</name>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="117"/>
        <source>Layers</source>
        <translation>Camadas</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="118"/>
        <source>Group</source>
        <translation>Grupo</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="125"/>
        <source>Combine selected layers</source>
        <translation>Combina camadas selecionadas</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="138"/>
        <source>Cache</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="139"/>
        <source>Render</source>
        <translation type="unfinished">Desenhar</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="140"/>
        <source>Rectangle</source>
        <translation>Retângulo</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="104"/>
        <source>Legend</source>
        <translation>Legenda</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="21"/>
        <source>Vector Legend Options</source>
        <translation>Opções da legenda de vetores</translation>
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
        <translation>Fonte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="148"/>
        <source>Box</source>
        <translation>Caixa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="140"/>
        <source>Column 1</source>
        <translation>Coluna 1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="53"/>
        <source>Preview</source>
        <translation>Pré-visualização</translation>
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
        <translation>A1 (594x841 mm)</translation>
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
        <translation>Carta (8.5x11 polegadas)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="98"/>
        <source>Legal (8.5x14 inches)</source>
        <translation>Legal (8.5x14 polegadas)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="111"/>
        <source>Portrait</source>
        <translation>Retrato</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="112"/>
        <source>Landscape</source>
        <translation>Paisagem</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="607"/>
        <source>Out of memory</source>
        <translation>Sem memória</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="610"/>
        <source>Qgis is unable to resize the paper size due to insufficient memory.
 It is best that you avoid using the map composer until you restart qgis.
</source>
        <translation>O QGIS não pode redimensionar o papel devido a falta de memória. Melhor você evitar usar o compositor de mapas até reiniciar o QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="754"/>
        <source>Label</source>
        <translation type="unfinished">Rótulo</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="804"/>
        <source>Warning</source>
        <translation>Advertência</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="805"/>
        <source>Cannot load picture.</source>
        <translation>Impossível carregar figura.</translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="21"/>
        <source>Composition</source>
        <translation>Composição</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="33"/>
        <source>Paper</source>
        <translation>Papel</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="176"/>
        <source>Size</source>
        <translation>Tamanho</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="158"/>
        <source>Units</source>
        <translation>Unidades</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="140"/>
        <source>Width</source>
        <translation>Largura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="122"/>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="104"/>
        <source>Orientation</source>
        <translation>Orientação</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="213"/>
        <source>Resolution (dpi)</source>
        <translation>Resolução (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialog</name>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="74"/>
        <source>Test connection</source>
        <translation type="unfinished">Testar conexão</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="72"/>
        <source>Connection to </source>
        <translation>Conexão para </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="72"/>
        <source> was successfull</source>
        <translation> foi completada</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="74"/>
        <source>Connection failed - Check settings and try again </source>
        <translation>Conexão falhou - Cheque configurações e tente novamente </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="96"/>
        <source>General Interface Help:

</source>
        <translation>Ajuda Geral da Interface:</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialogBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="31"/>
        <source>Connection Information</source>
        <translation>Informação da Conexão</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="90"/>
        <source>Host</source>
        <translation>Servidor</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="97"/>
        <source>Database</source>
        <translation>Banco de Dados</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="111"/>
        <source>Username</source>
        <translation>Usuário</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="83"/>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="135"/>
        <source>Name of the new connection</source>
        <translation>Nome da nova conexão</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="118"/>
        <source>Password</source>
        <translation>Senha</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="58"/>
        <source>Test Connect</source>
        <translation>Testar Conexão</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="51"/>
        <source>Save Password</source>
        <translation>Salvar Senha</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="13"/>
        <source>Create a New PostGIS connection</source>
        <translation>Criar nova conexão PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="104"/>
        <source>Port</source>
        <translation>Porta</translation>
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
        <translation type="unfinished">Cor Contínua</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="99"/>
        <source>Maximum Value:</source>
        <translation type="unfinished">Valor Máximo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="73"/>
        <source>Outline Width:</source>
        <translation type="unfinished">Espessura da borda:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="57"/>
        <source>Minimum Value:</source>
        <translation type="unfinished">Valor Mínimo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="41"/>
        <source>Classification Field:</source>
        <translation>Campo de classificação:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="28"/>
        <source>Draw polygon outline</source>
        <translation>Desenha o contorno do polígono</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>Failed</source>
        <translation>Falhou</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>transform of</source>
        <translation>transformar de</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="496"/>
        <source>with error: </source>
        <translation>com erro:</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="418"/>
        <source>The source spatial reference system (SRS) is not valid. </source>
        <translation>O sistema de referência espacial (SRE) não é válido. </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="426"/>
        <source>The coordinates can not be reprojected. The SRS is: </source>
        <translation>As coordenadas não podem ser reprojetadas. O SRE é: </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="425"/>
        <source>The destination spatial reference system (SRS) is not valid. </source>
        <translation>O sistema de referência espacial (SRE) não é válido. </translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="66"/>
        <source>Bottom Left</source>
        <translation>Inferior Esquerdo</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Left</source>
        <translation>Superior Esquerdo</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Right</source>
        <translation>Superior Direito</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Bottom Right</source>
        <translation type="unfinished">Inferior Direito</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="79"/>
        <source>&amp;Copyright Label</source>
        <translation>&amp;Etiqueta de Copyright</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="80"/>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>Cria uma etiqueta de copyright que será mostrada na tela do mapa.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="204"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Decorações</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="13"/>
        <source>Copyright Label Plugin</source>
        <translation>Plugin do rótulo de Copyright</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="148"/>
        <source>Placement</source>
        <translation>Posicionamento</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="156"/>
        <source>Bottom Left</source>
        <translation>Inferior Esquerdo</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="161"/>
        <source>Top Left</source>
        <translation>Superior Esquerdo</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="166"/>
        <source>Bottom Right</source>
        <translation>Inferior Direito</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="171"/>
        <source>Top Right</source>
        <translation>Superior Direito</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="179"/>
        <source>Orientation</source>
        <translation>Orientação</translation>
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
        <translation>Habilitar Rótulo de Copyright</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Enter your copyright label below. This plugin supports basic html markup tags for formatting the label. For example:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Bold text &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Italics &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(note: &amp;amp;copy; gives a copyright symbol)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Descrição&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Enter your copyright label below. This plugin supports basic html markup tags for formatting the label. Por examplo:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Bold text &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Italics &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(note: &amp;amp;copy; gives a copyright symbol)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-size:14pt;&quot;&gt;&#xa9; QGIS 2006&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-size:14pt;&quot;&gt;© QGIS 2008&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="36"/>
        <source>Color</source>
        <translation type="unfinished">Cor</translation>
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
        <translation type="unfinished"></translation>
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
        <translation>Excluir definição de projeção?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="166"/>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Excluir a definição de projeção é irreversível. Quer mesmo excluir?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="876"/>
        <source>Abort</source>
        <translation>Abortar</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="878"/>
        <source>New</source>
        <translation>Novo</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="933"/>
        <source>QGIS Custom Projection</source>
        <translation>Projeção personalizada do QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="794"/>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Esta definição de projeção não é válida. Corrija antes de pressionar salvar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="750"/>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Esta definição de projeção proj4 não é válida. Por Favor forneça um nome de projeção antes de pressionar salvar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="756"/>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Esta definição de projeção proj4 não é válida. Por favor adicione os parâmetros antes de pressionar salvar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="771"/>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Esta definição de projeção proj4 não é válida. Por favor adicione uma cláusula proj= antes de pressionar salvar</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="778"/>
        <source>This proj4 ellipsoid definition is not valid. Please add a ellips= clause before pressing save.</source>
        <translation>Esta definição de elipsóide proj4 não é válida. Por favor adicione uma cláusula ellips= antes de pressionar salvar.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="907"/>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Esta definição de projeção proj4 não é válida.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="922"/>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>N e E devem estar em formato decimal.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="934"/>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Erro interno (projeção fonte inválida?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="158"/>
        <source>Name:</source>
        <translation>Nome:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="65"/>
        <source>Custom Projection Definition</source>
        <translation>Definição de projeção personalizada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="198"/>
        <source>Parameters:</source>
        <translation>Parâmetros:</translation>
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
        <translation>&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="128"/>
        <source>New</source>
        <translation>Novo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="135"/>
        <source>Save</source>
        <translation>Salvar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="142"/>
        <source>Delete</source>
        <translation>Excluir</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="149"/>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="32"/>
        <source>Define</source>
        <translation>Definir</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="169"/>
        <source>Test</source>
        <translation type="unfinished">Testar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="181"/>
        <source>Transform from WGS84 to the chosen projection</source>
        <translation>Transforma de WGS84 para a projeção escolhida</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="227"/>
        <source>Projected Corrdinate System</source>
        <translation>Sistema de coordenadas projetado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="234"/>
        <source>Geographic / WGS84</source>
        <translation>Geográfica / WGS84</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="260"/>
        <source>East:</source>
        <translation>Leste:</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Você pode definir sua projeção personalizada aqui. A projeção deve estar em conformidade com o formato proj4 para especificar um Sistema de Referência Espacial.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt; Use estes campos de texto abaixo para testar a definição de projeção que você criou. Entre com as coordenadas onde lat/long e o resultado reprojetado são conhecidos (p.e. by reading off a map). Então, pressione o botão calcular para ver se a projeção que você criou é precisa.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="250"/>
        <source>Are you sure you want to remove the </source>
        <translation>Tem certeza que deseja remover o </translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="250"/>
        <source> connection and all associated settings?</source>
        <translation>conexão e todos os ajustes associados ?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="251"/>
        <source>Confirm Delete</source>
        <translation>Confirme a exclusão</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="317"/>
        <source>Select Table</source>
        <translation>Selecionar Tabela</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="317"/>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Você deve selecionar uma tabela para poder adicionar uma Camada.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="366"/>
        <source>Password for </source>
        <translation>Senha para</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="367"/>
        <source>Please enter your password:</source>
        <translation>Por favor, entre com sua senha:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="495"/>
        <source>Connection failed</source>
        <translation>A conexão Falhou</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Access to relation </source>
        <translation type="obsolete">Acesso a relação</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source> using sql;
</source>
        <translation type="obsolete">Usando sql</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>
has failed. The database said:
</source>
        <translation type="obsolete">falhou. A base de dados reportou:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="112"/>
        <source>Type</source>
        <translation type="unfinished">Tipo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="113"/>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="114"/>
        <source>Sql</source>
        <translation type="unfinished">Sql</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="404"/>
        <source>Point layer</source>
        <translation>Camada ponto</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="407"/>
        <source>Multi-point layer</source>
        <translation>Camada multi-ponto</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="411"/>
        <source>Linestring layer</source>
        <translation>Camada poli-linha</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="414"/>
        <source>Multi-linestring layer</source>
        <translation>Camada multi-poli-linha</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="418"/>
        <source>Polygon layer</source>
        <translation>Camada polígono</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="421"/>
        <source>Multi-polygon layer</source>
        <translation>Camada multi-polígono</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="425"/>
        <source>Mixed geometry layer</source>
        <translation>Camada de geometria mista</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="428"/>
        <source>Geometry collection layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="432"/>
        <source>Waiting for layer type</source>
        <translation>Esperando por uma camada tipo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="435"/>
        <source>Unknown layer type</source>
        <translation>Tipo de camada desconhecido</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="498"/>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>Conexão para %1 em %2 falhou. A base de dados pode estar fora do ar ou suas configurações estão incorretas.%3Cheque seu nome de usuário e senha e tente novamente.%4A base de dados disse:%5%6</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="329"/>
        <source>double click to open PostgreSQL query builder</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelectBase</name>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="13"/>
        <source>Add PostGIS Table(s)</source>
        <translation>Adicionar tabela(s) PostGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="74"/>
        <source>Add</source>
        <translation>Adicionar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="90"/>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="45"/>
        <source>Help</source>
        <translation>Ajuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="48"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="200"/>
        <source>Connect</source>
        <translation>Conectar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="193"/>
        <source>New</source>
        <translation>Novo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="186"/>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="179"/>
        <source>Delete</source>
        <translation>Deletar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="167"/>
        <source>PostgreSQL Connections</source>
        <translation>Conexões PostgreSQL </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="115"/>
        <source>Tables:</source>
        <translation>Tabelas:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="146"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="151"/>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="156"/>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="108"/>
        <source>Encoding:</source>
        <translation>Codificando:</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="16"/>
        <source>Delete Attributes</source>
        <translation>Excluir atributos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="52"/>
        <source>OK</source>
        <translation>OK</translation>
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
        <translation>&amp;Adicionar uma camada a partir de um texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="104"/>
        <source>Add a delimited text file as a map layer. </source>
        <translation>Adiciona um texto delimitado como camada no mapa. </translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>The file must have a header row containing the field names. </source>
        <translation>O arquivo deve ter cabeçalho de linha contendo os nomes dos campos. </translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>Campos X e Y são requeridos e devem conter coordenadas em unidades decimais.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="142"/>
        <source>&amp;Delimited text</source>
        <translation>&amp;Texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="56"/>
        <source>DelimitedTextLayer</source>
        <translation>CamadaTextoDelimitado</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="118"/>
        <source>No layer name</source>
        <translation>Sem nome de camada</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="118"/>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Por favor, entre com o nome da camada antes de adicioná-la ao mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="204"/>
        <source>No delimiter</source>
        <translation>Sem delimitador</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="204"/>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Por favor, especifique um delimitador antes de analisar o arquivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="238"/>
        <source>Choose a delimited text file to open</source>
        <translation>Escolha um arquivos de texto delimitado para abrir</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="35"/>
        <source>Parse</source>
        <translation>Analisar</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>&lt;h1&gt;Description&lt;/h1&gt;&lt;p&gt;Select a delimited text file containing x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!&lt;/p&gt;&lt;p&gt;Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma or tab). After choosing a delimiter, press the parse button an select the columns containing the x and y values for the layer.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;h1&gt;Descrição&lt;/h1&gt;&lt;p&gt; Selecione um arquivo de texto delimitado com coordenadas X e Y que você poderá usar como uma camada de ṕontos e este plugin fará o resto para você!&lt;/p&gt;&lt;p&gt; Use a caixa &apos;nome da camada&apos; para especificar o nome que aparecerá na legenda da nova camada. Use caixa de delimitador para definir qual o delimitados do seu arquivo de texto (e.g. espaço, vírgula ou tabulação). Após escolher o delimitador, pressione o botão Analisar e selecione as colunas que contém as coordenadas X e Y.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="61"/>
        <source>&lt;h2&gt;Description&lt;/h2&gt;&lt;p&gt;Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!&lt;/p&gt;&lt;p&gt;Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="13"/>
        <source>Create a Layer from a Delimited Text File</source>
        <translation>Criar uma camada a partir de arquivo de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="70"/>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Campo X&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="91"/>
        <source>Name of the field containing x values</source>
        <translation>Nome do campo contendo valores de X</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="94"/>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nome do campo contendo valores de X. Escolha um campo da lista. A lista é gerada a partir da linha de cabeçalho do arquivo.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="104"/>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Campo Y&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="125"/>
        <source>Name of the field containing y values</source>
        <translation>Nome do campo contendo valores de Y</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="128"/>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nome do campo contendo valores de Y. Escolha um campo da lista. A lista é gerada a partir da linha de cabeçalho do arquivo.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="194"/>
        <source>Layer name</source>
        <translation>Nome da camada</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="201"/>
        <source>Name to display in the map legend</source>
        <translation>Nome para exibir na legenda do mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="204"/>
        <source>Name displayed in the map legend</source>
        <translation>Nome exibido na legenda do mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="300"/>
        <source>Delimiter</source>
        <translation>Delimitador</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="321"/>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation>Delimitador usado ao separar os campos do arquivo texto. O delimitador pode possuir mais de um caractere.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="324"/>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Delimitador usado ao separar os campos do arquivo texto. O delimitador pode possuir um ou mais caracteres.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="39"/>
        <source>Delimited Text Layer</source>
        <translation>Camada de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="148"/>
        <source>Delimited text file</source>
        <translation>Arquivo texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="155"/>
        <source>Full path to the delimited text file</source>
        <translation>Caminho completo para o arquivo de texto delimitado</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="158"/>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Caminho completo para o arquivo texto delimitado. Para analisar apropriadamente os campos do arquivo, o delimitador deve ser escolhido antes do arquivo. Use o botão procurar ao lado deste campo para escolher um arquivo de entrada.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="171"/>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Procurar arquivo de texto delimitado para processamento</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="174"/>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Utilize este botão para procurar o arquivo texto delimitado. O botão não será habilitado enquanto um delimitador não houver sido escolhido no campo &lt;i&gt;Delimitador&lt;/i&gt;. Depois de escolhido um arquivo, as caixas de seleção X e Y serão preenchidas com os campos do arquivo texto.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="249"/>
        <source>Sample text</source>
        <translation>Texto de exemplo</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="177"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="337"/>
        <source>Plain characters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="347"/>
        <source>The delimiter is a regular expression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="350"/>
        <source>Regular expression</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextProvider</name>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="402"/>
        <source>Note: the following lines were not loaded because Qgis was unable to determine values for the x and y coordinates:
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="400"/>
        <source>Error</source>
        <translation type="unfinished">Erro</translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="13"/>
        <source>Buffer features</source>
        <translation>Feições do Buffer</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="224"/>
        <source>Buffer distance in map units:</source>
        <translation>Distância dos Buffers em unidades do mapa:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="132"/>
        <source>Table name for the buffered layer:</source>
        <translation>Nome da tabela para a camada Buferizada:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="172"/>
        <source>Create unique object id</source>
        <translation>Criar ID de objeto único</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="216"/>
        <source>public</source>
        <translation>público</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="59"/>
        <source>Geometry column:</source>
        <translation>Geometria da Coluna:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="92"/>
        <source>Spatial reference ID:</source>
        <translation>ID de referência Espacial:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="125"/>
        <source>Unique field to use as feature id:</source>
        <translation>Campo absoluto para usar como ID de feição:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="118"/>
        <source>Schema:</source>
        <translation>Esquema:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="66"/>
        <source>Add the buffered layer to the map?</source>
        <translation>Adicionar a camada buferizada para o mapa?</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="234"/>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Buferizar as feições na camada: &lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="28"/>
        <source>Parameters</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsBase</name>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="13"/>
        <source>Edit Reserved Words</source>
        <translation>Editar palavras abreviadas</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="47"/>
        <source>Status</source>
        <translation>Situação</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="57"/>
        <source>Index</source>
        <translation>Índice</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="89"/>
        <source>Reserved Words</source>
        <translation>Palavras abreviadas</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="37"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Double click the Column Name column to change the name of the column.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="52"/>
        <source>Column Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="82"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This shapefile contains reserved words. These may affect the import into PostgreSQL. Edit the column names so none of the reserved words listed at the right are used (click on a Column Name entry to edit). You may also change any other column name if desired.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsDialog</name>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsdialog.cpp" line="34"/>
        <source>Status</source>
        <translation type="unfinished">Situação</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsdialog.cpp" line="34"/>
        <source>Column Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsdialog.cpp" line="34"/>
        <source>Index</source>
        <translation type="unfinished">Índice</translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <location filename="../src/gui/qgsencodingfiledialog.cpp" line="29"/>
        <source>Encoding:</source>
        <translation>Codificando:</translation>
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
        <translation>Estilo de preenchimento</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="72"/>
        <source>PolyStyleWidget</source>
        <translation>Poli-Estilo de Widget</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="61"/>
        <source>Colour:</source>
        <translation>Cor:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="54"/>
        <source>col</source>
        <translation>col</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="43"/>
        <source>New device %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="56"/>
        <source>Are you sure?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="57"/>
        <source>Are you sure that you want to delete this device?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="24"/>
        <source>GPS Device Editor</source>
        <translation>Editor de dispositivo GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="135"/>
        <source>Device name:</source>
        <translation>Nome do dispositivo:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="150"/>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Este é o nome como o dispositivo irá aparecer na lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="102"/>
        <source>Update device</source>
        <translation>Atualizar dispositivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="87"/>
        <source>Delete device</source>
        <translation>Excluir dispositivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="72"/>
        <source>New device</source>
        <translation>Novo dispositivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="301"/>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="159"/>
        <source>Commands</source>
        <translation>Commandos</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="188"/>
        <source>Waypoint download:</source>
        <translation>Descarregar waypoints:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="251"/>
        <source>Waypoint upload:</source>
        <translation>Carregar waypoints:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="202"/>
        <source>Route download:</source>
        <translation>Descarregar rotas:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="181"/>
        <source>Route upload:</source>
        <translation>Carregar rotas:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="174"/>
        <source>Track download:</source>
        <translation>Descarregar trilhas:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="244"/>
        <source>The command that is used to upload tracks to the device</source>
        <translation>O comando que é usado para carregar trilhas para o dispositivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="216"/>
        <source>Track upload:</source>
        <translation>Carregar trilhas:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="223"/>
        <source>The command that is used to download tracks from the device</source>
        <translation>O comando que é usado para descarregar trilhas do dispositivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="230"/>
        <source>The command that is used to upload routes to the device</source>
        <translation>O comando que é usado para carregar rotas para o dispositivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="195"/>
        <source>The command that is used to download routes from the device</source>
        <translation>O comando que é usado para descarregar rotas do dispositivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="209"/>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>O comando que é usado para carregar waypoints para o dispositivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="237"/>
        <source>The command that is used to download waypoints from the device</source>
        <translation>O comando que é usado para descarregar waypoints do dispositivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="269"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="92"/>
        <source>&amp;Gps Tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="93"/>
        <source>&amp;Create new GPX layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="96"/>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="195"/>
        <source>&amp;Gps</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="159"/>
        <source>Save new GPX file as...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="159"/>
        <source>GPS eXchange file (*.gpx)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="164"/>
        <source>Could not create file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="166"/>
        <source>Unable to create a GPX file with the given name. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="167"/>
        <source>Try again with another name or in another </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="167"/>
        <source>directory.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="206"/>
        <source>GPX Loader</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="208"/>
        <source>Unable to read the selected file.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="208"/>
        <source>Please reselect a valid file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="491"/>
        <source>Could not start process</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="492"/>
        <source>Could not start GPSBabel!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="330"/>
        <source>Importing data...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="497"/>
        <source>Cancel</source>
        <translation type="unfinished">Cancelar</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="275"/>
        <source>Could not import data from %1!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="277"/>
        <source>Error importing data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="481"/>
        <source>Not supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="397"/>
        <source>This device does not support downloading </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="397"/>
        <source>of </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="412"/>
        <source>Downloading data...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="426"/>
        <source>Could not download data from GPS!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="428"/>
        <source>Error downloading data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="482"/>
        <source>This device does not support uploading of </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="497"/>
        <source>Uploading data...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="511"/>
        <source>Error while uploading data to GPS!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="513"/>
        <source>Error uploading data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="345"/>
        <source>Could not convert data from %1!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="347"/>
        <source>Error converting data</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="445"/>
        <source>Choose a filename to save under</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="447"/>
        <source>GPS eXchange format (*.gpx)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="434"/>
        <source>Select GPX file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="240"/>
        <source>Select file and format to import</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="258"/>
        <source>Waypoints</source>
        <translation type="unfinished">Waypoints</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="260"/>
        <source>Routes</source>
        <translation type="unfinished">Rotas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="262"/>
        <source>Tracks</source>
        <translation type="unfinished">Trilhas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="408"/>
        <source>Route -&gt; Waypoint</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="409"/>
        <source>Waypoint -&gt; Route</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="414"/>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="415"/>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="416"/>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX filename that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="13"/>
        <source>GPS Tools</source>
        <translation>Ferramentas de GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="58"/>
        <source>Load GPX file</source>
        <translation>Carregar arquivo GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="95"/>
        <source>File:</source>
        <translation>Arquivo:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="112"/>
        <source>Feature types:</source>
        <translation>Tipos de Feições:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="317"/>
        <source>Waypoints</source>
        <translation>Waypoints</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="322"/>
        <source>Routes</source>
        <translation>Rotas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="327"/>
        <source>Tracks</source>
        <translation>Trilhas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="159"/>
        <source>Import other file</source>
        <translation>Importar outro arquivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="266"/>
        <source>File to import:</source>
        <translation>Arquivo a ser importado:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="342"/>
        <source>Feature type:</source>
        <translation>Tipo de feição:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="584"/>
        <source>GPX output file:</source>
        <translation>Arquivo GPX de saída:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="560"/>
        <source>Layer name:</source>
        <translation>Nome da camada:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="274"/>
        <source>Download from GPS</source>
        <translation>Descarregar do GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="512"/>
        <source>Edit devices</source>
        <translation>Editar dispositivos</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="519"/>
        <source>GPS device:</source>
        <translation>Dispositivo GPS:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="359"/>
        <source>Output file:</source>
        <translation>Arquivo de saída:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="485"/>
        <source>Port:</source>
        <translation>Porta:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="433"/>
        <source>Upload to GPS</source>
        <translation>Carregar no GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="526"/>
        <source>Data layer:</source>
        <translation>Camada de dados:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="84"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;GPX is the &lt;/span&gt;&lt;a href=&quot;http://www.topografix.com/gpx.asp&quot;&gt;&lt;span style=&quot; font-size:10pt; text-decoration: underline; color:#0000ff;&quot;&gt;GPS eXchange file format&lt;/span&gt;&lt;/a&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;, which is used to store information about waypoints, routes, and tracks.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Select a GPX file and then select the feature types that you want to load.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="574"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="174"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (&lt;/span&gt;&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; font-size:10pt; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;). This requires that you have GPSBabel installed where QGIS can find it.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer. All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="567"/>
        <source>Save As...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="289"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This tool will help you download data from a GPS device. Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data. If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;This tool uses the program GPSBabel (&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;) to transfer the data. If you don&apos;t have GPSBabel installed where QGIS can find it, this tool will not work.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="464"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This tool will help you upload data from a GPX layer to a GPS device. Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to. If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;This tool uses the program GPSBabel (&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;) to transfer the data. If you don&apos;t have GPSBabel installed where QGIS can find it, this tool will not work.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="252"/>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="535"/>
        <source>GPX Conversions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="617"/>
        <source>Conversion:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="631"/>
        <source>GPX input file:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="68"/>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="112"/>
        <source>GPS eXchange file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="729"/>
        <source>Digitized in QGIS</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="31"/>
        <source>Name</source>
        <translation>Nome</translation>
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
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="81"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="93"/>
        <source>Point</source>
        <translation>Ponto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="100"/>
        <source>Line</source>
        <translation>Linha</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="107"/>
        <source>Polygon</source>
        <translation>Polígono</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="13"/>
        <source>New Vector Layer</source>
        <translation>Nova camada vetorial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="64"/>
        <source>Attributes:</source>
        <translation>Atributos:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="74"/>
        <source>Add</source>
        <translation>Adicionar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="118"/>
        <source>Column 1</source>
        <translation>Coluna 1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="41"/>
        <source>Remove</source>
        <translation>Remover</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="28"/>
        <source>File Format:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeorefDescriptionDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefdescriptiondialogbase.ui" line="13"/>
        <source>Description georeferencer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefdescriptiondialogbase.ui" line="44"/>
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
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="119"/>
        <source>&amp;Georeferencer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="85"/>
        <source>Choose a raster file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="87"/>
        <source>Raster files (*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="97"/>
        <source>Error</source>
        <translation type="unfinished">Erro</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="98"/>
        <source>The selected file is not a valid raster file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="122"/>
        <source>World file exists</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="124"/>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>world file! Do you want to replace it with the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>new world file?&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="13"/>
        <source>Georeferencer</source>
        <translation>Georreferenciar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="100"/>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="43"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="62"/>
        <source>Raster file:</source>
        <translation>Arquivo raster:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="28"/>
        <source>Arrange plugin windows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="77"/>
        <source>Description...</source>
        <translation type="unfinished"></translation>
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
        <translation>Opções Warp</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="35"/>
        <source>Resampling method:</source>
        <translation>Método de reamostragem:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="46"/>
        <source>Nearest neighbour</source>
        <translation>Vizinho mais próximo</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="51"/>
        <source>Linear</source>
        <translation>Linear</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="56"/>
        <source>Cubic</source>
        <translation>Cúbico</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="74"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="64"/>
        <source>Use 0 for transparency when needed</source>
        <translation>Use 0 para transparência quando necessário</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="28"/>
        <source>Compression:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="322"/>
        <source>Equal Interval</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="299"/>
        <source>Quantiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="346"/>
        <source>Empty</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="25"/>
        <source>graduated Symbol</source>
        <translation type="unfinished">Símbolo graduado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="73"/>
        <source>Classification Field:</source>
        <translation type="unfinished">Campo de classificação:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="89"/>
        <source>Mode:</source>
        <translation type="unfinished">Modo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="105"/>
        <source>Number of Classes:</source>
        <translation type="unfinished">Número de Classes:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="159"/>
        <source>Delete class</source>
        <translation type="unfinished">Excluir classe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="166"/>
        <source>Classify</source>
        <translation type="unfinished">Classifica</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="300"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="152"/>
        <source>Column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="153"/>
        <source>Value</source>
        <translation type="unfinished">Valor</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="154"/>
        <source>Type</source>
        <translation type="unfinished">Tipo</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Field</source>
        <translation type="obsolete">Campo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="301"/>
        <source>ERROR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="303"/>
        <source>OK</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="158"/>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="48"/>
        <source>GRASS Attributes</source>
        <translation>Atributos do GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="78"/>
        <source>Tab 1</source>
        <translation>Tabela 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="112"/>
        <source>result</source>
        <translation>resultado</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="180"/>
        <source>Update</source>
        <translation>Atualizar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="177"/>
        <source>Update database record</source>
        <translation>Atualiza a base de dados</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="210"/>
        <source>New</source>
        <translation>Novo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="207"/>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Adicionar categoria usando configurações da Caixa de ferramentas de edição do GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="240"/>
        <source>Delete</source>
        <translation>Excluir</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="237"/>
        <source>Delete selected category</source>
        <translation>Excluir a categoria selecionada</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="66"/>
        <source>Tools</source>
        <translation type="unfinished">Ferramentas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="71"/>
        <source>Add selected map to canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="79"/>
        <source>Copy selected map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="87"/>
        <source>Rename selected map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="95"/>
        <source>Delete selected map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="103"/>
        <source>Set current region to selected map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="111"/>
        <source>Refresh</source>
        <translation type="unfinished">Atualizar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="454"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="290"/>
        <source>Cannot copy map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="412"/>
        <source>&lt;br&gt;command: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="356"/>
        <source>Cannot rename map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="394"/>
        <source>Delete map &lt;b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="411"/>
        <source>Cannot delete map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="455"/>
        <source>Cannot write new region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="340"/>
        <source>New name</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="249"/>
        <source>New point</source>
        <translation>Novo ponto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="267"/>
        <source>New centroid</source>
        <translation>Novo centróide</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="285"/>
        <source>Delete vertex</source>
        <translation>Excluir vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1812"/>
        <source>Left: </source>
        <translation>Esquerdo: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1813"/>
        <source>Middle: </source>
        <translation>Médio: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="246"/>
        <source>Edit tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="255"/>
        <source>New line</source>
        <translation type="unfinished">Nova linha</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="261"/>
        <source>New boundary</source>
        <translation type="unfinished">Nova borda</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="273"/>
        <source>Move vertex</source>
        <translation type="unfinished">Mover vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="279"/>
        <source>Add vertex</source>
        <translation type="unfinished">Adicionar vértice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="291"/>
        <source>Move element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="297"/>
        <source>Split line</source>
        <translation type="unfinished">Dividir linha</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="303"/>
        <source>Delete element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="309"/>
        <source>Edit attributes</source>
        <translation type="unfinished">Editar atributos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="314"/>
        <source>Close</source>
        <translation type="unfinished">Fechar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1449"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="233"/>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="238"/>
        <source>Cannot open vector for update.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="697"/>
        <source>Info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="697"/>
        <source>The table was created</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1322"/>
        <source>Tool not yet implemented.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1348"/>
        <source>Cannot check orphan record: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1355"/>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1364"/>
        <source>Cannot delete orphan record: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1392"/>
        <source>Cannot describe table for field </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="395"/>
        <source>Background</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="396"/>
        <source>Highlight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="397"/>
        <source>Dynamic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="398"/>
        <source>Point</source>
        <translation type="unfinished">Ponto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="399"/>
        <source>Line</source>
        <translation type="unfinished">Linha</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="400"/>
        <source>Boundary (no area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="401"/>
        <source>Boundary (1 area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="402"/>
        <source>Boundary (2 areas)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="403"/>
        <source>Centroid (in area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="404"/>
        <source>Centroid (outside area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="405"/>
        <source>Centroid (duplicate in area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="406"/>
        <source>Node (1 line)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="407"/>
        <source>Node (2 lines)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="440"/>
        <source>Disp</source>
        <comment>Column title</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Color</source>
        <comment>







Column title</comment>
        <translation type="obsolete">Cor</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Type</source>
        <comment>







Column title</comment>
        <translation type="obsolete">Tipo</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Index</source>
        <comment>







Column title</comment>
        <translation type="obsolete">Índice</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="476"/>
        <source>Column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="477"/>
        <source>Type</source>
        <translation type="unfinished">Tipo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="478"/>
        <source>Length</source>
        <translation type="unfinished">Tamanho</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="533"/>
        <source>Next not used</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="534"/>
        <source>Manual entry</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="535"/>
        <source>No category</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1814"/>
        <source>Right: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="442"/>
        <source>Color</source>
        <comment>Column title</comment>
        <translation type="unfinished">Cor</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="444"/>
        <source>Type</source>
        <comment>Column title</comment>
        <translation type="unfinished">Tipo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="446"/>
        <source>Index</source>
        <comment>Column title</comment>
        <translation type="unfinished">Índice</translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="16"/>
        <source>GRASS Edit</source>
        <translation>Editor GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="106"/>
        <source>Category</source>
        <translation>Categoria</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="66"/>
        <source>Mode</source>
        <translation>Modo</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Field (layer)</source>
        <translation type="obsolete">Campo (camada)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="170"/>
        <source>Settings</source>
        <translation>Opções</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="190"/>
        <source>Snapping in screen pixels</source>
        <translation>Snapping em pixels da tela</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="241"/>
        <source>Symbology</source>
        <translation>Simbologia</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="274"/>
        <source>Column 1</source>
        <translation>Coluna 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="360"/>
        <source>Table</source>
        <translation>Tabela</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="496"/>
        <source>Add Column</source>
        <translation>Adicionar coluna</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="511"/>
        <source>Create / Alter Table</source>
        <translation>Criar/Alterar Tabela</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="298"/>
        <source>Line width</source>
        <translation type="unfinished">Largura da linha</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="325"/>
        <source>Marker size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="410"/>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="131"/>
        <source>Cancel</source>
        <translation type="unfinished">Cancelar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="162"/>
        <source>Ok</source>
        <translation type="unfinished">Ok</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="167"/>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="178"/>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="184"/>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="185"/>
        <source>Overwrite</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="110"/>
        <source>Mapcalc tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="113"/>
        <source>Add map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="120"/>
        <source>Add constant value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="127"/>
        <source>Add operator or function</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="134"/>
        <source>Add connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="141"/>
        <source>Select item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="148"/>
        <source>Delete selected item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="158"/>
        <source>Open</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="163"/>
        <source>Save</source>
        <translation type="unfinished">Salvar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="169"/>
        <source>Save as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="177"/>
        <source>Addition</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="178"/>
        <source>Subtraction</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="179"/>
        <source>Multiplication</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="180"/>
        <source>Division</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="181"/>
        <source>Modulus</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="182"/>
        <source>Exponentiation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="185"/>
        <source>Equal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="186"/>
        <source>Not equal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="187"/>
        <source>Greater than</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="188"/>
        <source>Greater than or equal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="189"/>
        <source>Less than</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="190"/>
        <source>Less than or equal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="191"/>
        <source>And</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="192"/>
        <source>Or</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="195"/>
        <source>Absolute value of x</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="196"/>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="197"/>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="198"/>
        <source>Current column of moving window (starts with 1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="199"/>
        <source>Cosine of x (x is in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="200"/>
        <source>Convert x to double-precision floating point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="201"/>
        <source>Current east-west resolution</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="202"/>
        <source>Exponential function of x</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="203"/>
        <source>x to the power y</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="204"/>
        <source>Convert x to single-precision floating point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="205"/>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="206"/>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="207"/>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="208"/>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="209"/>
        <source>Convert x to integer [ truncates ]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="210"/>
        <source>Check if x = NULL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="211"/>
        <source>Natural log of x</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="212"/>
        <source>Log of x base b</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="214"/>
        <source>Largest value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="216"/>
        <source>Median value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="218"/>
        <source>Smallest value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="220"/>
        <source>Mode value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="221"/>
        <source>1 if x is zero, 0 otherwise</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="222"/>
        <source>Current north-south resolution</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="223"/>
        <source>NULL value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="224"/>
        <source>Random value between a and b</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="225"/>
        <source>Round x to nearest integer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="226"/>
        <source>Current row of moving window (Starts with 1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="227"/>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="228"/>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="229"/>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="230"/>
        <source>Current x-coordinate of moving window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="231"/>
        <source>Current y-coordinate of moving window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1317"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="583"/>
        <source>Cannot get current region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="560"/>
        <source>Cannot check region of map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="616"/>
        <source>Cannot get region of map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="812"/>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1102"/>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1112"/>
        <source>New mapcalc</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1113"/>
        <source>Enter new mapcalc name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1118"/>
        <source>Enter vector name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1126"/>
        <source>The file already exists. Overwrite? </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1164"/>
        <source>Save mapcalc</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1146"/>
        <source>File name empty</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1165"/>
        <source>Cannot open mapcalc file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>The mapcalc schema (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>) not found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1302"/>
        <source>Cannot open mapcalc schema (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1313"/>
        <source>Cannot read mapcalc schema (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1314"/>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1315"/>
        <source> column </source>
        <translation type="unfinished">coluna</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1388"/>
        <source>Output</source>
        <translation type="unfinished">Saída</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="16"/>
        <source>MainWindow</source>
        <translation type="unfinished">JanelaPrincipal</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="37"/>
        <source>Output</source>
        <translation type="unfinished">Saída</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1367"/>
        <source>Run</source>
        <translation>Rodar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1345"/>
        <source>Stop</source>
        <translation>Parar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="192"/>
        <source>Module</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1339"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="207"/>
        <source>The module file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="207"/>
        <source>) not found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="211"/>
        <source>Cannot open module file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="979"/>
        <source>)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="974"/>
        <source>Cannot read module file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="974"/>
        <source>):
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="975"/>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="247"/>
        <source>Module </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="247"/>
        <source> not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="288"/>
        <source>Cannot find man page </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="968"/>
        <source>Not available, cannot open description (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="975"/>
        <source> column </source>
        <translation type="unfinished">coluna</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="979"/>
        <source>Not available, incorrect description (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1166"/>
        <source>Cannot get input region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1154"/>
        <source>Use Input Region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1268"/>
        <source>Cannot find module </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1340"/>
        <source>Cannot start module: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1356"/>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1362"/>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1365"/>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="965"/>
        <source>Not available, description not found (</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="13"/>
        <source>GRASS Module</source>
        <translation>Módulo do GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="26"/>
        <source>Options</source>
        <translation>Opções</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="31"/>
        <source>Output</source>
        <translation>Saída</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="47"/>
        <source>Manual</source>
        <translation>Manual</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="118"/>
        <source>Run</source>
        <translation>Rodar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="161"/>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="141"/>
        <source>View output</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="74"/>
        <source>TextLabel</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2702"/>
        <source>Attribute field</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2904"/>
        <source>File</source>
        <translation type="unfinished">Arquivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3017"/>
        <source>:&amp;nbsp;missing value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3024"/>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2653"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2515"/>
        <source>Cannot find layeroption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2657"/>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2680"/>
        <source>:&amp;nbsp;no input</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2528"/>
        <source>Cannot find whereoption </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2059"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1972"/>
        <source>Cannot find typeoption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1981"/>
        <source>Cannot find values for typeoption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2042"/>
        <source>Cannot find layeroption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2059"/>
        <source>GRASS element </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2059"/>
        <source> not supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2083"/>
        <source>Use region of this map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2419"/>
        <source>:&amp;nbsp;no input</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1888"/>
        <source>:&amp;nbsp;missing value</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2791"/>
        <source>Attribute field</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="873"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="356"/>
        <source>Cannot find module </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="373"/>
        <source>Cannot start module </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="386"/>
        <source>Cannot read module description (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="386"/>
        <source>):
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="387"/>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="387"/>
        <source> column </source>
        <translation type="unfinished">coluna</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="411"/>
        <source>Cannot find key </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="547"/>
        <source>Item with id </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="547"/>
        <source> not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="835"/>
        <source>Cannot get current region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="812"/>
        <source>Cannot check region of map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="874"/>
        <source>Cannot set region of map </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="114"/>
        <source>GRASS database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="115"/>
        <source>GRASS location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="116"/>
        <source>Projection</source>
        <translation type="unfinished">Projeção</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="117"/>
        <source>Default GRASS Region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="174"/>
        <source>Mapset</source>
        <translation type="unfinished">Conjunto de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="119"/>
        <source>Create New Mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="148"/>
        <source>Tree</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="149"/>
        <source>Comment</source>
        <translation type="unfinished">Comentário</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="150"/>
        <source>Database</source>
        <translation type="unfinished">Banco de Dados</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="154"/>
        <source>Location 2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="165"/>
        <source>User&apos;s mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="167"/>
        <source>System mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="161"/>
        <source>Location 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="175"/>
        <source>Owner</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="224"/>
        <source>Enter path to GRASS database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="232"/>
        <source>The directory doesn&apos;t exist!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="262"/>
        <source>No writable locations, the database not writable!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="360"/>
        <source>Enter location name!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="373"/>
        <source>The location exists!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="521"/>
        <source>Selected projection is not supported by GRASS!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1148"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="568"/>
        <source>Cannot create projection.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="617"/>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="752"/>
        <source>North must be greater than south</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="757"/>
        <source>East must be greater than west</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="804"/>
        <source>Regions file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="804"/>
        <source>) not found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="809"/>
        <source>Cannot open locations file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="809"/>
        <source>)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="818"/>
        <source>Cannot read locations file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="819"/>
        <source>):
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="819"/>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="820"/>
        <source> column </source>
        <translation type="unfinished">coluna</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1149"/>
        <source>Cannot create QgsSpatialRefSys</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="956"/>
        <source>Cannot reproject selected region.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1045"/>
        <source>Cannot reproject region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1277"/>
        <source>Enter mapset name.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1294"/>
        <source>The mapset already exists</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1318"/>
        <source>Database: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1329"/>
        <source>Location: </source>
        <translation type="unfinished">Local: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1331"/>
        <source>Mapset: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1362"/>
        <source>Create location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1364"/>
        <source>Cannot create new location: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1411"/>
        <source>Create mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1404"/>
        <source>Cannot open DEFAULT_WIND</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1411"/>
        <source>Cannot open WIND</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1438"/>
        <source>New mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1434"/>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1440"/>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1394"/>
        <source>Cannot create new mapset directory</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2068"/>
        <source>Column 1</source>
        <translation type="unfinished">Coluna 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="88"/>
        <source>Example directory tree:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="95"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS data are stored in tree directory structure.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="399"/>
        <source>Database Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2153"/>
        <source>Database:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="440"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="457"/>
        <source>Select existing directory or create a new one:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="508"/>
        <source>Location</source>
        <translation type="unfinished">Localidade</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="535"/>
        <source>Select location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="552"/>
        <source>Create new location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="832"/>
        <source>Location Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="848"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1159"/>
        <source>Projection Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1174"/>
        <source>Coordinate system</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1186"/>
        <source>Projection</source>
        <translation type="unfinished">Projeção</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1193"/>
        <source>Not defined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1273"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1334"/>
        <source>Set current QGIS extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1376"/>
        <source>Set</source>
        <translation type="unfinished">Marcar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1396"/>
        <source>Region Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1441"/>
        <source>S</source>
        <translation type="unfinished">S</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1500"/>
        <source>W</source>
        <translation type="unfinished">W</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1555"/>
        <source>E</source>
        <translation type="unfinished">E</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1614"/>
        <source>N</source>
        <translation type="unfinished">N</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1699"/>
        <source>New mapset:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1988"/>
        <source>Mapset Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2045"/>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2101"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS mapset is a collection of maps used by one user. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A user can read maps from all mapsets in the location but &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2174"/>
        <source>Location:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2195"/>
        <source>Mapset:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="211"/>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="811"/>
        <source>&amp;GRASS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="158"/>
        <source>Open mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="159"/>
        <source>New mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="160"/>
        <source>Close mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="163"/>
        <source>Add GRASS vector layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="165"/>
        <source>Add GRASS raster layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="181"/>
        <source>Open GRASS tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="170"/>
        <source>Display Current Grass Region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="174"/>
        <source>Edit Current Grass Region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="176"/>
        <source>Edit Grass Vector layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="179"/>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="180"/>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="182"/>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="183"/>
        <source>Edit the current GRASS region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="184"/>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="93"/>
        <source>GrassVector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="94"/>
        <source>0.1</source>
        <translation type="unfinished">0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="95"/>
        <source>GRASS layer</source>
        <translation type="unfinished">camada do GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="177"/>
        <source>Create new Grass Vector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="780"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="488"/>
        <source>GRASS Edit is already running.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="497"/>
        <source>New vector name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="513"/>
        <source>Cannot create new vector: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="535"/>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="546"/>
        <source>Cannot start editing.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="583"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="593"/>
        <source>Cannot read current region: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="697"/>
        <source>Cannot open the mapset. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="715"/>
        <source>Cannot close mapset. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="771"/>
        <source>Cannot close current mapset. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="780"/>
        <source>Cannot open GRASS mapset. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="419"/>
        <source>Could not add raster layer: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="460"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="196"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="203"/>
        <source>Cannot read current region: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="460"/>
        <source>Cannot write region</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="13"/>
        <source>GRASS Region Settings</source>
        <translation>GRASS Configurações da Região</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="76"/>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="146"/>
        <source>W</source>
        <translation>W</translation>
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
        <translation>N-S Res</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="293"/>
        <source>Rows</source>
        <translation>linhas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="303"/>
        <source>Cols</source>
        <translation>Colunas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="316"/>
        <source>E-W Res</source>
        <translation>E-W Res</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="364"/>
        <source>Color</source>
        <translation>Cor</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="384"/>
        <source>Width</source>
        <translation>Largura</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="464"/>
        <source>OK</source>
        <translation>OK</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="75"/>
        <source>Select GRASS Raster Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="82"/>
        <source>Select GRASS mapcalc schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="90"/>
        <source>Select GRASS Mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="408"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="408"/>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="466"/>
        <source>Choose existing GISDBASE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="482"/>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="483"/>
        <source>Wrong GISDBASE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="500"/>
        <source>Select a map.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="501"/>
        <source>No map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="509"/>
        <source>No layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="510"/>
        <source>No layers available in this map</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="65"/>
        <source>Gisdbase</source>
        <translation>Fonte de Dados</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="78"/>
        <source>Location</source>
        <translation>Localidade</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="161"/>
        <source>Browse</source>
        <translation>Exibir</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="85"/>
        <source>Mapset</source>
        <translation>Conjunto de mapas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="118"/>
        <source>Map name</source>
        <translation>Nome do mapa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="125"/>
        <source>Layer</source>
        <translation>Layer</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="175"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="102"/>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Selecione ou digite o nome do mapa (caracteres &apos;*&apos; e &apos;?&apos;  são aceitos para rasters)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="21"/>
        <source>Add GRASS Layer</source>
        <translation>Adicionar Camada GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="168"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="19"/>
        <source>GRASS Shell</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="49"/>
        <source>Close</source>
        <translation type="unfinished">Fechar</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="117"/>
        <source>Modules</source>
        <translation type="unfinished">Módulos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="149"/>
        <source>Browser</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="99"/>
        <source>GRASS Tools</source>
        <translation type="unfinished">Ferramentas GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="376"/>
        <source>GRASS Tools: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="298"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="211"/>
        <source>Cannot find MSYS (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="233"/>
        <source>GRASS Shell is not compiled.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="283"/>
        <source>The config file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="283"/>
        <source>) not found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="287"/>
        <source>Cannot open config file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="287"/>
        <source>)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="295"/>
        <source>Cannot read config file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="296"/>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="296"/>
        <source> column </source>
        <translation type="unfinished">coluna</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPlugin</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="93"/>
        <source>&amp;Graticule Creator</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="94"/>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="136"/>
        <source>&amp;Graticules</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="101"/>
        <source>QGIS - Grid Maker</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="52"/>
        <source>Please enter the file name before pressing OK!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="62"/>
        <source>Longitude Interval is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="70"/>
        <source>Latitude Interval is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="78"/>
        <source>Longitude Origin is invalid - please correct and try again..</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="86"/>
        <source>Latitude Origin is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="94"/>
        <source>End Point Longitude is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="102"/>
        <source>End Point Latitude is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="162"/>
        <source>Choose a filename to save under</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="164"/>
        <source>ESRI Shapefile (*.shp)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation>Modelo de Plugin para o QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="39"/>
        <source>Graticule Builder</source>
        <translation>Gerador de Grade (Grid)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="259"/>
        <source>#000.00000; </source>
        <translation>#000.00000; </translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="195"/>
        <source>Latitude:</source>
        <translation>Latitude:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="209"/>
        <source>Longitude:</source>
        <translation>Longitude:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="238"/>
        <source>Latitude Interval:</source>
        <translation>Intervalo de latitude:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="252"/>
        <source>Longitude Interval:</source>
        <translation>Intervalo de longitude:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="94"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:10pt;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="101"/>
        <source>Type</source>
        <translation type="unfinished">Tipo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="113"/>
        <source>Point</source>
        <translation type="unfinished">Ponto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="123"/>
        <source>Line</source>
        <translation type="unfinished">Linha</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="130"/>
        <source>Polygon</source>
        <translation type="unfinished">Polígono</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="140"/>
        <source>Origin (lower left)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="183"/>
        <source>End point (upper right)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="226"/>
        <source>Graticle size (units in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="269"/>
        <source>Output (shape) file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="284"/>
        <source>Save As...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="185"/>
        <source>Quantum GIS Help - </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="191"/>
        <source>Failed to get the help text from the database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="214"/>
        <source>Error</source>
        <translation type="unfinished">Erro</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="215"/>
        <source>The QGIS help database is not installed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="139"/>
        <source>This help file does not exist for your language</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="142"/>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="157"/>
        <source>Quantum GIS Help</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="16"/>
        <source>QGIS Help</source>
        <translation>
Ajuda do QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="42"/>
        <source>&amp;Home</source>
        <translation>&amp;Início</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="45"/>
        <source>Alt+H</source>
        <translation>Alt+H</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="55"/>
        <source>&amp;Forward</source>
        <translation>&amp;Próximo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="58"/>
        <source>Alt+F</source>
        <translation>Alt+F</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="68"/>
        <source>&amp;Back</source>
        <translation>&amp;Anterior</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="71"/>
        <source>Alt+B</source>
        <translation>Alt+B</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="81"/>
        <source>&amp;Close</source>
        <translation>&amp;Fechar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="84"/>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
</context>
<context>
    <name>QgsHttpTransaction</name>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="230"/>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="309"/>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="358"/>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location filename="../src/core/qgshttptransaction.cpp" line="437"/>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResults</name>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="225"/>
        <source>Identify Results - </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="44"/>
        <source>Feature</source>
        <translation type="unfinished">Feição</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="45"/>
        <source>Value</source>
        <translation type="unfinished">Valor</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="106"/>
        <source>Run action</source>
        <translation type="unfinished">Rodar ação</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="196"/>
        <source>(Derived)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="16"/>
        <source>Identify Results</source>
        <translation>Identificar Resultados</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="46"/>
        <source>Help</source>
        <translation>Ajudar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="49"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="75"/>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
</context>
<context>
    <name>QgsLUDialogBase</name>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="13"/>
        <source>Enter class bounds</source>
        <translation>Entre o limite das classes</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="31"/>
        <source>Lower value</source>
        <translation>Valor inferior</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="57"/>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="94"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="101"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="126"/>
        <source>Upper value</source>
        <translation>Valor superior</translation>
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
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1148"/>
        <source>Field containing label:</source>
        <translation>Campo contendo o rótulo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1191"/>
        <source>Default label:</source>
        <translation>Rótulo Padrão:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="41"/>
        <source>Preview:</source>
        <translation>Prévia:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="56"/>
        <source>QGIS Rocks!</source>
        <translation>QGIS é animal!</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="78"/>
        <source>Font Style</source>
        <translation>Estilo da Fonte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="183"/>
        <source>Font</source>
        <translation>Fonte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="586"/>
        <source>Points</source>
        <translation>Pontos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="579"/>
        <source>Map units</source>
        <translation>Unidade do Mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="478"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="980"/>
        <source>Transparency:</source>
        <translation>Transparência:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="534"/>
        <source>Colour</source>
        <translation>Cor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="542"/>
        <source>Position</source>
        <translation>Posição</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1080"/>
        <source>X Offset (pts):</source>
        <translation>X Offset (pontos)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1116"/>
        <source>Y Offset (pts):</source>
        <translation>Y Offset (Pontos)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="451"/>
        <source>Buffer Labels?</source>
        <translation>Rótulos do Buffer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1005"/>
        <source>Size:</source>
        <translation>Tamanho:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="408"/>
        <source>Size is in map units</source>
        <translation>Tamanho em unidades do mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="415"/>
        <source>Size is in points</source>
        <translation>Tamanho em pontos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="304"/>
        <source>Above</source>
        <translation>Acima</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="294"/>
        <source>Over</source>
        <translation>Sobre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="311"/>
        <source>Left</source>
        <translation>Esquerda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="287"/>
        <source>Below</source>
        <translation>Abaixo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="280"/>
        <source>Right</source>
        <translation>Direita</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="325"/>
        <source>Above Right</source>
        <translation>Sobre à Direira</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="273"/>
        <source>Below Right</source>
        <translation>Abaixo à Direita</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="332"/>
        <source>Above Left</source>
        <translation>Sobre à Esquerda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="318"/>
        <source>Below Left</source>
        <translation>Abaixo à Esquerda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="893"/>
        <source>Angle (deg):</source>
        <translation>Ângulo (graus):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="625"/>
        <source>Data Defined Style</source>
        <translation>Estilo definido de dados</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="813"/>
        <source>&amp;Font family:</source>
        <translation>&amp;Família da Fonte:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="645"/>
        <source>&amp;Italic:</source>
        <translation>&amp;Itálico:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="719"/>
        <source>&amp;Underline:</source>
        <translation>&amp;Subescrito:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="681"/>
        <source>&amp;Bold:</source>
        <translation>&amp;Negrito:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="663"/>
        <source>&amp;Size:</source>
        <translation>&amp;Tamanho:</translation>
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
        <location filename="../src/ui/qgslabeldialogbase.ui" line="914"/>
        <source>Placement:</source>
        <translation>Posicionamento:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="103"/>
        <source>Font size units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="236"/>
        <source>Font Alignment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="261"/>
        <source>Placement</source>
        <translation type="unfinished">Posicionamento</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="384"/>
        <source>Buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="396"/>
        <source>Buffer size units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="567"/>
        <source>Offset units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="847"/>
        <source>Data Defined Alignment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="922"/>
        <source>Data Defined Buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1016"/>
        <source>Data Defined Position</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1136"/>
        <source>Source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="767"/>
        <source>Size Units:</source>
        <translation type="unfinished"></translation>
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
        <translation>Seletor de Projeção para a Camada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="83"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="60"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Define this layer&apos;s projection:&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This layer appears to have no projection specification. By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="110"/>
        <source>group</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="423"/>
        <source>&amp;Remove</source>
        <translation type="unfinished">&amp;Remover</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="416"/>
        <source>&amp;Make to toplevel item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="428"/>
        <source>Re&amp;name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="433"/>
        <source>&amp;Add group</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="434"/>
        <source>&amp;Expand all</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="435"/>
        <source>&amp;Collapse all</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="437"/>
        <source>Show file groups</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1818"/>
        <source>No Layer Selected</source>
        <translation type="unfinished">Nenhuma Camada Selecionada</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1819"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="474"/>
        <source>&amp;Zoom to layer extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="477"/>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="481"/>
        <source>&amp;Show in overview</source>
        <translation>&amp;Mostrar no overview</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="487"/>
        <source>&amp;Remove</source>
        <translation type="unfinished">&amp;Remover</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="494"/>
        <source>&amp;Open attribute table</source>
        <translation type="unfinished">&amp;Abrir tabela de atributos</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="518"/>
        <source>Save as shapefile...</source>
        <translation>Salvar como arquivo SHP...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="525"/>
        <source>Save selection as shapefile...</source>
        <translation>Salva a seleção como arquivo SHP</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="535"/>
        <source>&amp;Properties</source>
        <translation type="unfinished">&amp;Propriedades</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="584"/>
        <source>More layers</source>
        <translation>Mais camadas</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="585"/>
        <source>This item contains more layer files. Displaying more layers in table is not supported.</source>
        <translation>Este item contém mais arquivos de camadas. Exibir mais camadas na tabela não é suportado.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="277"/>
        <source>Attribute table - </source>
        <translation type="unfinished">Tabela de atributos - </translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="347"/>
        <source>Save layer as...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="427"/>
        <source>Start editing failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="428"/>
        <source>Provider cannot be opened for editing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="441"/>
        <source>Stop editing</source>
        <translation type="unfinished">Parar edição</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="442"/>
        <source>Do you want to save the changes?</source>
        <translation type="unfinished">Você quer salvar as alterações?</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="460"/>
        <source>Error</source>
        <translation type="unfinished">Erro</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="449"/>
        <source>Could not commit changes</source>
        <translation type="unfinished">Impossível modificar</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="461"/>
        <source>Problems during roll back</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="227"/>
        <source>Not a vector layer</source>
        <translation type="unfinished">Não é uma camada vetorial</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="228"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="394"/>
        <source>Saving done</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="394"/>
        <source>Export to Shapefile has been completed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="398"/>
        <source>Driver not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="398"/>
        <source>ESRI Shapefile driver is not available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="402"/>
        <source>Error creating shapefile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="403"/>
        <source>The shapefile could not be created (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="407"/>
        <source>Layer creation failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="494"/>
        <source>&amp;Zoom to layer extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="497"/>
        <source>&amp;Show in overview</source>
        <translation type="unfinished">&amp;Mostrar no overview</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="505"/>
        <source>&amp;Remove</source>
        <translation type="unfinished">&amp;Remover</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="514"/>
        <source>&amp;Open attribute table</source>
        <translation type="unfinished">&amp;Abrir tabela de atributos</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="528"/>
        <source>Save as shapefile...</source>
        <translation type="unfinished">Salvar como shapefile...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="530"/>
        <source>Save selection as shapefile...</source>
        <translation type="unfinished">Salva a seleção como arquivo SHP</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="547"/>
        <source>&amp;Properties</source>
        <translation type="unfinished">&amp;Propriedades</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="271"/>
        <source>bad_alloc exception</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="271"/>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="411"/>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLineStyleDialogBase</name>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="13"/>
        <source>Select a line style</source>
        <translation>Selecionar um estilo de linha</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="28"/>
        <source>Styles</source>
        <translation>Estilos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="177"/>
        <source>Ok</source>
        <translation>Ok</translation>
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
        <translation>Estilo de Contorno</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="61"/>
        <source>Width:</source>
        <translation>Largura:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="87"/>
        <source>Colour:</source>
        <translation>Cor:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="98"/>
        <source>LineStyleWidget</source>
        <translation>Estilo de Linha da Janela</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1134"/>
        <source>because</source>
        <translation type="unfinished">porque</translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="430"/>
        <source>No features found</source>
        <translation type="unfinished">Nenhuma feição encontrada</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="433"/>
        <source>&lt;p&gt;No features were found within the search radius. Note that it is currently not possible to use the identify tool on unsaved features.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="289"/>
        <source>(clicked coordinate)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="222"/>
        <source>WMS identify result for %1
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="356"/>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="248"/>
        <source>Snap tolerance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="249"/>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished">Não mostra esta mensagem novamente</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="254"/>
        <source>Could not snap segment.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="257"/>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="76"/>
        <source>Name for the map file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="84"/>
        <source>Choose the QGIS project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="85"/>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="197"/>
        <source>Overwrite File?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="199"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a filename is prepended to this text, and appears in a dialog box</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmapserverexport.cpp" line="74"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="77"/>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="14"/>
        <source>Export to Mapserver</source>
        <translation>Exportar para Mapserver</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="352"/>
        <source>Map file</source>
        <translation>Arquivo Map</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="403"/>
        <source>Export LAYER information only</source>
        <translation>Exportar apenas a informação da CAMADA</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="159"/>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="330"/>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="304"/>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="291"/>
        <source>Width</source>
        <translation>Largura</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="196"/>
        <source>dd</source>
        <translation>dd</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="201"/>
        <source>feet</source>
        <translation>pés</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="206"/>
        <source>meters</source>
        <translation>metros</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="211"/>
        <source>miles</source>
        <translation>milhas</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="216"/>
        <source>inches</source>
        <translation>polegadas</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="221"/>
        <source>kilometers</source>
        <translation>quilômetros</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="182"/>
        <source>Units</source>
        <translation>Unidades</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="229"/>
        <source>Image type</source>
        <translation>Tipo de imagem</translation>
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
        <translation>Definição do usuário</translation>
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
        <translation>Escala Máxima</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="252"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation>Prefixo atachado ao mapa, barra de escalas e legendas arquivos GIF criados usando este MapFile. Deve ser mantido brevemente.</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="29"/>
        <source>Web Interface Definition</source>
        <translation>Definição de interface Web </translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="98"/>
        <source>Header</source>
        <translation>Cabeçalho</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="134"/>
        <source>Footer</source>
        <translation>Rodapé</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="58"/>
        <source>Template</source>
        <translation>Modelo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="56"/>
        <source>&amp;Help</source>
        <translation>&amp;Ajuda</translation>
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
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="116"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="362"/>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="400"/>
        <source>If checked, only the layer information will be processed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="68"/>
        <source>Path to the MapServer template file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="340"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="376"/>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="383"/>
        <source>QGIS project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="369"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="393"/>
        <source>Save As...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMarkerDialogBase</name>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="16"/>
        <source>Choose a marker symbol</source>
        <translation>Escolha um símbolo para marcador</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="28"/>
        <source>Directory</source>
        <translation>Diretório</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="38"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="71"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="81"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="108"/>
        <source>New Item</source>
        <translation>Novo Item</translation>
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
        <location filename="../src/ui/qgsmeasurebase.ui" line="145"/>
        <source>New</source>
        <translation>Novo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="122"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="152"/>
        <source>Cl&amp;ose</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="69"/>
        <source>Total:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="206"/>
        <source>Segments (in meters)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="209"/>
        <source>Segments (in feet)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="212"/>
        <source>Segments (in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="215"/>
        <source>Segments</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="73"/>
        <source>Incorrect measure results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="81"/>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="13"/>
        <source>QGIS Message</source>
        <translation>Mensagem do QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="48"/>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="28"/>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished">Não mostra esta mensagem novamente</translation>
    </message>
</context>
<context>
    <name>QgsMySQLProvider</name>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="166"/>
        <source>Unable to access relation</source>
        <translation type="unfinished">Impossível de acessar relação</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="167"/>
        <source>Unable to access the </source>
        <translation type="unfinished">Impossível de acessar o </translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="169"/>
        <source> relation.
The error message from the database was:
</source>
        <translation type="unfinished"> relação. 
A mensagem de erro do banco de dados foi:</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="186"/>
        <source>No GEOS Support!</source>
        <translation type="unfinished">Sem suporte ao GEOS!</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="189"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="unfinished">Sua instalação do PostGIS não têm suporte ao GEOS. 
Seleção e identificação de feições não irá funcionar corretamente. 
Por favor instale o PostGIS com o suporte ao GEOS
(http://geos.refractions.net)</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="813"/>
        <source>Save layer as...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="965"/>
        <source>Error</source>
        <translation type="unfinished">Erro</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="886"/>
        <source>Error creating field </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="965"/>
        <source>Layer creation failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="971"/>
        <source>Error creating shapefile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="973"/>
        <source>The shapefile could not be created (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="981"/>
        <source>Driver not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="982"/>
        <source> driver is not available</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="115"/>
        <source>Test connection</source>
        <translation>Testar conexão</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="115"/>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>A conexão falhou. Revise sua configuração e tente novamente. 

Informações adicionais do erro:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="112"/>
        <source>Connection to %1 was successful</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="21"/>
        <source>Create a New PostGIS connection</source>
        <translation>Criar nova conexão PostGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="255"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="271"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="287"/>
        <source>Help</source>
        <translation>Ajuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="39"/>
        <source>Connection Information</source>
        <translation>Informação da Conexão</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="150"/>
        <source>Host</source>
        <translation>Máquina</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="160"/>
        <source>Database</source>
        <translation>Banco de Dados</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="180"/>
        <source>Username</source>
        <translation>Usuário</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="140"/>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="210"/>
        <source>Name of the new connection</source>
        <translation>Nome da nova conexão</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="190"/>
        <source>Password</source>
        <translation>Senha</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="115"/>
        <source>Test Connect</source>
        <translation>Testar Conexão</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="108"/>
        <source>Save Password</source>
        <translation>Salvar Senha</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="290"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="170"/>
        <source>Port</source>
        <translation>Porta</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="223"/>
        <source>5432</source>
        <translation>5432</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="64"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Restrict the search to the public schema for spatial tables not in the geometry_columns table&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="67"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="70"/>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="87"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;&quot;&gt;Restrict the displayed tables to those that are in the geometry_columns table&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="90"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;&quot;&gt;Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="93"/>
        <source>Only look in the geometry_columns table</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="13"/>
        <source>Create a New WMS connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="31"/>
        <source>Connection Information</source>
        <translation type="unfinished">Informação da Conexão</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="72"/>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="85"/>
        <source>URL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="98"/>
        <source>Proxy Host</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="111"/>
        <source>Proxy Port</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="124"/>
        <source>Proxy User</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="137"/>
        <source>Proxy Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="158"/>
        <source>Your user name for the HTTP proxy (optional)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="173"/>
        <source>Password for your HTTP proxy (optional)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="62"/>
        <source>Name of the new connection</source>
        <translation type="unfinished">Nome da nova conexão</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="183"/>
        <source>HTTP address of the Web Map Server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="190"/>
        <source>Name of your HTTP proxy (optional)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="205"/>
        <source>Port number of your HTTP proxy (optional)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="226"/>
        <source>OK</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="242"/>
        <source>Cancel</source>
        <translation type="unfinished">Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="258"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="261"/>
        <source>F1</source>
        <translation type="unfinished">F1</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="80"/>
        <source>Bottom Left</source>
        <translation>Inferior Esquerdo</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="81"/>
        <source>Top Right</source>
        <translation>Superior Direito</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="81"/>
        <source>Bottom Right</source>
        <translation>Inferior Direito</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="81"/>
        <source>Top Left</source>
        <translation type="unfinished">Superior Esquerdo</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="94"/>
        <source>&amp;North Arrow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="95"/>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="250"/>
        <source>&amp;Decorations</source>
        <translation type="unfinished">&amp;Decorações</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="241"/>
        <source>North arrow pixmap not found</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugingui.cpp" line="157"/>
        <source>Pixmap not found</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="237"/>
        <source>North Arrow Plugin</source>
        <translation>Plugin de Rosa do Ventos</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="35"/>
        <source>Properties</source>
        <translation>Propriedades</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="55"/>
        <source>Angle</source>
        <translation>Ângulo</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="62"/>
        <source>Placement</source>
        <translation>Posicionamento</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="75"/>
        <source>Set direction automatically</source>
        <translation>Configurar a direção automaticamente</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="85"/>
        <source>Enable North Arrow</source>
        <translation>Habilitar Rosa dos Ventos</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="141"/>
        <source>Top Left</source>
        <translation>Superior Esquerdo</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="146"/>
        <source>Top Right</source>
        <translation>Superior Direito</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="151"/>
        <source>Bottom Left</source>
        <translation>Inferior Esquerdo</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="156"/>
        <source>Bottom Right</source>
        <translation>Inferior Direito</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="137"/>
        <source>Placement on screen</source>
        <translation>Posicionamento na Tela</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="164"/>
        <source>Preview of north arrow</source>
        <translation>Pré-visualização da rosa dos ventos</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="183"/>
        <source>Icon</source>
        <translation>Ícone</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="206"/>
        <source>New Item</source>
        <translation>Novo Item</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="198"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsOGRFactory</name>
    <message>
        <location filename="../src/providers/ogr/qgsogrfactory.cpp" line="63"/>
        <source>Wrong Path/URI</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/ogr/qgsogrfactory.cpp" line="63"/>
        <source>The provided path for the dataset is not valid.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="149"/>
        <source>Detected active locale on your system: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsOptionsBase</name>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="13"/>
        <source>QGIS Options</source>
        <translation>Opções do QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="869"/>
        <source>epiphany</source>
        <translation>epiphany</translation>
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
        <location filename="../src/ui/qgsoptionsbase.ui" line="613"/>
        <source>Search Radius for Identifying Features</source>
        <translation>Raio de busca para identificação de Feições</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="110"/>
        <source>Hide splash screen at startup</source>
        <translation>Não exibir a tela inicial (splash screen)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="86"/>
        <source>&amp;Appearance</source>
        <translation>Apa&amp;rência</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="120"/>
        <source>&amp;Icon Theme</source>
        <translation>Ícone &amp;Tema</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="150"/>
        <source>Theme</source>
        <translation>Tema</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="132"/>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Nota:&lt;/b&gt;Mudanças no tema somente terão efeito na próxima vez que o QGIS for iniciado</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="817"/>
        <source>Help &amp;Browser</source>
        <translation>&amp;Navegador de Ajuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="907"/>
        <source>Open help documents with</source>
        <translation>Abrir ajuda com</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="311"/>
        <source>&amp;Rendering</source>
        <translation>&amp;Renderizar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="355"/>
        <source>Update display after reading</source>
        <translation>Atualizar tela após renderizar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="342"/>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>A visualização do mapa será atualizada (desenhada) após este número de feições ter sido lido</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="335"/>
        <source>features</source>
        <translation>feições</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="365"/>
        <source>(Set to 0 to not update the display until all features have been read)</source>
        <translation>(Defina 0 para não atualizar a visualização até que todas as feições tenham sido lidas)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="858"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="375"/>
        <source>Initial Visibility</source>
        <translation>Visibilidade Inicial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="681"/>
        <source>Select Global Default ...</source>
        <translation>Selecione o Padrão Global</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="703"/>
        <source>Prompt for projection.</source>
        <translation>Perguntar a projeção.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="710"/>
        <source>Project wide default projection will be used.</source>
        <translation>A projeção padrão do projeto será usada.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="98"/>
        <source>&amp;Splash screen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="188"/>
        <source>Default Map Appearance (Overridden by project properties)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="236"/>
        <source>Background Color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="200"/>
        <source>Selection Color:</source>
        <translation type="unfinished">Cor da Seleção:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="275"/>
        <source>Appearance</source>
        <translation type="unfinished">Aparência</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="287"/>
        <source>Capitalise layer name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="409"/>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="387"/>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="323"/>
        <source>&amp;Update during drawing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="523"/>
        <source>Measure tool</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="574"/>
        <source>Ellipsoid for distance calculations:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="587"/>
        <source>Search radius</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="656"/>
        <source>Pro&amp;jection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="691"/>
        <source>When layer is loaded that has no projection information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="717"/>
        <source>Global default projection displa&amp;yed below will be used.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="842"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; The browser must be in your PATH or you can specify the full path above</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="874"/>
        <source>firefox</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="879"/>
        <source>mozilla-firefox</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="397"/>
        <source>Rendering</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="416"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Selecting this will unselect the &apos;make lines less&apos; jagged toggle&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="419"/>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="426"/>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="450"/>
        <source>&amp;Map tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="599"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Note:&lt;/span&gt; Specify the search radius as a percentage of the map width.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="623"/>
        <source>%</source>
        <translation type="unfinished">%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="462"/>
        <source>Panning and zooming</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="475"/>
        <source>Zoom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="480"/>
        <source>Zoom and recenter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="485"/>
        <source>Nothing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="493"/>
        <source>Zoom factor:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="500"/>
        <source>Mouse wheel action:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="38"/>
        <source>&amp;General</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="50"/>
        <source>General</source>
        <translation type="unfinished">Geral</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="62"/>
        <source>Ask to save project changes when required</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="564"/>
        <source>Rubberband color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="728"/>
        <source>Locale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="740"/>
        <source>Force Override System Locale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="755"/>
        <source>Locale to use instead</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="768"/>
        <source>Note: Enabling / changing overide on local requires an application restart.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="794"/>
        <source>Additional Info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="806"/>
        <source>Detected active locale on your system:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="19"/>
        <source>Paste Transformations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="42"/>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="65"/>
        <source>Source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="86"/>
        <source>Destination</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="125"/>
        <source>&amp;Help</source>
        <translation type="unfinished">&amp;Ajuda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="128"/>
        <source>F1</source>
        <translation type="unfinished">F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="154"/>
        <source>Add New Transfer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="161"/>
        <source>&amp;OK</source>
        <translation type="unfinished">&amp;OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="177"/>
        <source>&amp;Cancel</source>
        <translation type="unfinished">&amp;Cancelar</translation>
    </message>
</context>
<context>
    <name>QgsPatternDialogBase</name>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="13"/>
        <source>Select a fill pattern</source>
        <translation>Selecione um padrão de preenchimento</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="205"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="212"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="189"/>
        <source>No Fill</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="117"/>
        <source>Buffer features in layer %1</source>
        <translation>Feições de buffer na camada %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="340"/>
        <source>Error connecting to the database</source>
        <translation>Erro na conexão com o banco de dados</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="78"/>
        <source>&amp;Buffer features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="410"/>
        <source>&amp;Geoprocessing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="325"/>
        <source>Unable to add geometry column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="327"/>
        <source>Unable to add geometry column to the output table </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="331"/>
        <source>Unable to create table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="333"/>
        <source>Failed to create the output table </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="346"/>
        <source>No GEOS support</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="347"/>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="356"/>
        <source>No Active Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="357"/>
        <source>You must select a layer in the legend to buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="350"/>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="74"/>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Tabela &lt;b&gt;%1&lt;/b&gt; no banco de dados &lt;b&gt;%2&lt;/b&gt; no servidor &lt;b&gt;%3&lt;/b&gt;, usuário &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="59"/>
        <source>Connection Failed</source>
        <translation>A Conexão Falhou</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="59"/>
        <source>Connection to the database failed:</source>
        <translation>Conexão com o banco de dados falhou:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="222"/>
        <source>Database error</source>
        <translation>Erro no banco de dados</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="222"/>
        <source>Failed to get sample of field values</source>
        <translation>Falha ao pegar uma amostra dos valores da tabela</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="248"/>
        <source>Query Result</source>
        <translation>Resultado da Consulta</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="250"/>
        <source>The where clause returned </source>
        <translation>A cláusula where retornou</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="250"/>
        <source> rows.</source>
        <translation> colunas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="254"/>
        <source>Query Failed</source>
        <translation>A Consulta Falhou</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="256"/>
        <source>An error occurred when executing the query:</source>
        <translation>Um erro ocorreu quando foi executada a consulta:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="309"/>
        <source>No Records</source>
        <translation>Nenhum registro encontrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="309"/>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>A consulta que você especificou retornou que não há nenhum registro encontrado. Camadas válidas do PostgreSQL precisam ter pelo menos uma feição.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="182"/>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="237"/>
        <source>No Query</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="237"/>
        <source>You must create a query before you can test it</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="303"/>
        <source>Error in Query</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="23"/>
        <source>PostgreSQL Query Builder</source>
        <translation>Construtor de Consulta PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="178"/>
        <source>Clear</source>
        <translation>Limpar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="188"/>
        <source>Test</source>
        <translation>Testar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="198"/>
        <source>Ok</source>
        <translation>Ok</translation>
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
        <translation>Tudo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="263"/>
        <source>Sample</source>
        <translation>Amostra</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="273"/>
        <source>Fields</source>
        <translation>Campos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="291"/>
        <source>Datasource:</source>
        <translation>Origem dos Dados:</translation>
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
        <translation>IN</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="102"/>
        <source>NOT IN</source>
        <translation>NOT IN</translation>
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
        <translation>LIKE</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="81"/>
        <source>AND</source>
        <translation>AND</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="130"/>
        <source>ILIKE</source>
        <translation>ILIKE</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="74"/>
        <source>OR</source>
        <translation>OR</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="67"/>
        <source>NOT</source>
        <translation>NOT</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="304"/>
        <source>SQL where clause</source>
        <translation>Cláusula SQL where</translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <location filename="" line="7471221"/>
        <source>Choose a directory</source>
        <translation type="obsolete">Escolha um diretório</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="180"/>
        <source>No Plugins</source>
        <translation>Sem Plugins</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="180"/>
        <source>No QGIS plugins found in </source>
        <translation>Nenhum plugin QGIS encontrado em</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="76"/>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="77"/>
        <source>Version</source>
        <translation type="unfinished">Versão</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="78"/>
        <source>Description</source>
        <translation type="unfinished">Descrição</translation>
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
        <location filename="" line="7471221"/>
        <source>QGIS Plugin Manger</source>
        <translation type="obsolete">Gerenciador de Plugin do QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="39"/>
        <source>Plugin Directory</source>
        <translation>Diretório com Plugins</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Name</source>
        <translation type="obsolete">Nome</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Version</source>
        <translation type="obsolete">Versão</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Description</source>
        <translation type="obsolete">Descrição</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Library Name</source>
        <translation type="obsolete">Nome da Biblioteca</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="61"/>
        <source>To load a plugin, click the checkbox next to the plugin and click Ok</source>
        <translation>Para carregar um plugin, clique no checkbox ao lado do plugin e clique em Ok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="92"/>
        <source>&amp;Select All</source>
        <translation>&amp;Selecionar Tudo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="95"/>
        <source>Alt+S</source>
        <translation>Atl+S</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="102"/>
        <source>C&amp;lear All</source>
        <translation>&amp;Limpar Tudo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="105"/>
        <source>Alt+L</source>
        <translation>Alt+L</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="112"/>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="115"/>
        <source>Alt+O</source>
        <translation>Alt+O</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="122"/>
        <source>&amp;Close</source>
        <translation>&amp;Fechar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="125"/>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="16"/>
        <source>QGIS Plugin Manager</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="487"/>
        <source>Zoom In</source>
        <translation type="unfinished">Aproximar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="486"/>
        <source>z</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="492"/>
        <source>Zoom Out</source>
        <translation type="unfinished">Afastar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="491"/>
        <source>Z</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="495"/>
        <source>Zoom To Layer</source>
        <translation type="unfinished">Ver a camada</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="497"/>
        <source>Zoom to Layer</source>
        <translation type="unfinished">Ver a camada</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="500"/>
        <source>Pan Map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="501"/>
        <source>Pan the map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="504"/>
        <source>Add Point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="505"/>
        <source>.</source>
        <translation type="unfinished">.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="506"/>
        <source>Capture Points</source>
        <translation type="unfinished">Captura pontos</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="509"/>
        <source>Delete Point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="510"/>
        <source>Delete Selected</source>
        <translation type="unfinished">Excluir seleção</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="556"/>
        <source>Linear</source>
        <translation type="unfinished">Linear</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="557"/>
        <source>Helmert</source>
        <translation type="unfinished">Helmert</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="197"/>
        <source>Choose a name for the world file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="215"/>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given filename</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="264"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="279"/>
        <source>Affine</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="289"/>
        <source>Not implemented!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="284"/>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="291"/>
        <source>&lt;p&gt;The </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="292"/>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="323"/>
        <source>Error</source>
        <translation type="unfinished">Erro</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="324"/>
        <source>Could not write to </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="271"/>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="270"/>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="65"/>
        <source>Transform type:</source>
        <translation>Tipo de Transformação:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="178"/>
        <source>Zoom in</source>
        <translation>Aumentar Zoom</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="200"/>
        <source>Zoom out</source>
        <translation>Menos Zoom</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="222"/>
        <source>Zoom to the raster extents</source>
        <translation>Zoom para a extensão do raster</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="244"/>
        <source>Pan</source>
        <translation>Movimentar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="105"/>
        <source>Add points</source>
        <translation>Adicionar pontos</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="130"/>
        <source>Delete points</source>
        <translation>Apagar pontos</translation>
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
        <translation>Raster modificado:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="13"/>
        <source>Reference points</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="75"/>
        <source>Create</source>
        <translation type="unfinished">Criar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="282"/>
        <source>Create and load layer</source>
        <translation type="unfinished"></translation>
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
        <translation>Estilo do Símbolo</translation>
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
        <translation>Impossível de acessar relação</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="118"/>
        <source>Unable to access the </source>
        <translation>Impossível de acessar o </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="120"/>
        <source> relation.
The error message from the database was:
</source>
        <translation> relação. 
A mensagem de erro do banco de dados foi:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="284"/>
        <source>No GEOS Support!</source>
        <translation>Sem suporte ao GEOS!</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="288"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>Sua instalação do PostGIS não têm suporte ao GEOS. 
Seleção e identificação de feições não irá funcionar corretamente. 
Por favor instale o PostGIS com o suporte ao GEOS
(http://geos.refractions.net)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="875"/>
        <source>No suitable key column in table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="879"/>
        <source>The table has no column suitable for use as a key.

Qgis requires that the table either has a column of type
int4 with a unique constraint on it (which includes the
primary key) or has a PostgreSQL oid column.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="923"/>
        <source>The unique index on column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="925"/>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="950"/>
        <source>and </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="955"/>
        <source>The unique index based on columns </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="957"/>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1003"/>
        <source>Unable to find a key column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1086"/>
        <source> derives from </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1090"/>
        <source>and is suitable.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1094"/>
        <source>and is not suitable </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1095"/>
        <source>type is </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1097"/>
        <source> and has a suitable constraint)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1099"/>
        <source> and does not have a suitable constraint)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1199"/>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1205"/>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1206"/>
        <source>The view </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1207"/>
        <source>has no column suitable for use as a unique key.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1208"/>
        <source>No suitable key column in view</source>
        <translation type="unfinished">Sem chave de coluna adequada na visão</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2601"/>
        <source>Unknown geometry type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2602"/>
        <source>Column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2612"/>
        <source> in </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2604"/>
        <source> has a geometry type of </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2604"/>
        <source>, which Qgis does not currently support.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2613"/>
        <source>. The database communication log was:
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2614"/>
        <source>Unable to get feature type and srid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1185"/>
        <source>Note: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1187"/>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1812"/>
        <source>INSERT error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1806"/>
        <source>An error occured during feature insertion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1865"/>
        <source>DELETE error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1859"/>
        <source>An error occured during deletion from disk</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2179"/>
        <source>PostGIS error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2181"/>
        <source>When trying: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2173"/>
        <source>An error occured contacting the PostgreSQL database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2180"/>
        <source>The PostgreSQL database returned: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2611"/>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsProjectPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="13"/>
        <source>Project Properties</source>
        <translation>Propriedades do Projeto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="85"/>
        <source>Map Units</source>
        <translation>Unidade do Mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="97"/>
        <source>Meters</source>
        <translation>Metros</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="107"/>
        <source>Feet</source>
        <translation>Pés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="114"/>
        <source>Decimal degrees</source>
        <translation>Graus Decimais</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="75"/>
        <source>Default project title</source>
        <translation>Título padrão do projeto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="48"/>
        <source>General</source>
        <translation>Geral</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="231"/>
        <source>Line Width:</source>
        <translation>Largura da Linha:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="287"/>
        <source>Line Colour:</source>
        <translation>Cor da Linha:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="155"/>
        <source>Automatic</source>
        <translation>Automática</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="149"/>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>Ajustar automaticamente o número de casas decimais na posição (geográfica) mostrada pelo mouse</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="152"/>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>O número de casas decimais que são usadas para mostrar a posição (geográfica) do mouse é automaticamente ajustada de forma a ser suficientemente precisa, dessa forma, ao mover na tela o mouse em um pixel resulta em uma mudança na posição.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="171"/>
        <source>Manual</source>
        <translation>Manual</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="168"/>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Ajusta o número de casas decimais á ser usada para mostrar a posição (geográfica) do mouse</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="181"/>
        <source>The number of decimal places for the manual option</source>
        <translation>O número de casas decimais para a opção manual</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="196"/>
        <source>decimal places</source>
        <translation>casas decimais</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="300"/>
        <source>Map Appearance</source>
        <translation>Aparência do Mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="312"/>
        <source>Selection Color:</source>
        <translation>Cor da Seleção:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="60"/>
        <source>Project Title</source>
        <translation>Título do Projeto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="404"/>
        <source>Projection</source>
        <translation>Projeção</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="416"/>
        <source>Enable on the fly projection</source>
        <translation>Habilitar a aplicação de projeção em tempo real</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="351"/>
        <source>Background Color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="137"/>
        <source>Precision</source>
        <translation type="unfinished">Precisão</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="219"/>
        <source>Digitizing</source>
        <translation type="unfinished">Digitalizar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="72"/>
        <source>Descriptive project name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="241"/>
        <source>Line width in pixels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="280"/>
        <source>Snapping tolerance in map units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="264"/>
        <source>Snapping Tolerance (in map units):</source>
        <translation type="unfinished"></translation>
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
        <translation>Seletor de Projeção</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="47"/>
        <source>Projection</source>
        <translation>Projeção</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="67"/>
        <source>Search</source>
        <translation>Procurar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="85"/>
        <source>Find</source>
        <translation>Encontrar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="137"/>
        <source>Postgis SRID</source>
        <translation>Postgis SRID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="124"/>
        <source>EPSG ID</source>
        <translation>EPSG ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="111"/>
        <source>QGIS SRSID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="98"/>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="196"/>
        <source>Spatial Reference System</source>
        <translation type="unfinished">Sistema de referência espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="201"/>
        <source>Id</source>
        <translation type="unfinished">ID</translation>
    </message>
</context>
<context>
    <name>QgsPythonDialog</name>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="13"/>
        <source>Python console</source>
        <translation type="unfinished">Console Python</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="33"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans Condensed&apos;; font-size:10pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;To access Quantum GIS environment from this python console use object &lt;span style=&quot; font-weight:600;&quot;&gt;iface&lt;/span&gt; from global scope which is an instance of QgisInterface class.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Usage e.g.: iface.zoomFull()&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="65"/>
        <source>&gt;&gt;&gt;</source>
        <translation type="unfinished"></translation>
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
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3245"/>
        <source>Not Set</source>
        <translation>Não Ajustado</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3862"/>
        <source>Driver:</source>
        <translation>Driver:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3938"/>
        <source>Dimensions:</source>
        <translation>Dimensões:</translation>
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
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4050"/>
        <source>Origin:</source>
        <translation>Origem:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4059"/>
        <source>Pixel Size:</source>
        <translation>Tamanho do Pixel:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2633"/>
        <source>Raster Extent: </source>
        <translation>Extensão do Raster: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2636"/>
        <source>Clipped area: </source>
        <translation>Area clipada: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4001"/>
        <source>Pyramid overviews:</source>
        <translation>Visões da Pirâmide:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4076"/>
        <source>Property</source>
        <translation>Propriedade</translation>
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
        <translation>No da Banda</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4107"/>
        <source>No Stats</source>
        <translation>Sem Informações</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4110"/>
        <source>No stats collected yet</source>
        <translation>Nenhuma informação coletada até o momento</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4120"/>
        <source>Min Val</source>
        <translation>Val Min</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4128"/>
        <source>Max Val</source>
        <translation>Val Máx</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4136"/>
        <source>Range</source>
        <translation>Intervalo</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4144"/>
        <source>Mean</source>
        <translation>Média</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4152"/>
        <source>Sum of squares</source>
        <translation>Soma das raízes</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4160"/>
        <source>Standard Deviation</source>
        <translation>Desvio Padrão</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4168"/>
        <source>Sum of all cells</source>
        <translation>Soma de todas as células</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4176"/>
        <source>Cell Count</source>
        <translation>Número de Células</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3957"/>
        <source>Data Type:</source>
        <translation>Tipo de Dados:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3963"/>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte - Inteiro de 8 bits sem sinal</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3966"/>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 - Inteiro de 16 bits sem sinal</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3969"/>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 - Inteiro de 16 bits</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3972"/>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 - Inteiro de 32 bits sem sinal</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3975"/>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 - Inteiro de 32 bits</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3978"/>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 - Ponto flutuante de 32 bits</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3981"/>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 - Ponto flutuante de 64 bits</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3984"/>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 - Inteiro complexo de 16 bits</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3987"/>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 - Inteiro complexo de 32 bits</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3990"/>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 - Ponto flutuante complexo de 32 bits</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3993"/>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 - Ponto flutuante complexto de 64 bits</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3996"/>
        <source>Could not determine raster data type.</source>
        <translation>Não foi possível determinar o tipo de dados do raster.</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4261"/>
        <source>Average Magphase</source>
        <translation>Magphase Média</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4266"/>
        <source>Average</source>
        <translation>Média</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4022"/>
        <source>Layer Spatial Reference System: </source>
        <translation>Sistema de Referência Espacial da Camada: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4880"/>
        <source>out of extent</source>
        <translation>fora da extensão</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4917"/>
        <source>null (no data)</source>
        <translation>nulo (sem dado)</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3888"/>
        <source>Dataset Description</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3949"/>
        <source>No Data Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="281"/>
        <source>and all other files</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="651"/>
        <source>&lt;h3&gt;Multiband Image Notes&lt;/h3&gt;&lt;p&gt;This is a multiband image. You can choose to render it as grayscale or color (RGB). For color images, you can associate bands to colors arbitarily. For example, if you have a seven band landsat image, you may choose to render it as:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Visible Blue (0.45 to 0.52 microns) - not mapped&lt;/li&gt;&lt;li&gt;Visible Green (0.52 to 0.60 microns) - not mapped&lt;/li&gt;&lt;/li&gt;Visible Red (0.63 to 0.69 microns) - mapped to red in image&lt;/li&gt;&lt;li&gt;Near Infrared (0.76 to 0.90 microns) - mapped to green in image&lt;/li&gt;&lt;li&gt;Mid Infrared (1.55 to 1.75 microns) - not mapped&lt;/li&gt;&lt;li&gt;Thermal Infrared (10.4 to 12.5 microns) - not mapped&lt;/li&gt;&lt;li&gt;Mid Infrared (2.08 to 2.35 microns) - mapped to blue in image&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>&lt;h3&gt;Notas sobre imagens multibandas&lt;/h3&gt;&lt;p&gt; Esta é uma imagem multibanda. Pode-se escolher renderizá-la com escalas de cinza ou cores(RGB). Para imagens coloridas, pode-se associar bandas para cores arbitrárias. Por exemplo, tem-se uma imagem landsat de sete bandas, deve-se escolher renderizá-lo como:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Azul visível (0.45 a 0.52 mícrons) - não mapeado&lt;/li&gt;&lt;li&gt;Verde Visível (0.52 a 0.60 mícrons) - não mapeado&lt;/li&gt;&lt;/li&gt;Vermelho visível (0.63 a 0.69 mícrons) - não mapeado&lt;/li&gt;&lt;li&gt;Infravermelho Aproximado (0.76 a 0.90 mícrons) - mapeado para o verde na imagem&lt;/li&gt;&lt;li&gt;Semi-infravermelho (1.55 a 1.75 mícrons) - não mapeado&lt;/li&gt;&lt;li&gt;Infravermelho Térmico (10.4 a 12.5 mícrons) - não mapeado&lt;/li&gt;&lt;li&gt;Semi-infravermelho (2.08 a 2.35 mícrons) - mapeado para o azul na imagem&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="658"/>
        <source>&lt;h3&gt;Paletted Image Notes&lt;/h3&gt; &lt;p&gt;This image uses a fixed color palette. You can remap these colors in different combinations e.g.&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Red - blue in image&lt;/li&gt;&lt;li&gt;Green - blue in image&lt;/li&gt;&lt;li&gt;Blue - green in image&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>&lt;h3&gt;Notas sobre imagem com paleta&lt;/h3&gt; &lt;p&gt;Esta imagem usa uma paleta de cores fixa. Pode-se remapear essas cores em diferentes combinações ex.&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Vermelho - azul na imagem&lt;/li&gt;&lt;li&gt;Verde - azul na imagem&lt;/li&gt;&lt;li&gt;Azul - verde na imagem&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="665"/>
        <source>&lt;h3&gt;Grayscale Image Notes&lt;/h3&gt; &lt;p&gt;You can remap these grayscale colors to a pseudocolor image using an automatically generated color ramp.&lt;/p&gt;</source>
        <translation>&lt;h3&gt;Notas para imagem com Escalas de cinza&lt;/h3&gt; &lt;p&gt;Pode-se remapear essas Escalas de cinza para uma imagem com pseudocores usando uma rampa gerada de cores automátcas.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="688"/>
        <source>Grayscale</source>
        <translation>Escalas de cinza</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="678"/>
        <source>Pseudocolor</source>
        <translation>Pseudocores</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="682"/>
        <source>Freak Out</source>
        <translation>Barbarizar</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="116"/>
        <source>Palette</source>
        <translation>Paleta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="217"/>
        <source>Not Set</source>
        <translation type="unfinished">Não Ajustado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="571"/>
        <source>Columns: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="572"/>
        <source>Rows: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="573"/>
        <source>No-Data Value: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="573"/>
        <source>n/a</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="484"/>
        <source>Write access denied</source>
        <translation type="unfinished">Acesso a escrita negado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="485"/>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation type="unfinished">Acesso a escrita negado. Ajuste as permissões do arquivo e tente novamente.

</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="494"/>
        <source>Building pyramids failed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="490"/>
        <source>The file was not writeable. Some formats can not be written to, only read. You can also try to check the permissions and then try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="495"/>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="13"/>
        <source>Raster Layer Properties</source>
        <translation>Propriedades da camada Raster</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="393"/>
        <source>General</source>
        <translation>Geral</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="585"/>
        <source>Layer Source:</source>
        <translation>Origem da Camada:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="565"/>
        <source>Display Name:</source>
        <translation>Identificação:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="463"/>
        <source>Legend:</source>
        <translation>Legenda:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="617"/>
        <source>No Data:</source>
        <translation>Nenhum Dado:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="38"/>
        <source>Symbology</source>
        <translation>Simbologia</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="140"/>
        <source>Transparency:</source>
        <translation>Transparência:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="110"/>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Cheio&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="70"/>
        <source>None</source>
        <translation>Nenhum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="150"/>
        <source>Invert Color Map</source>
        <translation>Inverter Mapa de Cor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="90"/>
        <source>0%</source>
        <translation>0%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="313"/>
        <source>Band</source>
        <translation>Banda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="336"/>
        <source>&lt;b&gt;&lt;font color=&quot;#00ff00&quot;&gt;Green&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&quot;#00ff00&quot;&gt;Verde&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="346"/>
        <source>&lt;b&gt;&lt;font color=&quot;#ff0000&quot;&gt;Red&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&quot;#ff0000&quot;&gt;Vermelho&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="369"/>
        <source>&lt;b&gt;&lt;font color=&quot;#0000ff&quot;&gt;Blue&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&quot;#0000ff&quot;&gt;Azul&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="306"/>
        <source>Color</source>
        <translation>Cor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="275"/>
        <source>Gray</source>
        <translation>Cinza</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="259"/>
        <source>Std Deviations</source>
        <translation>Desvios padrões</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="249"/>
        <source>Color Map</source>
        <translation>Cor do Mapa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="727"/>
        <source>Metadata</source>
        <translation>Metadata</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="747"/>
        <source>Pyramids</source>
        <translation>Pirâmides</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="843"/>
        <source>Resampling Method</source>
        <translation>Método de Redimensionamento</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="854"/>
        <source>Average</source>
        <translation>Média</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="859"/>
        <source>Nearest Neighbour</source>
        <translation>Vizinho mais Próximo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="867"/>
        <source>Build Pyramids</source>
        <translation>Construir Pirâmides</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="812"/>
        <source>Pyramid Resolutions</source>
        <translation>Resoluções das Pirâmides</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="413"/>
        <source>Thumbnail</source>
        <translation>Pré-Visualização</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="603"/>
        <source>Columns:</source>
        <translation>Colunas:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="610"/>
        <source>Rows:</source>
        <translation>Linhas:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="513"/>
        <source>Palette:</source>
        <translation>Paleta:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="661"/>
        <source>Maximum 1:</source>
        <translation>Máximo 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="648"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Escala máxima em que a esta camada será mostrada. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="684"/>
        <source>Minimum 1:</source>
        <translation>Mínimo 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="671"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Escala mínima em que esta camada será mostrada. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="877"/>
        <source>Histogram</source>
        <translation>Histograma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="969"/>
        <source>Options</source>
        <translation>Opções</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="988"/>
        <source>Out Of Range OK?</source>
        <translation>Fora de Alcance OK?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="995"/>
        <source>Allow Approximation</source>
        <translation>Permitir Aproximação</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="916"/>
        <source>Chart Type</source>
        <translation>Tipo de Carta</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="938"/>
        <source>Bar Chart</source>
        <translation>Barra da Carta</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="928"/>
        <source>Line Graph</source>
        <translation>Gráfico de Linha</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1024"/>
        <source>Refresh</source>
        <translation>Atualizar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="50"/>
        <source>Display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="208"/>
        <source>Grayscale Image</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="288"/>
        <source>Color Image</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="626"/>
        <source>DebugInfo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="633"/>
        <source>Scale Dependent Visibility</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="697"/>
        <source>Spatial Reference System</source>
        <translation type="unfinished">Sistema de referência espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="709"/>
        <source>Change</source>
        <translation type="unfinished">Modificar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="981"/>
        <source>Column Count:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="379"/>
        <source>Transparent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="168"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot;font-size:10pt;font-family:Sans Serif&quot;&gt;
&lt;p style=&quot;margin-top:14px&quot; dir=&quot;ltr&quot;&gt;&lt;span style=&quot;font-weight:600&quot;&gt;Notes&lt;/span&gt;&lt;/p&gt;
&lt;/body&gt;&lt;/html&gt;
</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRunProcess</name>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="146"/>
        <source>Unable to run command</source>
        <translation>Impossível executar comando</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="59"/>
        <source>Starting</source>
        <translation>Iniciando</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="115"/>
        <source>Done</source>
        <translation>Feito</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPlugin</name>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="164"/>
        <source> metres/km</source>
        <translation> metros/km</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="281"/>
        <source> feet</source>
        <translation>pés</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="288"/>
        <source> degrees</source>
        <translation> graus</translation>
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
        <translation> pés</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="286"/>
        <source> degree</source>
        <translation> graus</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="291"/>
        <source> unknown</source>
        <translation> desconhecido</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Left</source>
        <translation>Superior Esquerdo</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="78"/>
        <source>Bottom Left</source>
        <translation>Inferior Esquerdo</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Right</source>
        <translation>Superior Direito</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Bottom Right</source>
        <translation>Inferior Direito</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="81"/>
        <source>Tick Down</source>
        <translation>Marca (tick) acima</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Tick Up</source>
        <translation>Marca (tick) abaixo</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Bar</source>
        <translation>Barra</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Box</source>
        <translation>Caixa</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="102"/>
        <source>&amp;Scale Bar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="103"/>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="543"/>
        <source>&amp;Decorations</source>
        <translation type="unfinished">&amp;Decorações</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="165"/>
        <source> feet/miles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="262"/>
        <source> miles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="267"/>
        <source> mile</source>
        <translation type="unfinished"> milha</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="272"/>
        <source> inches</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="300"/>
        <source>Scale Bar Plugin</source>
        <translation>Plugin de Barra de Escala</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="143"/>
        <source>Top Left</source>
        <translation>Superior Esquerdo</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="148"/>
        <source>Top Right</source>
        <translation>Superior Direito</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation>Inferior Esquerdo</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="158"/>
        <source>Bottom Right</source>
        <translation>Inferior Direito</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="94"/>
        <source>Size of bar:</source>
        <translation>Tamanho da barra:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="254"/>
        <source>Placement:</source>
        <translation>Posicionamento:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="211"/>
        <source>Tick Down</source>
        <translation>Marca (tick) abaixo</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="216"/>
        <source>Tick Up</source>
        <translation>Marca (tick) acima</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="221"/>
        <source>Box</source>
        <translation>Caixa</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="226"/>
        <source>Bar</source>
        <translation>Barra</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="207"/>
        <source>Select the style of the scale bar</source>
        <translation>Selecione o estilo da barra de escala</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="127"/>
        <source>Colour of bar:</source>
        <translation>Cor da barra:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="192"/>
        <source>Scale bar style:</source>
        <translation>Estilo da barra de escala:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="174"/>
        <source>Enable scale bar</source>
        <translation>Habilitar barra de escala</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="109"/>
        <source>Automatically snap to round number on resize</source>
        <translation>Arredondar números automaticamente ao redimensionar</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="76"/>
        <source>Click to select the colour</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="274"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="128"/>
        <source>No matching features found.</source>
        <translation type="unfinished">Nehuma característica produrada encontrada.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="129"/>
        <source>Search results</source>
        <translation type="unfinished">Pesquisar resultados</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="138"/>
        <source>Search string parsing error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="194"/>
        <source>No Records</source>
        <translation type="unfinished">Nenhum registro encontrado</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="194"/>
        <source>The query you specified results in zero records being returned.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="35"/>
        <source>Search query builder</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="126"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="171"/>
        <source>Are you sure you want to remove the </source>
        <translation type="unfinished">Tem certeza que deseja remover o </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="171"/>
        <source> connection and all associated settings?</source>
        <translation type="unfinished">conexão e todos os ajustes associados ?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="172"/>
        <source>Confirm Delete</source>
        <translation type="unfinished">Confirme a exclusão</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="454"/>
        <source>WMS Provider</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="456"/>
        <source>Could not open the WMS Provider</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="465"/>
        <source>Select Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="465"/>
        <source>You must select at least one layer first.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="767"/>
        <source>Could not understand the response.  The</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="768"/>
        <source>provider said</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="822"/>
        <source>WMS proxies</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="822"/>
        <source>&lt;p&gt;Several WMS servers have been added to the server list. Note that the proxy fields have been left blank and if you access the internet via a web proxy, you will need to individually set the proxy fields with appropriate values.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="469"/>
        <source>Coordinate Reference System</source>
        <translation type="unfinished">Sistema de referência de coordenadas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="469"/>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsserversourceselect.cpp" line="588"/>
        <source>Coordinate Reference System (%1 available)</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelectBase</name>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="13"/>
        <source>Add Layer(s) from a Server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="315"/>
        <source>C&amp;lose</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="318"/>
        <source>Alt+L</source>
        <translation type="unfinished">Alt+L</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="302"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="305"/>
        <source>F1</source>
        <translation type="unfinished">F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="276"/>
        <source>Image encoding</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="203"/>
        <source>Layers</source>
        <translation type="unfinished">Camadas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="236"/>
        <source>ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="241"/>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="246"/>
        <source>Title</source>
        <translation type="unfinished">Título</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="251"/>
        <source>Abstract</source>
        <translation type="unfinished">Resumo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="187"/>
        <source>&amp;Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="190"/>
        <source>Alt+A</source>
        <translation type="unfinished">Alt+A</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="34"/>
        <source>Server Connections</source>
        <translation type="unfinished">Conexões do servidor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="108"/>
        <source>&amp;New</source>
        <translation type="unfinished">&amp;Novo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="101"/>
        <source>Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="91"/>
        <source>Edit</source>
        <translation type="unfinished">Editar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="81"/>
        <source>C&amp;onnect</source>
        <translation type="unfinished">C&amp;onectar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="174"/>
        <source>Ready</source>
        <translation type="unfinished">Pronto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="118"/>
        <source>Coordinate Reference System</source>
        <translation type="unfinished">Sistema de referência de coordenadas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="156"/>
        <source>Change ...</source>
        <translation type="unfinished">Mudar ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="46"/>
        <source>Adds a few example WMS servers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="52"/>
        <source>Add default servers</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="435"/>
        <source>The database gave an error while executing this SQL:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="443"/>
        <source>The error was:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="440"/>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="21"/>
        <source>Single Symbol</source>
        <translation type="unfinished">Símbolo simples</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="168"/>
        <source>Fill Patterns:</source>
        <translation type="unfinished">Padrões de Preenchimento:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="69"/>
        <source>Point</source>
        <translation type="unfinished">Ponto</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="121"/>
        <source>Size</source>
        <translation type="unfinished">Tamanho</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="155"/>
        <source>Symbol</source>
        <translation type="unfinished">Símbolo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="898"/>
        <source>Outline Width:</source>
        <translation type="unfinished">Espessura da borda:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="950"/>
        <source>Fill Color:</source>
        <translation type="unfinished">Cor de Preenchimento:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="960"/>
        <source>Outline color:</source>
        <translation type="unfinished">Cor da Borda:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="684"/>
        <source>Outline Style:</source>
        <translation type="unfinished">Estilo da Borda:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="47"/>
        <source>Label:</source>
        <translation type="unfinished">Rótulo:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="544"/>
        <source>No Fill</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="641"/>
        <source>Browse:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="145"/>
        <source>Are you sure you want to remove the [</source>
        <translation>Você tem certeza que quer remover a [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="145"/>
        <source>] connection and all associated settings?</source>
        <translation>] conexão e todos os ajustes associados?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="146"/>
        <source>Confirm Delete</source>
        <translation>Confirme a exclusão</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="866"/>
        <source> - Edit Column Names</source>
        <translation> - Editar Nomes das Colunas</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="308"/>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>O seguinte(s) Shapefile(s) não foi (foram) carregado(s):</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="312"/>
        <source>REASON: File cannot be opened</source>
        <translation>MOTIVO: O arquivo não pode ser aberto</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="317"/>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>RAZÃO: Um ou ambos arquivos do Shapefile (*.dbf, *.shx) não foram encontrados</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="394"/>
        <source>General Interface Help:</source>
        <translation>Interface Geral de Ajuda:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="396"/>
        <source>PostgreSQL Connections:</source>
        <translation>Conexões PostgreSQL:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="398"/>
        <source>[New ...] - create a new connection</source>
        <translation>[Novo...] - criar uma nova conexão</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="399"/>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation>[Editar] - editar a conexão atualmente selecionada</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="400"/>
        <source>[Remove] - remove the currently selected connection</source>
        <translation>[Remover] - Remove a conexão atualmente selecionada</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="401"/>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation>-você precisa selecionar a conexão que funciona (conecta corretamente) para conseguir importar arquivos</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="402"/>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation>-quando mudar conexões o \&quot;Global Schema\&quot; também mudará de acordo</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="403"/>
        <source>Shapefile List:</source>
        <translation>Lista de Shapefiles:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="405"/>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation>[Adicionar...] - abrir uma caixa de diálogo e selecionar o(s) arquivo(s) para importar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="406"/>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation>[Remover] - remove os arquivos atualmente selecionados da lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="407"/>
        <source>[Remove All] - remove all the files in the list</source>
        <translation>[Remove Todos] - remove todos os arquivos da lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="408"/>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] - Referência ID para os shapefiles à serem importados</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="409"/>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation>[Usar Padrão (SRID)] - setar SRID para -1</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="410"/>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation>[Nome da Coluna Geometria] - nome da coluna geometria no banco de dados</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="411"/>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation>[Usar Padrão (Nome da Coluna Geometria)] - seta o nome para &apos;the_geom&apos;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="412"/>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation>[Glogal Schema] - seta o esquema para todos os arquivos à serem importados em</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="414"/>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[Importar] - Importa os shapefiles atualmente na lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="415"/>
        <source>[Quit] - quit the program
</source>
        <translation>[Sair] - sai do programa</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="416"/>
        <source>[Help] - display this help dialog</source>
        <translation>[Ajuda] - mostra essa caixa de ajuda</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="842"/>
        <source>Import Shapefiles</source>
        <translation>Omportar Shapefiles</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="430"/>
        <source>You need to specify a Connection first</source>
        <translation>Você precisa especificar uma Conexão primeiro</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="446"/>
        <source>Connection failed - Check settings and try again</source>
        <translation>A conexão falhou - Cheque a sua configuração e tente novamente</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="550"/>
        <source>You need to add shapefiles to the list first</source>
        <translation>Você precisa adicionar shapefiles para a lista primeiro</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="611"/>
        <source>Importing files</source>
        <translation>Importando arquivos</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="555"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="559"/>
        <source>Progress</source>
        <translation>Progresso</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="566"/>
        <source>Problem inserting features from file:</source>
        <translation>Problema inserindo feições do arquivo:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="573"/>
        <source>Invalid table name.</source>
        <translation>Nome de tabela inválido.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="583"/>
        <source>No fields detected.</source>
        <translation>Nenhum campo detectado.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="604"/>
        <source>The following fields are duplicates:</source>
        <translation>Os seguintes campos estão duplicados:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="704"/>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>Importar Shapefiles - Existe Relação</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="705"/>
        <source>The Shapefile:</source>
        <translation>O Shapefile:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="706"/>
        <source>will use [</source>
        <translation>irá usar [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="706"/>
        <source>] relation for its data,</source>
        <translation>] relação com estes dados,</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="706"/>
        <source>which already exists and possibly contains data.</source>
        <translation>e que já existem e possivelmente contém dados.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="707"/>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>Para evitar a perda de dados mude o &quot;DB Relation Name&quot;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="707"/>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation>para este Shapefile na lista da caixa de diálogo principal.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="708"/>
        <source>Do you want to overwrite the [</source>
        <translation>Você quer salvar sobre a relação do arquivo [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="708"/>
        <source>] relation?</source>
        <translation>] ?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="869"/>
        <source>Use the table below to edit column names. Make sure that none of the columns are named using a PostgreSQL reserved word</source>
        <translation>Use a tabela abaixo para editar o nome das colunas. Tenha certeza que nenhuma das colunas tenha um nome que contenha uma palavra reservada do PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>File Name</source>
        <translation type="unfinished">Nome do Arquivo</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>Feature Class</source>
        <translation type="unfinished">Classe da Feição</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Features</source>
        <translation type="unfinished">Feições</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>DB Relation Name</source>
        <translation type="unfinished">Nome Relacional DB</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Schema</source>
        <translation type="unfinished">Esquema</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="120"/>
        <source>New Connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="170"/>
        <source>Add Shapefiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="172"/>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="477"/>
        <source>PostGIS not available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="479"/>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="596"/>
        <source>Checking to see if </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="844"/>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="845"/>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="19"/>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT - Ferramenta de importação Shapefile para PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="360"/>
        <source>PostgreSQL Connections</source>
        <translation>Conexões PostgreSQL </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="399"/>
        <source>Remove</source>
        <translation>Remover</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="63"/>
        <source>Shapefile List</source>
        <translation>Lista Shapefile</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="269"/>
        <source>Geometry Column Name</source>
        <translation>Nome da Coluna Geométrica</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="187"/>
        <source>SRID</source>
        <translation>SRID</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="125"/>
        <source>Remove All</source>
        <translation>Remover Tudo</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="330"/>
        <source>Global Schema</source>
        <translation>Esquema global</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="53"/>
        <source>Shapefile to PostGIS Import Tool</source>
        <translation>Ferramenta de importação de Shapefile para PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="99"/>
        <source>Add</source>
        <translation>Adicionar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="96"/>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Adicionar um shapefile para a lista de arquivos a serem importados</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="109"/>
        <source>Remove the selected shapefile from the import list</source>
        <translation>Remover o shapefile selecionado da lista de importação</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="122"/>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Remover todos os shapefiles da lista de importação</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="215"/>
        <source>Use Default SRID</source>
        <translation>Utilizar SRID Padrão</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="212"/>
        <source>Set the SRID to the default value</source>
        <translation>Definir o SRID para valor padrão</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="254"/>
        <source>Use Default Geometry Column Name</source>
        <translation>Utilizar o Nome da Coluna de Geometria Padrão</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="251"/>
        <source>Set the geometry column name to the default value</source>
        <translation>Definir o nome da coluna de geometria como valor padrão</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="386"/>
        <source>New</source>
        <translation>Novo</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="383"/>
        <source>Create a new PostGIS connection</source>
        <translation>Criar uma nova conexão PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="396"/>
        <source>Remove the current PostGIS connection</source>
        <translation>Remover a atual conexão PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="409"/>
        <source>Connect</source>
        <translation>Conectar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="422"/>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="419"/>
        <source>Edit the current PostGIS connection</source>
        <translation>Editar a conexão PostGIS atual</translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="68"/>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>&amp;Importa arquivos SHP para PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="70"/>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>Importa arquivos SHP para uma base de dados PostGis-enabled PostgreSQL. O esquema e os nomes de campo podem ser personalizados na importação</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="93"/>
        <source>&amp;Spit</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialogBase</name>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="13"/>
        <source>Form1</source>
        <translation type="unfinished">Form1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="37"/>
        <source>Classification Field:</source>
        <translation>Campo de classificação:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="47"/>
        <source>Delete class</source>
        <translation>Excluir classe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="80"/>
        <source>Classify</source>
        <translation>Classifica</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2032"/>
        <source>Could not commit the added features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2123"/>
        <source>No other types of changes will be committed at this time.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2054"/>
        <source>Could not commit the changed attributes.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2113"/>
        <source>However, the added features were committed OK.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2080"/>
        <source>Could not commit the changed geometries.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2117"/>
        <source>However, the changed attributes were committed OK.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2110"/>
        <source>Could not commit the deleted features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2121"/>
        <source>However, the changed geometries were committed OK.</source>
        <translation>Entretanto, as mgeometrias modificadas foram entregues OK.</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="101"/>
        <source>Transparency: </source>
        <translation>Transparência: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="188"/>
        <source>Single Symbol</source>
        <translation>Símbolo simples</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="191"/>
        <source>Graduated Symbol</source>
        <translation type="unfinished">Símbolo Graduado</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="192"/>
        <source>Continuous Color</source>
        <translation type="unfinished">Cor Contínua</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="193"/>
        <source>Unique Value</source>
        <translation type="unfinished">Valor absoluto</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="150"/>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation type="unfinished">Este botão abre a ferramenta de consulta do PostgreSQL e permite que você filtre os seus dados e crie um novo conjunto para visualizá-los no mapa, evitando assim a visualização de todas as informações</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="151"/>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation type="unfinished">A consulta usada para limitar as feições é mostrada aqui. Isto é atualmente suportado apenas nas camadas do PostgreSQL. Pressione Enter ou modifique a consulta clicando no botão Ferramenta de Consulta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="371"/>
        <source>Spatial Index</source>
        <translation>Índice espacial</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="366"/>
        <source>Creation of spatial index successfull</source>
        <translation type="unfinished">Índice Espacial criado com sucesso</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="371"/>
        <source>Creation of spatial index failed</source>
        <translation type="unfinished">A criação do Índice Espacial falhou</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="384"/>
        <source>General:</source>
        <translation type="unfinished">Geral:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="399"/>
        <source>Storage type of this layer : </source>
        <translation>Tipode armazenamento desta camada : </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="405"/>
        <source>Source for this layer : </source>
        <translation>Fonte ṕara esta camada: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="422"/>
        <source>Geometry type of the features in this layer : </source>
        <translation type="unfinished">Tipo de geometria de feições nesta camada:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="430"/>
        <source>The number of features in this layer : </source>
        <translation type="unfinished">Número de feições nesta camada: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="435"/>
        <source>Editing capabilities of this layer : </source>
        <translation>Editar capacidades para esta camada: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="442"/>
        <source>Extents:</source>
        <translation type="unfinished">Estender:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="447"/>
        <source>In layer spatial reference system units : </source>
        <translation type="unfinished">Sistema de unidades de referência espacial na camada</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="448"/>
        <source>xMin,yMin </source>
        <translation type="unfinished">xMin,yMin </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="452"/>
        <source> : xMax,yMax </source>
        <translation type="unfinished"> : xMax,yMax </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="507"/>
        <source>In project spatial reference system units : </source>
        <translation type="unfinished">Sistema de unidades de referência espacial no projeto</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="481"/>
        <source>Layer Spatial Reference System:</source>
        <translation type="unfinished">Sistema de referência espacial na camada:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="518"/>
        <source>Attribute field info:</source>
        <translation type="unfinished">Informação do campo atributo:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="525"/>
        <source>Field</source>
        <translation type="unfinished">Campo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="528"/>
        <source>Type</source>
        <translation type="unfinished">Tipo</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="531"/>
        <source>Length</source>
        <translation type="unfinished">Tamanho</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="534"/>
        <source>Precision</source>
        <translation type="unfinished">Precisão</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="392"/>
        <source>Layer comment: </source>
        <translation>Comentário da camada: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="537"/>
        <source>Comment</source>
        <translation>Comentário</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="19"/>
        <source>Layer Properties</source>
        <translation>Propriedades da Camada</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="102"/>
        <source>Legend type:</source>
        <translation>Tipo de legenda:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="54"/>
        <source>Symbology</source>
        <translation type="unfinished">Simbologia</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="112"/>
        <source>Transparency:</source>
        <translation type="unfinished">Transparência:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="136"/>
        <source>General</source>
        <translation type="unfinished">Geral</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="184"/>
        <source>Use scale dependent rendering</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="199"/>
        <source>Maximum 1:</source>
        <translation type="unfinished">Máximo 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="209"/>
        <source>Minimum 1:</source>
        <translation type="unfinished">Mínimo 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="219"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Escala mínima em que essa camada pode ser exibida. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="232"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Escala máxima em que essa camada pode ser exibida. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="148"/>
        <source>Display name</source>
        <translation type="unfinished">Identificação</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="174"/>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation type="unfinished">Use este controle para selecionar qual campo é colocado no nível mais alto na caixa de diálogo Identifique Resultados</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="296"/>
        <source>Spatial Reference System</source>
        <translation type="unfinished">Sistema de referência espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="315"/>
        <source>Change</source>
        <translation type="unfinished">Modificar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="158"/>
        <source>Display field for the Identify Results dialog box</source>
        <translation type="unfinished">Mostra o campo na caixa de diálogo Identifique Resultados</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="161"/>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation type="unfinished">Seleciona o campo a ser mostrado na caixa de diálogo Identifique Resultados</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="164"/>
        <source>Display field</source>
        <translation type="unfinished">Exibir campo</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="325"/>
        <source>Subset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="366"/>
        <source>Query Builder</source>
        <translation type="unfinished">Ferramenta de Consulta</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="248"/>
        <source>Spatial Index</source>
        <translation>Índice Espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="260"/>
        <source>Create Spatial Index</source>
        <translation type="unfinished">Criar índice espacial</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="270"/>
        <source>Create</source>
        <translation type="unfinished">Criar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="377"/>
        <source>Metadata</source>
        <translation>Metadados</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="397"/>
        <source>Labels</source>
        <translation type="unfinished">Labels</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="435"/>
        <source>Display labels</source>
        <translation type="unfinished">Mostrar rótulos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="443"/>
        <source>Actions</source>
        <translation type="unfinished">Ações</translation>
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
        <translation>Rótulo</translation>
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
        <translation>Classes de Símbolos:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="77"/>
        <source>Count:</source>
        <translation>Contagem:</translation>
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
        <translation>&amp;Adiciona camada WFS</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1390"/>
        <source>unknown</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1396"/>
        <source>received %1 bytes from %2</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="259"/>
        <source>Are you sure you want to remove the </source>
        <translation type="unfinished">Tem certeza que deseja remover o </translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="259"/>
        <source> connection and all associated settings?</source>
        <translation type="unfinished">conexão e todos os ajustes associados ?</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="260"/>
        <source>Confirm Delete</source>
        <translation type="unfinished">Confirme a exclusão</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="29"/>
        <source>Title</source>
        <translation type="unfinished">Título</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="34"/>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="39"/>
        <source>Abstract</source>
        <translation>Resumo</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="47"/>
        <source>Coordinate Reference System</source>
        <translation>Sistema de referência de coordenadas</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="85"/>
        <source>Change ...</source>
        <translation>Mudar ...</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="95"/>
        <source>Server Connections</source>
        <translation>Conexões do servidor</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="107"/>
        <source>&amp;New</source>
        <translation type="unfinished">&amp;Novo</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="117"/>
        <source>Delete</source>
        <translation>Excluir</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="127"/>
        <source>Edit</source>
        <translation type="unfinished">Editar</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="153"/>
        <source>C&amp;onnect</source>
        <translation>C&amp;onectar</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>Note: this plugin not considered stable yet. Use it on your own risk</source>
        <translation type="obsolete">Nota: este plugin ainda é instável. Use por sua conta e risco.</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="13"/>
        <source>Add WFS Layer from a Server</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="718"/>
        <source>Tried URL: </source>
        <translation>URL tentada: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="698"/>
        <source>HTTP Exception</source>
        <translation>Exceção HTTP</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="661"/>
        <source>WMS Service Exception</source>
        <translation>Exceção de serviço WMS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1512"/>
        <source>DOM Exception</source>
        <translation>Exceção DOM</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="772"/>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>Impossível obter capacidades WMS:  %1 na linha %2 coluna %3</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="803"/>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>Isto é provavelmente devido a uma URL incorreta do servidor WMS.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="799"/>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>Impossível obter capacidades WMS neste formato (DTD): no %1 ou %2 encontrado</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1514"/>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>Impossível obter oo serviço WMS. Exceção em %1: %2 na linha %3 coluna %4</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1564"/>
        <source>Request contains a Format not offered by the server.</source>
        <translation>Requisição contém um formato não oferecido pelo servidor</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1568"/>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Requisição contém um CRS não oferecido pelo servidor para uma ou mais camadas.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1572"/>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Requisição contém um SRS não oferecido pelo servidor para uma ou mais camadas.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1577"/>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation>A requisição GetMap é para uma camada não oferecida pelo servidor, ou a requisição GetFeatureInfo é para uma camada não mostrada no mapa.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1581"/>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>Requisição é para uma camada em um estilo não oferecido pelo servidor.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1585"/>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>GetFeatureInfo pedido é aplicado a uma camada declarada como não pesquisável.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1589"/>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>GetFeatureInfo pedido contém um valor inválido de X e Y </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1594"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation>O valor do parâmetro (opcional) em GetCapabilities é igual ao atual valor de serviço.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1599"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation>O valor do parâmetro (opcional) em GetCapabilities deve ser maior do que o atual valor de serviço.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1604"/>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation>Requisição não inclui um valor de dimensão de amostra, o servidor não pode atribuir um valor padrão para esta dimensão.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1608"/>
        <source>Request contains an invalid sample dimension value.</source>
        <translation>Requisição contém um valor inválido de dimensão de amostra. </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1612"/>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>Requisição é para uma operação opcional que não é suportada pelo servidor.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1616"/>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(Código de erro desconhecido pela postagem - 1.3 WMS server)</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1619"/>
        <source>The WMS vendor also reported: </source>
        <translation>O vendedor WMS também reportou: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1622"/>
        <source>This is probably due to a bug in the QGIS program.  Please report this error.</source>
        <translation>Isto se deve provavelmente a um &apos;bug&apos; do QGIS. Por favor reporte este erro.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1806"/>
        <source>Server Properties:</source>
        <translation>Propriedades do servidor:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1937"/>
        <source>Property</source>
        <translation type="unfinished">Propriedade</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1940"/>
        <source>Value</source>
        <translation type="unfinished">Valor</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1823"/>
        <source>WMS Version</source>
        <translation>Versão WMS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2069"/>
        <source>Title</source>
        <translation type="unfinished">Título</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2077"/>
        <source>Abstract</source>
        <translation>Resumo</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1847"/>
        <source>Keywords</source>
        <translation>Palavras-chave</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1855"/>
        <source>Online Resource</source>
        <translation>Recurso on-line</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1863"/>
        <source>Contact Person</source>
        <translation>Contato pessoal</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1875"/>
        <source>Fees</source>
        <translation>Taxa</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1883"/>
        <source>Access Constraints</source>
        <translation>Acesso reservado</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1891"/>
        <source>Image Formats</source>
        <translation>Formatos de imagem</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1899"/>
        <source>Identify Formats</source>
        <translation>Identifica fomatos</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1907"/>
        <source>Layer Count</source>
        <translation>Contar camadas</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1927"/>
        <source>Layer Properties: </source>
        <translation>Propriedades da camada: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1945"/>
        <source>Selected</source>
        <translation>Selecionado</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2002"/>
        <source>Yes</source>
        <translation type="unfinished">Sim</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2002"/>
        <source>No</source>
        <translation type="unfinished">Não</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1954"/>
        <source>Visibility</source>
        <translation type="unfinished">Visibilidade</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1960"/>
        <source>Visible</source>
        <translation>Visível</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1961"/>
        <source>Hidden</source>
        <translation>Oculto</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1962"/>
        <source>n/a</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1983"/>
        <source>Can Identify</source>
        <translation>Pode Identificar</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1991"/>
        <source>Can be Transparent</source>
        <translation>Pode ser transparente</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1999"/>
        <source>Can Zoom In</source>
        <translation>Pode aproximar</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2007"/>
        <source>Cascade Count</source>
        <translation>Contador cascata</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2015"/>
        <source>Fixed Width</source>
        <translation>Largura fixada</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2023"/>
        <source>Fixed Height</source>
        <translation>Altura fixada</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2031"/>
        <source>WGS 84 Bounding Box</source>
        <translation>Caixa de contorno WGS 84</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2041"/>
        <source>Available in CRS</source>
        <translation>Disponível em CRS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2052"/>
        <source>Available in style</source>
        <translation>Disponível no estilo</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2061"/>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2162"/>
        <source>Layer cannot be queried.</source>
        <translation>Camada não pode ser consultada.</translation>
    </message>
</context>
<context>
    <name>[pluginname]Gui</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished">Modelo de Plugin para o QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.ui" line="47"/>
        <source>Plugin Template</source>
        <translation type="unfinished">Modelo do Plugin</translation>
    </message>
    <message>
        <location filename="" line="7471221"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans Condensed&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Welcome to your automatically generated plugin!&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Documentation:&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;You really need to read the QGIS API Documentation now at:&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; color:#0000ff;&quot;&gt;http://svn.qgis.org/api_doc/html/&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;In particular look at the following classes:&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;QGisInterface&lt;/span&gt;      : http://svn.qgis.org/api_doc/html/classQgisInterface.html&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;QgsMapCanvas &lt;/span&gt; : http://svn.qgis.org/api_doc/html/classQgsMapCanvas.html&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;QgsMapTool&lt;/span&gt;         : http://svn.qgis.org/api_doc/html/classQgsMapTool.html&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;QgsPlugin&lt;/span&gt;              : http://svn.qgis.org/api_doc/html/classQgisPlugin.html&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;QGisInterface is an abstract base class (ABC) that specifies what publicly available features of QGIS are exposed to third party code and plugins. An instance of the QgisInterface is passed to the plugin when it loads. Please consult the QGIS development team if there is functionality required in the QGisInterface that is not available.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;What are all the files in my generated plugin directory for?&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;CMakeLists.txt&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename].h&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename].cpp  &lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename]gui.ui&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600; color:#000000;&quot;&gt;&lt;span style=&quot; font-weight:400;&quot;&gt;This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; color:#000000;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename]gui.cpp  &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename]gui.h &lt;span style=&quot; font-weight:400; color:#000000;&quot;&gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename].qrc  &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600; color:#dc143c;&quot;&gt;&lt;span style=&quot; font-weight:400; color:#000000;&quot;&gt;This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&quot;:/[pluginname]/&quot;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename].png  &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600; color:#dc143c;&quot;&gt;&lt;span style=&quot; font-weight:400; color:#000000;&quot;&gt;This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600; color:#dc143c;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;README&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;This file contains the documentation you are reading now!&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Getting developer help:&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt; * the QGIS developers mailing list, or&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt; * IRC (#qgis on freenode.net)&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;Have fun and thank you for choosing QGIS.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;The QGIS Team&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600;&quot;&gt;2007&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans Condensed&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Bem-vindo ao seu gerador automático de plugin!&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;Este á apenas o ponto de início. Agora você precisa modificar o código para tornar usável....leia mais para ter mais informação de como começar.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Documentação:&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;Você precisa realmente ler a documentação API do QGIS agora em:&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; color:#0000ff;&quot;&gt;http://svn.qgis.org/api_doc/html/&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;Dê uma olhada mais detalhada nas classes que seguem:&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;QGisInterface&lt;/span&gt;      : http://svn.qgis.org/api_doc/html/classQgisInterface.html&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;QgsMapCanvas &lt;/span&gt; : http://svn.qgis.org/api_doc/html/classQgsMapCanvas.html&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;QgsMapTool&lt;/span&gt;         : http://svn.qgis.org/api_doc/html/classQgsMapTool.html&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;QgsPlugin&lt;/span&gt;              : http://svn.qgis.org/api_doc/html/classQgisPlugin.html&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;QGisInterface é um resumo da base de classes (RBC) que especifica quais as feições públicas do QGIS estão expostas a códigos de plugins de terceiros. Uma instância da QgisInterface é passada ao plugin quando este é carregado. Por favor consulte o time de desenvolvimento do QGIS se a QGisinterface requerida não está disponível.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;What are all the files in my generated plugin directory for?&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;CMakeLists.txt&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename].h&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename].cpp  &lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename]gui.ui&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600; color:#000000;&quot;&gt;&lt;span style=&quot; font-weight:400;&quot;&gt;This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; color:#000000;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename]gui.cpp  &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename]gui.h &lt;span style=&quot; font-weight:400; color:#000000;&quot;&gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename].qrc  &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600; color:#dc143c;&quot;&gt;&lt;span style=&quot; font-weight:400; color:#000000;&quot;&gt;This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&quot;:/[pluginname]/&quot;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;[pluginlcasename].png  &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600; color:#dc143c;&quot;&gt;&lt;span style=&quot; font-weight:400; color:#000000;&quot;&gt;This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600; color:#dc143c;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600; color:#dc143c;&quot;&gt;README&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;This file contains the documentation you are reading now!&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Getting developer help:&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt; * the QGIS developers mailing list, or&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt; * IRC (#qgis on freenode.net)&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;Have fun and thank you for choosing QGIS.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;The QGIS Team&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600;&quot;&gt;2007&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="75"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation>Troque isso por uma breve descrição do que o plugin faz</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="73"/>
        <source>[menuitemname]</source>
        <translation type="unfinished">{nomedoitemdomenu]</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="80"/>
        <source>&amp;[menuname]</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
