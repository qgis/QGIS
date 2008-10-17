<!DOCTYPE TS><TS>
<defaultcodec></defaultcodec>
<context>
    <name>CoordinateCapture</name>
    <message>
        <source>Coordinate Capture</source>
        <translation>Captura de Coordenadas</translation>
    </message>
    <message>
        <source>Click on the map to view coordinates and capture to clipboard.</source>
        <translation>Clique no mapa para ver as coordenadas e capturar para a área de transferência.</translation>
    </message>
    <message>
        <source>&amp;Coordinate Capture</source>
        <translation>&amp;Captura de Coordenadas</translation>
    </message>
    <message>
        <source>Click to select the CRS to use for coordinate display</source>
        <translation>Clique para selecionar o CRS e usá-lo para mostrar as coordenadas</translation>
    </message>
    <message>
        <source>Coordinate in your selected CRS</source>
        <translation>Coordenadas no seu CRS selecionado</translation>
    </message>
    <message>
        <source>Coordinate in map canvas coordinate reference system</source>
        <translation>Coordenada no sistema de coordenadas do mapa</translation>
    </message>
    <message>
        <source>Copy to clipboard</source>
        <translation>Copiar para a área de tranferência</translation>
    </message>
    <message>
        <source>Click to enable mouse tracking. Click the canvas to stop</source>
        <translation>Clique para habilitar o rastreamento do mouse. Clique na barra de ferramentas para desabilitar.</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Bem-vindo ao seu plugin automaticamente criado!</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>Isso é apenas um ponto de partida. Você agora precisa modificar o código para fazé-lo útil... leia adiante para mais informações para introduzí-lo no assunto.</translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation>Documentação:</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>Você realmente precisa ler a documentação do QGIS API agora em:</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation>Em especial olhe as seguintes classes:</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin is um ABC que define o comportamento exigido que o seu plugin deve prover. Veja abaixo para mais detalhes.</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>Para que servem todos os arquivos gerados no meu diretório do plugin?</translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>Este é o arquivo criado pelo CMake que gera o plugin. Você deveria adicionar as dependências específicas e os arquivos fontes para esse arquivo.</translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>Esta é que classe que fornece o \&quot;elo\&quot; entre sua aplicação personalizada e o QGIS. Você verá que o número de métodos já implementados para você - incluíndo alguns exemplos como adicionar dados raster ou camadas vetoriais para a área da visão do mapa. Essa classe é uma instância concreta da interface do QgisPlugin, que define o comportamento requerido para um plugin. Em particular, um plugin têm um número estático de métodos e membros de forma que, o gerenciador do QgsPlugin e a lógica do carregador de plugins pode identificar cada plugin, criar uma entrada no menu apropriada para ele e etc. Note que não há nada lhe impedindo de criar múltiplas barras de ferramentas ou entradas de menu para somente um plugin. Por padrão uma única entrada no menu e um botão na barra de ferramentas são criados, e ele é pré-configurado na classe \&quot;run() method\&quot; quando selecionado. Esta implementação padrão provida à para você pelo construtor de plugins é bem documentada, então por favor refirá-se ao código para mais conselhos.</translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>Este é o arquivo para a interface com o usuário do Qt designer. Ele define o visual padrão dos diálogos do plugin sem implementar nenhuma lógica da aplicacão. Você pode modificar este formulário para atender suas necessidades, ou remové-lo completamente se o seu plugin não precisa mostrar algo ao usuário.  (p.ex. para mapas personalizados).</translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>Esta é a classe concreta mencionada no diálogo anterior onde a lógica dos aplicativos deve entrar. O mundo é seu.... </translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>Este é o arquivo de recursos do Qt4 para o seu plugin. O Makefile gerado para o seu plugin é todo configurado para compilar o arquivo de recursos, então tudo que você precisa fazer é adicionar os seus ícones adicionais, e etc, usando um simples arquivo em formato xml. Preste atencão no formato de nome usado para todos os seus recursos (p. ex., \&quot;/Ferreira/\&quot;). Para ele é importante usar um prefixo igual para todos os seus recursos. Nós sugerimos à você incluir outras imagens e dados de compilacão à esse arquivo de recursos também.</translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>Este é o ícone que vai ser usado para o seu plugin no menu e na barra de ferramentas. Simplesmente troque este ícone com o seu próprio ícone para fazer o seu plugin distinto do resto.</translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation>Este arquivo contém a documentacão que você está lendo agora!</translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation>Obter ajuda dos desenvolvedores:</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>Para questões e comentários a respeito do exemplo usado para construir um plugin, e/ou a criacão de funcões no QGIS usando a interface de geracão de plugins, por favor nos contate via:</translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; a lista de email do desenvolvedores do QGIS , ou &lt;/li&gt;&lt;li&gt; IRC (#qgis em freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS é distribuído segundo a Licenca Pública GNU. Se você criar um plugin que pode ser útil à outras pessoas, por favor considere compartilhá-lo com a comunidade.</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Se divirta e obrigado por esclher o QGIS.</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished">Modelo de Plugin para o QGIS</translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation type="unfinished">Modelo do Plugin</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <source>QGIS Plugin Installer</source>
        <translation type="obsolete">Instalador de Plugins do QGIS</translation>
    </message>
    <message>
        <source>Select repository, retrieve the list of available plugins, select one and install it</source>
        <translation type="obsolete">Selecione um repositório, obtenha a lista de plugins disponíveis, selecione eles e instale.</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation type="obsolete">Repositório</translation>
    </message>
    <message>
        <source>Active repository:</source>
        <translation type="obsolete">Repositório ativo:</translation>
    </message>
    <message>
        <source>Get List</source>
        <translation type="obsolete">Obter lista</translation>
    </message>
    <message>
        <source>Add</source>
        <translation type="obsolete">Adicionar</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="obsolete">Editar</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="obsolete">Apagar</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Nome</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="obsolete">Versão</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="obsolete">Descrição</translation>
    </message>
    <message>
        <source>Author</source>
        <translation type="obsolete">Autor</translation>
    </message>
    <message>
        <source>Name of plugin to install</source>
        <translation type="obsolete">Nome do plugin à ser instalado</translation>
    </message>
    <message>
        <source>Install Plugin</source>
        <translation type="obsolete">Instalar Plugin</translation>
    </message>
    <message>
        <source>The plugin will be installed to ~/.qgis/python/plugins</source>
        <translation type="obsolete">O plugin vai ser instalado para ~/.qgis/python/plugins</translation>
    </message>
    <message>
        <source>Done</source>
        <translation type="obsolete">Feito</translation>
    </message>
    <message>
        <source>Dialog</source>
        <translation type="obsolete">Diálogo</translation>
    </message>
    <message>
        <source>Point Symbol</source>
        <translation type="obsolete">Símbolo do Ponto</translation>
    </message>
    <message>
        <source>Size</source>
        <translation type="obsolete">Tamanho</translation>
    </message>
    <message>
        <source>Area scale field</source>
        <translation type="obsolete">Campo da escala da área</translation>
    </message>
    <message>
        <source>Rotation field</source>
        <translation type="obsolete">Campo da rotacão</translation>
    </message>
    <message>
        <source>Style Options</source>
        <translation type="obsolete">Opcões de estilo</translation>
    </message>
    <message>
        <source>Outline style</source>
        <translation type="obsolete">Estilo da borda</translation>
    </message>
    <message>
        <source>Outline color</source>
        <translation type="obsolete">Cor da borda</translation>
    </message>
    <message>
        <source>Outline width</source>
        <translation type="obsolete">Espessura da borda</translation>
    </message>
    <message>
        <source>Fill color</source>
        <translation type="obsolete">Cor de preenchimento</translation>
    </message>
    <message>
        <source>Fill style</source>
        <translation type="obsolete">Estilo de preenchimento</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="unfinished">Conectar</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation type="unfinished">Exibir</translation>
    </message>
    <message>
        <source>OGR Converter</source>
        <translation>Conversor OGR</translation>
    </message>
    <message>
        <source>Could not establish connection to: &apos;</source>
        <translation>Não foi possível conectar com: &apos;</translation>
    </message>
    <message>
        <source>Open OGR file</source>
        <translation>Abrir arquivo OGR</translation>
    </message>
    <message>
        <source>OGR File Data Source (*.*)</source>
        <translation>Fonte do arquivo OGR (*.*)</translation>
    </message>
    <message>
        <source>Open Directory</source>
        <translation>Abrir diretório</translation>
    </message>
    <message>
        <source>Input OGR dataset is missing!</source>
        <translation>Base de dados OGR está faltando!</translation>
    </message>
    <message>
        <source>Input OGR layer name is missing!</source>
        <translation>Camada de entrada OGR está faltando!</translation>
    </message>
    <message>
        <source>Target OGR format not selected!</source>
        <translation>Formato OGR de destino não selecionado!</translation>
    </message>
    <message>
        <source>Output OGR dataset is missing!</source>
        <translation>Base de dados OGR de saída está faltando!</translation>
    </message>
    <message>
        <source>Output OGR layer name is missing!</source>
        <translation>Camada de saída OGR está faltando!</translation>
    </message>
    <message>
        <source>Successfully translated layer &apos;</source>
        <translation>Camada convertida com sucesso &apos;</translation>
    </message>
    <message>
        <source>Failed to translate layer &apos;</source>
        <translation>Falha ao converter camada &apos;</translation>
    </message>
    <message>
        <source>Successfully connected to: &apos;</source>
        <translation>Conectado com sucesso à: &apos;</translation>
    </message>
    <message>
        <source>Choose a file name to save to</source>
        <translation>Escolha um nome de arquivo para salvar para</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Bem-vindo ao seu plugin automaticamente gerado!</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>Este é o ponto de partida. Você precisa saber modificar o código-fonte para que ele faca algo útil.... leia adiante para ter mais informacoes e iniciar sua aprendizagem.</translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation>Documentacao</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>Você realmente precisa ler a documentacao do QGIS API em:</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation>Em especial olhe as seguintes classes:</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>O Plugin QgsPlugin é um ABC que define o comportamento requerido que o seu plugin deve ter. Veja abaixo para mais detalhes.</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>Para que servem todos esses arquivos no diretório do meu plugin criado?</translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>Este é o arquivo criado pelo CMake que gera o plugin. Você deveria adicionar as dependências específicas e os arquivos fontes para esse arquivo.</translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>Esta é que classe que fornece o \&quot;elo\&quot; entre sua aplicação personalizada e o QGIS. Você verá que o número de métodos já implementados para você - incluíndo alguns exemplos como adicionar dados raster ou camadas vetoriais para a área da visão do mapa. Essa classe é uma instância concreta da interface do QgisPlugin, que define o comportamento requerido para um plugin. Em particular, um plugin têm um número estático de métodos e membros de forma que, o gerenciador do QgsPlugin e a lógica do carregador de plugins pode identificar cada plugin, criar uma entrada no menu apropriada para ele e etc. Note que não há nada lhe impedindo de criar múltiplas barras de ferramentas ou entradas de menu para somente um plugin. Por padrão uma única entrada no menu e um botão na barra de ferramentas são criados, e ele é pré-configurado na classe \&quot;run() method\&quot; quando selecionado. Esta implementação padrão provida à para você pelo construtor de plugins é bem documentada, então por favor refirá-se ao código para mais conselhos.</translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>Este é o arquivo para a interface com o usuário do Qt designer. Ele define o visual padrão dos diálogos do plugin sem implementar nenhuma lógica da aplicacão. Você pode modificar este formulário para atender suas necessidades, ou remové-lo completamente se o seu plugin não precisa mostrar algo ao usuário.  (p.ex. para mapas personalizados).</translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>Esta é a classe concreta onde a lógica da aplicação mencionada acima deveriam ir. \&quot;O mundo é sua ostra aqui, realmente ....\&quot;</translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>Este é o arquivo de recursos do Qt4 para o seu plugin. O Makefile gerado para o seu plugin é todo configurado para compilar o arquivo de recursos, então tudo que você precisa fazer é adicionar os seus ícones adicionais, e etc, usando um simples arquivo em formato xml. Preste atencão no formato de nome usado para todos os seus recursos (p. ex., \&quot;/Ferreira/\&quot;). Para ele é importante usar um prefixo igual para todos os seus recursos. Nós sugerimos à você incluir outras imagens e dados de compilacão à esse arquivo de recursos também.</translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>Este é o ícone que vai ser usado para o seu plugin no menu e na barra de ferramentas. Simplesmente troque este ícone com o seu próprio ícone para fazer o seu plugin distinto do resto.</translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation>This file contains the documentation you are reading now!</translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation>Obter ajuda dos desenvolvedores:</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>Para questões e comentários a respeito do exemplo usado para construir um plugin, e/ou a criacão de funcões no QGIS usando a interface de geracão de plugins, por favor nos contate via:</translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; a lista de email do desenvolvedores do QGIS , ou &lt;/li&gt;&lt;li&gt; IRC (#qgis em freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS é distribuído segundo a Licenca Pública GNU. Se você criar um plugin que pode ser útil à outras pessoas, por favor considere compartilhá-lo com a comunidade.</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Divirta-se e </translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <source>Enter map coordinates</source>
        <translation>Entre as coordenadas do mapa</translation>
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
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>Entre com as coordenadas X e Y que correspondem ao ponto selecionado na imagem. Alternativamente, clique no ícone com um lápis e, então, clique num ponto correspondente na superfície do mapa para preencher as cooredenadas para aquele ponto.</translation>
    </message>
    <message>
        <source> from map canvas</source>
        <translation> da superfície do mapa</translation>
    </message>
</context>
<context>
    <name>OgrConverterGuiBase</name>
    <message>
        <source>OGR Layer Converter</source>
        <translation>Conversor de Camada OGR</translation>
    </message>
    <message>
        <source>Source</source>
        <translation type="unfinished">Fonte</translation>
    </message>
    <message>
        <source>Format</source>
        <translation type="unfinished">Formatar</translation>
    </message>
    <message>
        <source>File</source>
        <translation type="unfinished">Arquivo</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation type="unfinished">Diretório</translation>
    </message>
    <message>
        <source>Remote source</source>
        <translation>Fonte remota</translation>
    </message>
    <message>
        <source>Dataset</source>
        <translation>Conjunto de dados</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation type="unfinished">Exibir</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <source>Target</source>
        <translation>Destino</translation>
    </message>
</context>
<context>
    <name>OgrPlugin</name>
    <message>
        <source>Run OGR Layer Converter</source>
        <translation>Rodar o conversor de camada OGR</translation>
    </message>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="obsolete">Troque isso por uma breve descrição do que o plugin faz</translation>
    </message>
    <message>
        <source>OG&amp;R Converter</source>
        <translation>Conversor OG&amp;R</translation>
    </message>
    <message>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation type="unfinished">Converte camadas vetoriais entre os diferentes formatos suportados pela biblioteca OGR</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <source>Load layer properties from style file (.qml)</source>
        <translation>Carregar propriedades da camada de um arquivo de estilo (.qml)</translation>
    </message>
    <message>
        <source>Save layer properties as style file (.qml)</source>
        <translation>Salvar propriedades da camada como um arquivo de estilo (.qml)</translation>
    </message>
    <message>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation>Salvar relatório do experimento para PDF (.pdf)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>QGis files (*.qgs)</source>
        <translation>Arquivos QGis (*.qgs)</translation>
    </message>
    <message>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation>Nenhum plugin de acesso a dados encontrado</translation>
    </message>
    <message>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>Nenhuma camada vetorial pôde ser carregada. Verifique sua instalação do QGIS</translation>
    </message>
    <message>
        <source>No Data Providers</source>
        <translation>Nenhum mecanismo de acesso a dados</translation>
    </message>
    <message>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>Nenhum plugin de acesso a dados disponível. Nenhuma camada vetorial pôde ser carregada</translation>
    </message>
    <message>
        <source> at line </source>
        <translation>na linha</translation>
    </message>
    <message>
        <source> column </source>
        <translation>coluna</translation>
    </message>
    <message>
        <source> for file </source>
        <translation>para arquivo</translation>
    </message>
    <message>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>Coluna referenciada não foi encontrada: </translation>
    </message>
    <message>
        <source>Division by zero.</source>
        <translation>Divisão por zero.</translation>
    </message>
    <message>
        <source>No active layer</source>
        <translation>Nenhuma camada ativa</translation>
    </message>
    <message>
        <source>Band</source>
        <translation>Banda</translation>
    </message>
    <message>
        <source>action</source>
        <translation>ação</translation>
    </message>
    <message>
        <source> features found</source>
        <translation> feições encontradas</translation>
    </message>
    <message>
        <source> 1 feature found</source>
        <translation> 1 feição encontrada</translation>
    </message>
    <message>
        <source>No features found</source>
        <translation>Nenhuma feição encontrada</translation>
    </message>
    <message>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>Nenhuma feição encontrada no ponto clicado da camada ativa</translation>
    </message>
    <message>
        <source>Could not identify objects on</source>
        <translation>Impossível identificar objetos em</translation>
    </message>
    <message>
        <source>because</source>
        <translation>porque</translation>
    </message>
    <message>
        <source>Unable to save to file </source>
        <translation>Impossível salvar o arquivo </translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation>Novo centróide</translation>
    </message>
    <message>
        <source>New point</source>
        <translation>Novo ponto</translation>
    </message>
    <message>
        <source>New vertex</source>
        <translation>Novo vértice</translation>
    </message>
    <message>
        <source>Undo last point</source>
        <translation>Desfazer último ponto</translation>
    </message>
    <message>
        <source>Close line</source>
        <translation>Fechar linha</translation>
    </message>
    <message>
        <source>Select vertex</source>
        <translation>Selecione vértice</translation>
    </message>
    <message>
        <source>Select new position</source>
        <translation>Selecione nova posição</translation>
    </message>
    <message>
        <source>Select line segment</source>
        <translation>Selecione segmento de linha</translation>
    </message>
    <message>
        <source>New vertex position</source>
        <translation>Nova posição do vértice</translation>
    </message>
    <message>
        <source>Release</source>
        <translation>Desmarcar</translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation>Excluir vértice</translation>
    </message>
    <message>
        <source>Release vertex</source>
        <translation>Desmarcar vértice</translation>
    </message>
    <message>
        <source>Select element</source>
        <translation>Selecione elemento</translation>
    </message>
    <message>
        <source>New location</source>
        <translation>Nova localidade</translation>
    </message>
    <message>
        <source>Release selected</source>
        <translation>Desmarcar selecionado</translation>
    </message>
    <message>
        <source>Delete selected / select next</source>
        <translation>Excluir selecionado / selecione próximo</translation>
    </message>
    <message>
        <source>Select position on line</source>
        <translation>Selecione posição na linha</translation>
    </message>
    <message>
        <source>Split the line</source>
        <translation>Quebrar linha</translation>
    </message>
    <message>
        <source>Release the line</source>
        <translation>Desmarcar a linha</translation>
    </message>
    <message>
        <source>Select point on line</source>
        <translation>Selecione ponto na linha</translation>
    </message>
    <message>
        <source>Label</source>
        <translation>Rótulo</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Comprimento</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Área</translation>
    </message>
    <message>
        <source>Project file read error: </source>
        <translation>Erro na leitura do arquivo de projeto: </translation>
    </message>
    <message>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>Extender para uma tranformação linear requer no mínimo 2 pontos.</translation>
    </message>
    <message>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Extender para uma transformação Helmet requer um mínimo de 2 pontos.</translation>
    </message>
    <message>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Estender para uma transformação afim requer no mínimo 4 pontos.</translation>
    </message>
    <message>
        <source>Couldn&apos;t open the data source: </source>
        <translation>Não foi possível abrir a fonte de dados:</translation>
    </message>
    <message>
        <source>Parse error at line </source>
        <translation>Erro de análise na linha </translation>
    </message>
    <message>
        <source>GPS eXchange format provider</source>
        <translation>Provedor de formato de troca GPS</translation>
    </message>
    <message>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Encontrada um excecao no sistema de coordenadas enquanto transformava um ponto. Impossível calcular o comprimento da linha.</translation>
    </message>
    <message>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation>Encontrada um excecao no sistema de coordenadas enquanto transformava um ponto. Impossível calcular o comprimento do polígono.</translation>
    </message>
    <message>
        <source>GRASS plugin</source>
        <translation>Plugin do GRASS</translation>
    </message>
    <message>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>QGIS não encontrou sua instalação do GRASS. 
Você gostaria de especificar um caminhos (GISBASE) para sua instalação GRASS?</translation>
    </message>
    <message>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Escolha o caminho de instalação do GRASS (GISBASE)</translation>
    </message>
    <message>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>Dados do GRASS não serão habilitados se o GISBASE não for especificado.</translation>
    </message>
    <message>
        <source>CopyrightLabel</source>
        <translation>Etiqueta de Copyright </translation>
    </message>
    <message>
        <source>Draws copyright information</source>
        <translation>Infomações do copyright do desenho</translation>
    </message>
    <message>
        <source>Version 0.1</source>
        <translation>Versão 0.1</translation>
    </message>
    <message>
        <source>Version 0.2</source>
        <translation>Versão 0.2</translation>
    </message>
    <message>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation>Carrega e mostra arquivos de texto delimitados contendo coordenadas x e y</translation>
    </message>
    <message>
        <source>Add Delimited Text Layer</source>
        <translation>Adiciona uma camada de texto delimitado</translation>
    </message>
    <message>
        <source>Georeferencer</source>
        <translation>Georreferenciador</translation>
    </message>
    <message>
        <source>Adding projection info to rasters</source>
        <translation>Adicionando informação de projeção para os rasters</translation>
    </message>
    <message>
        <source>GPS Tools</source>
        <translation>Ferramentas de GPS</translation>
    </message>
    <message>
        <source>Tools for loading and importing GPS data</source>
        <translation>Ferramentas para carregar e descarregar dados de GPS</translation>
    </message>
    <message>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation>camada do GRASS</translation>
    </message>
    <message>
        <source>Graticule Creator</source>
        <translation>Criador de Grade</translation>
    </message>
    <message>
        <source>Builds a graticule</source>
        <translation>Constrói uma grade</translation>
    </message>
    <message>
        <source>NorthArrow</source>
        <translation>Seta Norte</translation>
    </message>
    <message>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Mostra uma seta Norte sobreposta ao mapa</translation>
    </message>
    <message>
        <source>[menuitemname]</source>
        <translation>{nomedoitemdomenu]</translation>
    </message>
    <message>
        <source>[plugindescription]</source>
        <translation>[descriçãodoplugin]</translation>
    </message>
    <message>
        <source>ScaleBar</source>
        <translation>Barra de escala</translation>
    </message>
    <message>
        <source>Draws a scale bar</source>
        <translation>Desenha uma barra de escala</translation>
    </message>
    <message>
        <source>SPIT</source>
        <translation>SPIT</translation>
    </message>
    <message>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>Ferramenta para importar um Shapefile para PostgreSQL/PostGIS </translation>
    </message>
    <message>
        <source>WFS plugin</source>
        <translation>Plugin WFS</translation>
    </message>
    <message>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>Adiciona uma camada WFS para a tela do QGIS</translation>
    </message>
    <message>
        <source>Not a vector layer</source>
        <translation>Não é uma camada vetorial</translation>
    </message>
    <message>
        <source>The current layer is not a vector layer</source>
        <translation>A camada atual não é uma camada vetorial</translation>
    </message>
    <message>
        <source>Layer cannot be added to</source>
        <translation>Camada não pode ser adicionada para</translation>
    </message>
    <message>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>O provedor de dados para esta camada não suporta a adição de feições.</translation>
    </message>
    <message>
        <source>Layer not editable</source>
        <translation>A camada não pode ser editada</translation>
    </message>
    <message>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>Camada bloqueada para edição. Para tornar editável você precisa clicar com o botão direito do mause e habilitar &apos;Permitir edição&apos;.</translation>
    </message>
    <message>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Para selecionar feições você precisa escolher uma camada clicando em seu nome na legenda</translation>
    </message>
    <message>
        <source>Python error</source>
        <translation>Erro Python</translation>
    </message>
    <message>
        <source>Couldn&apos;t load plugin </source>
        <translation>Não consegui carregar o plugin </translation>
    </message>
    <message>
        <source> due an error when calling its classFactory() method</source>
        <translation> devido um erro when calling its classFactory() method</translation>
    </message>
    <message>
        <source> due an error when calling its initGui() method</source>
        <translation> devido a um erro ao chamar seu método Gui()</translation>
    </message>
    <message>
        <source>Error while unloading plugin </source>
        <translation>Erro enquanto descarregava o plugin</translation>
    </message>
    <message>
        <source>2.5D shape type not supported</source>
        <translation>Shape tipo 2.5D não suportado</translation>
    </message>
    <message>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>Adicionar feições para shapes tipo 2.5D ainda não suportada</translation>
    </message>
    <message>
        <source>Wrong editing tool</source>
        <translation>Ferramanta de edição errada</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>Impossível aplicar a ferramenta &apos;ponto de captura&apos; nesta camada vetorial</translation>
    </message>
    <message>
        <source>Coordinate transform error</source>
        <translation>Erro na transformação da coordenada</translation>
    </message>
    <message>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>Impossível transformar o ponto para o sistema de coordenadas da camada</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>Impossível aplicar a ferramenta &apos;capturar linha&apos; nesta camada vetoria</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>Impossível aplicar a ferramenta &apos;capturar polígono&apos; nesta camada vetorial</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Erro</translation>
    </message>
    <message>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>Impossível adicionar feição. Tipo WKB desconhecido</translation>
    </message>
    <message>
        <source>Error, could not add island</source>
        <translation>Erro, impossível adicionar ilha</translation>
    </message>
    <message>
        <source>A problem with geometry type occured</source>
        <translation>Ocorreu um problema com o tipo de geometria</translation>
    </message>
    <message>
        <source>The inserted Ring is not closed</source>
        <translation>O Anel inserido não está fechado</translation>
    </message>
    <message>
        <source>The inserted Ring is not a valid geometry</source>
        <translation>O Anel inserido não é uma geometria válida</translation>
    </message>
    <message>
        <source>The inserted Ring crosses existing rings</source>
        <translation>O Anel inserido cruza anéis existentes</translation>
    </message>
    <message>
        <source>The inserted Ring is not contained in a feature</source>
        <translation>O Anel inserido não está contido na feição</translation>
    </message>
    <message>
        <source>An unknown error occured</source>
        <translation>Ocorreu um erro desconhecido</translation>
    </message>
    <message>
        <source>Error, could not add ring</source>
        <translation>Erro, não posso adicionar anel</translation>
    </message>
    <message>
        <source> km2</source>
        <translation> km2</translation>
    </message>
    <message>
        <source> ha</source>
        <translation> ha</translation>
    </message>
    <message>
        <source> m2</source>
        <translation> m2</translation>
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
        <translation> sq mile</translation>
    </message>
    <message>
        <source> sq ft</source>
        <translation> sq ft</translation>
    </message>
    <message>
        <source> mile</source>
        <translation> milha</translation>
    </message>
    <message>
        <source> foot</source>
        <translation> pés</translation>
    </message>
    <message>
        <source> feet</source>
        <translation> pés</translation>
    </message>
    <message>
        <source> sq.deg.</source>
        <translation> sq deg.</translation>
    </message>
    <message>
        <source> degree</source>
        <translation> graus</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation> graus</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation> desconhecido</translation>
    </message>
    <message>
        <source>Received %1 of %2 bytes</source>
        <translation>Recebidos %1 de %2 bytes</translation>
    </message>
    <message>
        <source>Received %1 bytes (total unknown)</source>
        <translation>Recebidos %1 bytes (total desconhecido)</translation>
    </message>
    <message>
        <source>Not connected</source>
        <translation>Não conectado</translation>
    </message>
    <message>
        <source>Looking up &apos;%1&apos;</source>
        <translation>Olhando para &apos;%1&apos;</translation>
    </message>
    <message>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>Conectando a &apos;%1&apos;</translation>
    </message>
    <message>
        <source>Sending request &apos;%1&apos;</source>
        <translation>Enviando requisição &apos;%1&apos;</translation>
    </message>
    <message>
        <source>Receiving reply</source>
        <translation>Recebendo resposta</translation>
    </message>
    <message>
        <source>Response is complete</source>
        <translation>Resposta completa</translation>
    </message>
    <message>
        <source>Closing down connection</source>
        <translation>Fechando a conexão</translation>
    </message>
    <message>
        <source>Unable to open </source>
        <translation>Impossível abrir</translation>
    </message>
    <message>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation>Expressões regulares nos valores numéricos não têm senso. Ao invés disso, use comparação.</translation>
    </message>
    <message>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Funções de geoprocessamento para trabalhar com camadas PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Local: </translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Maset: </translation>
    </message>
    <message>
        <source>Location: </source>
        <translation>Local: </translation>
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
        <translation>Impossível abrir cabaçalho raster</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation>Linhas</translation>
    </message>
    <message>
        <source>Columns</source>
        <translation>Colunas</translation>
    </message>
    <message>
        <source>N-S resolution</source>
        <translation>Resolução N-S</translation>
    </message>
    <message>
        <source>E-W resolution</source>
        <translation>Resolução E-W</translation>
    </message>
    <message>
        <source>North</source>
        <translation>Norte</translation>
    </message>
    <message>
        <source>South</source>
        <translation>Sul</translation>
    </message>
    <message>
        <source>East</source>
        <translation>Leste</translation>
    </message>
    <message>
        <source>West</source>
        <translation>Oeste</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Formatar</translation>
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
        <translation>Fonte de dados</translation>
    </message>
    <message>
        <source>Data description</source>
        <translation>Decrição de dados</translation>
    </message>
    <message>
        <source>Comments</source>
        <translation>Comentários</translation>
    </message>
    <message>
        <source>Categories</source>
        <translation>Categorias</translation>
    </message>
    <message>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;Vetor&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Points</source>
        <translation>Pontos</translation>
    </message>
    <message>
        <source>Lines</source>
        <translation>Linhas</translation>
    </message>
    <message>
        <source>Boundaries</source>
        <translation>Limites</translation>
    </message>
    <message>
        <source>Centroids</source>
        <translation>Centróides</translation>
    </message>
    <message>
        <source>Faces</source>
        <translation>Faces</translation>
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
        <translation>Ilhas</translation>
    </message>
    <message>
        <source>Top</source>
        <translation>Topo</translation>
    </message>
    <message>
        <source>Bottom</source>
        <translation>Base</translation>
    </message>
    <message>
        <source>yes</source>
        <translation>sim</translation>
    </message>
    <message>
        <source>no</source>
        <translation>não</translation>
    </message>
    <message>
        <source>History&lt;br&gt;</source>
        <translation>Histórico&lt;br&gt;</translation>
    </message>
    <message>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Camada&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Features</source>
        <translation>Feições</translation>
    </message>
    <message>
        <source>Driver</source>
        <translation>Driver</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>Banco de Dados</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabela</translation>
    </message>
    <message>
        <source>Key column</source>
        <translation>Coluna chave</translation>
    </message>
    <message>
        <source>GISBASE is not set.</source>
        <translation>GISBASE não definido.</translation>
    </message>
    <message>
        <source> is not a GRASS mapset.</source>
        <translation>Não é um GRASS mapset </translation>
    </message>
    <message>
        <source>Cannot start </source>
        <translation>Impossível iniciar</translation>
    </message>
    <message>
        <source>Mapset is already in use.</source>
        <translation>Mapset já em uso. </translation>
    </message>
    <message>
        <source>Temporary directory </source>
        <translation>Diretório temporário </translation>
    </message>
    <message>
        <source> exist but is not writable</source>
        <translation> existe mas não é gravável</translation>
    </message>
    <message>
        <source>Cannot create temporary directory </source>
        <translation>Impossível criar diretório temporário </translation>
    </message>
    <message>
        <source>Cannot create </source>
        <translation>Impossível criar</translation>
    </message>
    <message>
        <source>Cannot remove mapset lock: </source>
        <translation>Impossível remover mapset bloqueado: </translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Aviso</translation>
    </message>
    <message>
        <source>Cannot read raster map region</source>
        <translation>Impossível ler região do mapa raster</translation>
    </message>
    <message>
        <source>Cannot read vector map region</source>
        <translation>Impossível ler região do mapa vetorial</translation>
    </message>
    <message>
        <source>Cannot read region</source>
        <translation>Impossível ler região</translation>
    </message>
    <message>
        <source>Where is &apos;</source>
        <translation>Onde está &apos;</translation>
    </message>
    <message>
        <source>original location: </source>
        <translation>local original: </translation>
    </message>
    <message>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Para identificar feições, você precisa escolher uma camada ativa na legenda clicando no seu nome</translation>
    </message>
    <message>
        <source>Could not remove polygon intersection</source>
        <translation>Não foi possível remover a interseccão de polígonos</translation>
    </message>
    <message>
        <source>Loaded default style file from </source>
        <translation type="obsolete">Estilo padrão carregado de.</translation>
    </message>
    <message>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation>O diretório contendo a sua base de dados precisa ter permissão de escrita!</translation>
    </message>
    <message>
        <source>Created default style file as </source>
        <translation>Estilo padrão criado como arquivo.</translation>
    </message>
    <message>
        <source>PostgreSQL Geoprocessing</source>
        <translation>Geoprocessamento PostgreSQL</translation>
    </message>
    <message>
        <source>Quick Print</source>
        <translation>Impressão rápida</translation>
    </message>
    <message>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation>Impressão rápida é um plugin para rapidamente criar um mapa e com mínimo esforço.</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation type="unfinished">Abortar</translation>
    </message>
    <message>
        <source>Version 0.001</source>
        <translation type="unfinished">Versão 0.001</translation>
    </message>
    <message>
        <source>ERROR: Failed to created default style file as %1 Check file permissions and retry.</source>
        <translation>ERRO: A criação do arquivo do estilo padrão falhou em %1 Cheque as permissões do arquivo e tente novamente.</translation>
    </message>
    <message>
        <source> is not writeable.</source>
        <translation>.não tem permissão de escrita.</translation>
    </message>
    <message>
        <source>Please adjust permissions (if possible) and try again.</source>
        <translation>Por favor ajuste as permissões (se isso for possível) e tente novamente.</translation>
    </message>
    <message>
        <source>Uncatched fatal GRASS error</source>
        <translation>Erro fatal no GRASS</translation>
    </message>
    <message>
        <source>Couldn&apos;t load SIP module.</source>
        <translation>Não foi possível carregar o módulo SIP.</translation>
    </message>
    <message>
        <source>Python support will be disabled.</source>
        <translation>Suporte à Python vai ser desabilitado.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQt4.</source>
        <translation>Não foi possível carregar PyQt4.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQGIS.</source>
        <translation>Não foi possível carregar PyQGIS.</translation>
    </message>
    <message>
        <source>An error has occured while executing Python code:</source>
        <translation>Um erro ocorreu enquanto executava o seguinte código Python:</translation>
    </message>
    <message>
        <source>Python version:</source>
        <translation>Versão do Python:</translation>
    </message>
    <message>
        <source>Python path:</source>
        <translation>Caminho para o Python:</translation>
    </message>
    <message>
        <source>An error occured during execution of following code:</source>
        <translation>Um erro ocorreu durante a execução do seguinte código:</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation type="unfinished">Legenda</translation>
    </message>
    <message>
        <source>Coordinate Capture</source>
        <translation>Captura de coordenadas</translation>
    </message>
    <message>
        <source>Capture mouse coordinates in different CRS</source>
        <translation>Capturar coordenadas do mouse em um diferente CRS</translation>
    </message>
    <message>
        <source>Dxf2Shp Converter</source>
        <translation>Conversor Dxf2Shp</translation>
    </message>
    <message>
        <source>Converts from dxf to shp file format</source>
        <translation>Converte de um arquivo DXF para Shapefile</translation>
    </message>
    <message>
        <source>Interpolating...</source>
        <translation>Interpolando...</translation>
    </message>
    <message>
        <source>Interpolation plugin</source>
        <translation>Plugin de Interpolação</translation>
    </message>
    <message>
        <source>A plugin for interpolation based on vertices of a vector layer</source>
        <translation>Um plugin de interpolação baseado nos vértices de uma camada vetorial</translation>
    </message>
    <message>
        <source>OGR Layer Converter</source>
        <translation>Conversor de camadas OGR</translation>
    </message>
    <message>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation>Converte camadas vetoriais entre os diferentes formatos suportados pela biblioteca OGR</translation>
    </message>
    <message>
        <source>CRS Exception</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Selection extends beyond layer&apos;s coordinate system.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Loading style file </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> failed because:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not save symbology because:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to save to file. Your project may be corrupted on disk. Try clearing some space on the volume and check file permissions before pressing save again.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS -</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Versão</translation>
    </message>
    <message>
        <source>is not a valid or recognized data source</source>
        <translation>não é uma fonte de dados válida ou conhecida</translation>
    </message>
    <message>
        <source>Invalid Data Source</source>
        <translation>Fonte de Dados Inválida</translation>
    </message>
    <message>
        <source>Invalid Layer</source>
        <translation>Camada Inválida</translation>
    </message>
    <message>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>%1 é uma camada inválida e não pode ser carregada.</translation>
    </message>
    <message>
        <source>No Layer Selected</source>
        <translation>Nenhuma Camada Selecionada</translation>
    </message>
    <message>
        <source>Error Loading Plugin</source>
        <translation>Erro Carregando Plugin</translation>
    </message>
    <message>
        <source>There was an error loading %1.</source>
        <translation>Ocorreu um erro ao carregar %1.</translation>
    </message>
    <message>
        <source>There is a new version of QGIS available</source>
        <translation>Existe uma nova versão do QGIS disponível</translation>
    </message>
    <message>
        <source>You are running a development version of QGIS</source>
        <translation>Você está executando uma versão de desenvolvimento do QGIS</translation>
    </message>
    <message>
        <source>You are running the current version of QGIS</source>
        <translation>Você está rodando a versão atual do QGIS</translation>
    </message>
    <message>
        <source>Would you like more information?</source>
        <translation>Gostaria de obter mais informações?</translation>
    </message>
    <message>
        <source>QGIS Version Information</source>
        <translation>Informações sobre a versão do QGIS</translation>
    </message>
    <message>
        <source>Unable to get current version information from server</source>
        <translation>Impossível obter informações sobre a versão atual</translation>
    </message>
    <message>
        <source>Connection refused - server may be down</source>
        <translation>Conexão recusada - o servidor pode estar indisponível</translation>
    </message>
    <message>
        <source>QGIS server was not found</source>
        <translation>Servidor do QGIS não foi encontrado</translation>
    </message>
    <message>
        <source>Saved map image to</source>
        <translation>Imagem de mapa salva em</translation>
    </message>
    <message>
        <source>Extents: </source>
        <translation>Extensão:</translation>
    </message>
    <message>
        <source>Problem deleting features</source>
        <translation>Problema ao excluir feições</translation>
    </message>
    <message>
        <source>A problem occured during deletion of features</source>
        <translation>Um problema ocorreu durante a exclusão das feições</translation>
    </message>
    <message>
        <source>No Vector Layer Selected</source>
        <translation>Nenhuma camada vetorial selecionada</translation>
    </message>
    <message>
        <source>Deleting features only works on vector layers</source>
        <translation>Apagar feições funciona apenas em camadas vetoriais</translation>
    </message>
    <message>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation>Para excluir feições. você precisa selecionar uma camada vetorial</translation>
    </message>
    <message>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>A legenda do mapa mostra todas as camadas na área do mapa. Clique na caixa para ativar ou desativar a camada. Duplo clique em uma camada em sua legenda serve para customizar sua aparência e ajustar outras propriedades.</translation>
    </message>
    <message>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Visão geral da área do mapa. Esta área pode ser usado para mostrar um localizador na área total do mapa. A extensão atual é mostrada como um retângulo vermelho. Qualquer camada do mapa pode ser adicionada para a visão geral.</translation>
    </message>
    <message>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation type="unfinished">Área do mapa. Onde camadas vetoriais e raster são exibidas quando adicionadas ao mapa.</translation>
    </message>
    <message>
        <source>&amp;Plugins</source>
        <translation>&amp;Plugins</translation>
    </message>
    <message>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>A barra de progresso mostra o status da renderização das camadas e outras operações que levam muito tempo</translation>
    </message>
    <message>
        <source>Displays the current map scale</source>
        <translation>Exibe a escala atual do mapa</translation>
    </message>
    <message>
        <source>Render</source>
        <translation>Desenhar</translation>
    </message>
    <message>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Quando marcadas, as camadas do mapa são renderezidas em resposta aos comandos de navegação pelo mapa e outros eventos. Quando não marcadas, nenhuma renderização será feita. Isso permite que você adicione um grande número de camadas e as altere antes de renderizá-las.</translation>
    </message>
    <message>
        <source>Unable to save project</source>
        <translation>Incapaz de salvar projeto</translation>
    </message>
    <message>
        <source>Unable to save project to </source>
        <translation>Incapaz de salvar projeto em</translation>
    </message>
    <message>
        <source>Toggle map rendering</source>
        <translation>Ativa a renderização do mapa</translation>
    </message>
    <message>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>Abrir uma camada vetorial OGR suportada</translation>
    </message>
    <message>
        <source>QGIS Project Read Error</source>
        <translation>Erro de leitura do projeto QGIS </translation>
    </message>
    <message>
        <source>Try to find missing layers?</source>
        <translation>Tento encontrar as camadas que estão faltando?</translation>
    </message>
    <message>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>Abrir uma fonte de dados raster GDAL suportada</translation>
    </message>
    <message>
        <source>Reading settings</source>
        <translation>Lendo características</translation>
    </message>
    <message>
        <source>Setting up the GUI</source>
        <translation>Configurar o GUI</translation>
    </message>
    <message>
        <source>Checking database</source>
        <translation>Checando base de dados</translation>
    </message>
    <message>
        <source>Restoring loaded plugins</source>
        <translation>Restaurar plugins carregados</translation>
    </message>
    <message>
        <source>Initializing file filters</source>
        <translation>Iniciar filtros de arquivo</translation>
    </message>
    <message>
        <source>Restoring window state</source>
        <translation>Restaurar estado da janela</translation>
    </message>
    <message>
        <source>QGIS Ready!</source>
        <translation>QGIS pronto!</translation>
    </message>
    <message>
        <source>&amp;New Project</source>
        <translation>&amp;Novo Projeto</translation>
    </message>
    <message>
        <source>New Project</source>
        <translation>Novo Projeto</translation>
    </message>
    <message>
        <source>&amp;Open Project...</source>
        <translation>&amp;Abrir Projeto...</translation>
    </message>
    <message>
        <source>Open a Project</source>
        <translation>Abre um Projeto</translation>
    </message>
    <message>
        <source>&amp;Save Project</source>
        <translation>Salvar &amp;Projeto</translation>
    </message>
    <message>
        <source>Save Project</source>
        <translation>Salvar Projeto</translation>
    </message>
    <message>
        <source>Save Project &amp;As...</source>
        <translation>Salvar Projeto &amp;Como...</translation>
    </message>
    <message>
        <source>Save Project under a new name</source>
        <translation>Salva Projeto com um novo nome</translation>
    </message>
    <message>
        <source>Save as Image...</source>
        <translation>Salva como imagem...</translation>
    </message>
    <message>
        <source>Save map as image</source>
        <translation>Salva mapa como imagem</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation>Sair</translation>
    </message>
    <message>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <source>Exit QGIS</source>
        <translation>Sair do QGIS</translation>
    </message>
    <message>
        <source>Add a Vector Layer</source>
        <translation>Adiciona uma Camada Vetorial</translation>
    </message>
    <message>
        <source>Add a Raster Layer</source>
        <translation>Adicionar uma Camada Raster</translation>
    </message>
    <message>
        <source>Add a PostGIS Layer</source>
        <translation>Adiciona uma Camada PostGIS</translation>
    </message>
    <message>
        <source>New Vector Layer...</source>
        <translation>Nova camada vetorial</translation>
    </message>
    <message>
        <source>Create a New Vector Layer</source>
        <translation>Criar .uma nova camada vetorial</translation>
    </message>
    <message>
        <source>Remove Layer</source>
        <translation>Remover camada</translation>
    </message>
    <message>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <source>Remove a Layer</source>
        <translation>Remover uma camada</translation>
    </message>
    <message>
        <source>Show all layers in the overview map</source>
        <translation>Mostrar todas as camadas no &apos;overview map&apos;</translation>
    </message>
    <message>
        <source>Remove All From Overview</source>
        <translation>Remover tudo da Visão Geral</translation>
    </message>
    <message>
        <source>Remove all layers from overview map</source>
        <translation>Remover todas as camadas do &apos;overview map&apos;</translation>
    </message>
    <message>
        <source>Show All Layers</source>
        <translation>Mostrar Todas as Camadas</translation>
    </message>
    <message>
        <source>Show all layers</source>
        <translation>Exibir todas as camadas</translation>
    </message>
    <message>
        <source>Hide All Layers</source>
        <translation>Ocultar Todas as Camadas</translation>
    </message>
    <message>
        <source>Hide all layers</source>
        <translation>Oculta todas as camadas</translation>
    </message>
    <message>
        <source>Project Properties...</source>
        <translation>Propriedades do projeto...</translation>
    </message>
    <message>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation>P</translation>
    </message>
    <message>
        <source>Set project properties</source>
        <translation>Gerencie as propriedades do projeto</translation>
    </message>
    <message>
        <source>Options...</source>
        <translation>Opções...</translation>
    </message>
    <message>
        <source>Change various QGIS options</source>
        <translation>Modificar várias opções do QGIS</translation>
    </message>
    <message>
        <source>Help Contents</source>
        <translation>Conteúdo da Ajuda</translation>
    </message>
    <message>
        <source>Help Documentation</source>
        <translation>Documentação da ajuda</translation>
    </message>
    <message>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation>Ctrl+H</translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>Site do QGIS</translation>
    </message>
    <message>
        <source>About</source>
        <translation>Sobre</translation>
    </message>
    <message>
        <source>About QGIS</source>
        <translation>Sobreo o QGIS</translation>
    </message>
    <message>
        <source>Check Qgis Version</source>
        <translation>Checar a versão do QGIS</translation>
    </message>
    <message>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Checar se seu QGIS é a versão atual (requer acesso a internet)</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Atualizar</translation>
    </message>
    <message>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <source>Refresh Map</source>
        <translation>Atualizar mapa</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Aproximar</translation>
    </message>
    <message>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation>Ctrl++</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Afastar</translation>
    </message>
    <message>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation>Ctrl+-</translation>
    </message>
    <message>
        <source>Zoom Full</source>
        <translation>Ver tudo</translation>
    </message>
    <message>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation>F</translation>
    </message>
    <message>
        <source>Zoom to Full Extents</source>
        <translation>Ver a toda a extensão</translation>
    </message>
    <message>
        <source>Pan Map</source>
        <translation>Panoramica no Mapa</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation>Panoramica no mapa</translation>
    </message>
    <message>
        <source>Zoom Last</source>
        <translation>Última vizualização</translation>
    </message>
    <message>
        <source>Zoom to Last Extent</source>
        <translation>Ver extenção anterior</translation>
    </message>
    <message>
        <source>Zoom to Layer</source>
        <translation>Ver a camada</translation>
    </message>
    <message>
        <source>Identify Features</source>
        <translation>Identifica feições</translation>
    </message>
    <message>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation>I</translation>
    </message>
    <message>
        <source>Click on features to identify them</source>
        <translation>Clique nas feições para identificá-las</translation>
    </message>
    <message>
        <source>Select Features</source>
        <translation>Selecionar feições</translation>
    </message>
    <message>
        <source>Measure Line </source>
        <translation>Medir linha</translation>
    </message>
    <message>
        <source>Measure a Line</source>
        <translation>Mede uma linha</translation>
    </message>
    <message>
        <source>Measure Area</source>
        <translation>Medir Área</translation>
    </message>
    <message>
        <source>Measure an Area</source>
        <translation>Mede uma área</translation>
    </message>
    <message>
        <source>Show Bookmarks</source>
        <translation>Mostra Favoritos</translation>
    </message>
    <message>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation>B</translation>
    </message>
    <message>
        <source>New Bookmark...</source>
        <translation>Novo Favorito</translation>
    </message>
    <message>
        <source>New Bookmark</source>
        <translation>Novo Favorito</translation>
    </message>
    <message>
        <source>Add WMS Layer...</source>
        <translation>Adiciona camada WMS</translation>
    </message>
    <message>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation>O</translation>
    </message>
    <message>
        <source>Add current layer to overview map</source>
        <translation>Adiciona a camada ativa ao overview map</translation>
    </message>
    <message>
        <source>Open the plugin manager</source>
        <translation>Abrir o gerenciador de plugin</translation>
    </message>
    <message>
        <source>Capture Point</source>
        <translation>Capturar Ponto</translation>
    </message>
    <message>
        <source>.</source>
        <comment>Capture Points</comment>
        <translation>.</translation>
    </message>
    <message>
        <source>Capture Points</source>
        <translation>Captura pontos</translation>
    </message>
    <message>
        <source>Capture Line</source>
        <translation>Capturar linha</translation>
    </message>
    <message>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation>/</translation>
    </message>
    <message>
        <source>Capture Lines</source>
        <translation>Captura linhas</translation>
    </message>
    <message>
        <source>Capture Polygon</source>
        <translation>Capturar Polígono</translation>
    </message>
    <message>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation>Ctrl+/</translation>
    </message>
    <message>
        <source>Capture Polygons</source>
        <translation>Captura polígonos</translation>
    </message>
    <message>
        <source>Delete Selected</source>
        <translation>Excluir seleção</translation>
    </message>
    <message>
        <source>Add Vertex</source>
        <translation>Adicionar Vértice</translation>
    </message>
    <message>
        <source>Delete Vertex</source>
        <translation>Exclui vértice</translation>
    </message>
    <message>
        <source>Move Vertex</source>
        <translation>Mover vértice</translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>&amp;Arquivo</translation>
    </message>
    <message>
        <source>&amp;Open Recent Projects</source>
        <translation>&amp;Abrir projetos recentes</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Exibir</translation>
    </message>
    <message>
        <source>&amp;Layer</source>
        <translation>&amp;Camada</translation>
    </message>
    <message>
        <source>&amp;Settings</source>
        <translation>&amp;Configurações</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Ajuda</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Arquivo</translation>
    </message>
    <message>
        <source>Manage Layers</source>
        <translation>Gerenciar camadas</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ajuda</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitalizar</translation>
    </message>
    <message>
        <source>Map Navigation</source>
        <translation>Navegar no mapa</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Atributos</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation>Plugins</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation>Pronto</translation>
    </message>
    <message>
        <source>New features</source>
        <translation>Novas feições</translation>
    </message>
    <message>
        <source>Save As</source>
        <translation>Salvar como</translation>
    </message>
    <message>
        <source>Choose a QGIS project file to open</source>
        <translation>Escolha um projeto do QGIS para abrir</translation>
    </message>
    <message>
        <source>Unable to open project</source>
        <translation>Impossível abrir projeto</translation>
    </message>
    <message>
        <source>Choose a QGIS project file</source>
        <translation>Escolha um projeto do QGIS</translation>
    </message>
    <message>
        <source>Saved project to:</source>
        <translation>Projeto salvo em:</translation>
    </message>
    <message>
        <source>Unable to save project </source>
        <translation>Impossível salvar projeto </translation>
    </message>
    <message>
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: Impossível carregar projeto</translation>
    </message>
    <message>
        <source>Unable to load project </source>
        <translation>Impossível carregar projeto </translation>
    </message>
    <message>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS - Mudificações no SVN desde o último lançamento</translation>
    </message>
    <message>
        <source>Layer is not valid</source>
        <translation>Camada não é válida</translation>
    </message>
    <message>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>A camada não é uma camada válida e não pode ser adicionada ao mapa</translation>
    </message>
    <message>
        <source>Save?</source>
        <translation>Salvar?</translation>
    </message>
    <message>
        <source> is not a valid or recognized raster data source</source>
        <translation>é uma fonte de dados raster não reconhecida ou inválida</translation>
    </message>
    <message>
        <source> is not a supported raster data source</source>
        <translation>é uma fonte de dados raster não suportada</translation>
    </message>
    <message>
        <source>Unsupported Data Source</source>
        <translation>Fonte de dados não suportada</translation>
    </message>
    <message>
        <source>Enter a name for the new bookmark:</source>
        <translation>Entre com o nome para o novo favorito: </translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Erro</translation>
    </message>
    <message>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>Impossível criar o favorito. Seu usuário pode estar perdido ou corrompido</translation>
    </message>
    <message>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation>Ctrl+?</translation>
    </message>
    <message>
        <source>Cut Features</source>
        <translation>cortar feições</translation>
    </message>
    <message>
        <source>Cut selected features</source>
        <translation>Corta feições selecionadas</translation>
    </message>
    <message>
        <source>Copy Features</source>
        <translation>Copiar feições</translation>
    </message>
    <message>
        <source>Copy selected features</source>
        <translation>Copia feições elecionadas</translation>
    </message>
    <message>
        <source>Paste Features</source>
        <translation>Cola feições</translation>
    </message>
    <message>
        <source>Paste selected features</source>
        <translation>Cola feições selecionadas</translation>
    </message>
    <message>
        <source>Network error while communicating with server</source>
        <translation>Erro na comunicação enquanto comunica com o servidor</translation>
    </message>
    <message>
        <source>Unknown network socket error</source>
        <translation>Erro de encaixe de rede desconhecido</translation>
    </message>
    <message>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>Impossível a comunicação com esta versão do QGIS</translation>
    </message>
    <message>
        <source>Checking provider plugins</source>
        <translation>Verificando provedor de plugins</translation>
    </message>
    <message>
        <source>Starting Python</source>
        <translation>Iniciando Python</translation>
    </message>
    <message>
        <source>Python error</source>
        <translation>Erro Python</translation>
    </message>
    <message>
        <source>Error when reading metadata of plugin </source>
        <translation>Erro ao ler metadados do plugin</translation>
    </message>
    <message>
        <source>Provider does not support deletion</source>
        <translation>O provedor não suporta apagar o arquivo</translation>
    </message>
    <message>
        <source>Data provider does not support deleting features</source>
        <translation>O provedor de dados não suporta apagar feições</translation>
    </message>
    <message>
        <source>Layer not editable</source>
        <translation>A camada não pode ser editada</translation>
    </message>
    <message>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation>A camada atual não é editável. Escolha &apos;Iniciar edição&apos; na barra da ferramentas de digitalização</translation>
    </message>
    <message>
        <source>Toggle editing</source>
        <translation>Alternar edição</translation>
    </message>
    <message>
        <source>Toggles the editing state of the current layer</source>
        <translation>Alterna o estado de edição da camada ativa</translation>
    </message>
    <message>
        <source>Add Ring</source>
        <translation>Adiciona anel</translation>
    </message>
    <message>
        <source>Add Island</source>
        <translation>Adiciona ilha</translation>
    </message>
    <message>
        <source>Add Island to multipolygon</source>
        <translation>Adiciona ilha ao multipolígono</translation>
    </message>
    <message>
        <source>Scale </source>
        <translation>Escala</translation>
    </message>
    <message>
        <source>Current map scale (formatted as x:y)</source>
        <translation>Escala do mapa atual (formatada como x:y)</translation>
    </message>
    <message>
        <source>Map coordinates at mouse cursor position</source>
        <translation>Coordenadas onde o cursor do mouse se encontra</translation>
    </message>
    <message>
        <source>Invalid scale</source>
        <translation>escala inválida</translation>
    </message>
    <message>
        <source>Do you want to save the current project?</source>
        <translation>Você quer salvar o projeto atual?</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <comment>











New Project</comment>
        <translation type="obsolete">Ctrl+N</translation>
    </message>
    <message>
        <source>Ctrl+O</source>
        <comment>











Open a Project</comment>
        <translation type="obsolete">Ctrl+O</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <comment>











Save Project</comment>
        <translation type="obsolete">Ctrl+S</translation>
    </message>
    <message>
        <source>V</source>
        <comment>











Add a Vector Layer</comment>
        <translation type="obsolete">V</translation>
    </message>
    <message>
        <source>R</source>
        <comment>











Add a Raster Layer</comment>
        <translation type="obsolete">R</translation>
    </message>
    <message>
        <source>D</source>
        <comment>











Add a PostGIS Layer</comment>
        <translation type="obsolete">D</translation>
    </message>
    <message>
        <source>N</source>
        <comment>











Create a New Vector Layer</comment>
        <translation type="obsolete">N</translation>
    </message>
    <message>
        <source>+</source>
        <comment>











Show all layers in the overview map</comment>
        <translation type="obsolete">+</translation>
    </message>
    <message>
        <source>-</source>
        <comment>











Remove all layers from overview map</comment>
        <translation type="obsolete">-</translation>
    </message>
    <message>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation>Ctrl-F</translation>
    </message>
    <message>
        <source>Toggle fullscreen mode</source>
        <translation>Ativar modo tela inteira</translation>
    </message>
    <message>
        <source>S</source>
        <comment>











Show all layers</comment>
        <translation type="obsolete">S</translation>
    </message>
    <message>
        <source>H</source>
        <comment>











Hide all layers</comment>
        <translation type="obsolete">H</translation>
    </message>
    <message>
        <source>F1</source>
        <comment>











Help Documentation</comment>
        <translation type="obsolete">F1</translation>
    </message>
    <message>
        <source>Ctrl+B</source>
        <comment>











New Bookmark</comment>
        <translation type="obsolete">Ctrl+B</translation>
    </message>
    <message>
        <source>Move Feature</source>
        <translation>Mover Feição</translation>
    </message>
    <message>
        <source>Split Features</source>
        <translation>Dividir Feições</translation>
    </message>
    <message>
        <source>Map Tips</source>
        <translation>Dicas do Mapa</translation>
    </message>
    <message>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation>Mostrar informação sobre a feição quando o mouse passar sobre ela</translation>
    </message>
    <message>
        <source>Current map scale</source>
        <translation>Escala atual do mapa</translation>
    </message>
    <message>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation>Mostrar as coordenadas do mapa na posição do mouse. O valor é atualizado conforme move-se o mouse.</translation>
    </message>
    <message>
        <source>Project file is older</source>
        <translation>O arquivo do projeto é mais velho</translation>
    </message>
    <message>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation>&lt;p&gt;Este arquivo do projeto foi salvado por uma versão mais velha do QGIS.</translation>
    </message>
    <message>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation> Ao salvar este arquivo do projeto, o QGIS irá atualizá-lo para a última versão, e possivelmente ele será incompatível com as versões mais velhas do QGIS.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation>&lt;p&gt;Mesmo que os desenvolvedores do QGIS irão tentar manter alguma compatibilidade com as versões anteriores, algumas informações do velho projeto podem ser perdidas.</translation>
    </message>
    <message>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation> Para melhorar a qualidade do QGIS, nós agradeceríamos se você poderia reportar bugs em %3.</translation>
    </message>
    <message>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation> Tenha certeza de incluir o arquivo de projeto antigo, e a versão do QGIS para ajudar-nos a descobrir o erro.</translation>
    </message>
    <message>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation>&lt;p&gt;Para remover esse aviso quando abrir um arquivo de projeto antigo, desmarque a caixa &apos;%5&apos; no menu %4.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation>&lt;p&gt;A versão do arquivo do projeto: %1&lt;br&gt;Versão do QGIS: %2</translation>
    </message>
    <message>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation>&lt;tt&gt;Configurações:Opções:Geral&lt;/tt&gt;</translation>
    </message>
    <message>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Avisar-me quando abrir um arquivo de projeto salvado por uma versão mais velha do QGIS</translation>
    </message>
    <message>
        <source>Resource Location Error</source>
        <translation>Erro de localização do recurso</translation>
    </message>
    <message>
        <source>Error reading icon resources from: 
 %1
 Quitting...</source>
        <translation>Erro ao ler ícone de: 
 %1
 Fechando...</translation>
    </message>
    <message>
        <source>Overview</source>
        <translation>Visão Geral</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation type="unfinished">Legenda</translation>
    </message>
    <message>
        <source>You are using QGIS version %1 built against code revision %2.</source>
        <translation>Você está usando a versao %1 do QGIS, e criada com a revisão de código %2.</translation>
    </message>
    <message>
        <source> This copy of QGIS has been built with PostgreSQL support.</source>
        <translation> Está versão do QGIS foi criada com suporte à PostgreSQL.</translation>
    </message>
    <message>
        <source> This copy of QGIS has been built without PostgreSQL support.</source>
        <translation> Está versão do QGIS foi criada sem suporte à PostgreSQL.</translation>
    </message>
    <message>
        <source>
This binary was compiled against Qt %1,and is currently running against Qt %2</source>
        <translation>
Este binário está compilado com Qt %1, e está rodando sobre o Qt %2</translation>
    </message>
    <message>
        <source>Stop map rendering</source>
        <translation>Parar renderização do mapa</translation>
    </message>
    <message>
        <source></source>
        <translation> </translation>
    </message>
    <message>
        <source>Multiple Instances of QgisApp</source>
        <translation>Múltiplas Instâncias do QgisApp</translation>
    </message>
    <message>
        <source>Multiple instances of Quantum GIS application object detected.
Please contact the developers.
</source>
        <translation>Múltiplas instâncias da aplicação de objeto do Quantum GIS detectadas. Por favor contate os desenvolvedores.</translation>
    </message>
    <message>
        <source>Shift+Ctrl+S</source>
        <comment>Save Project under a new name</comment>
        <translation>Shift+Ctrl+S</translation>
    </message>
    <message>
        <source>&amp;Print Composer</source>
        <translation>Criador de Ma&amp;pas</translation>
    </message>
    <message>
        <source>Ctrl+P</source>
        <comment>











Print Composer</comment>
        <translation type="obsolete">Ctrl+P</translation>
    </message>
    <message>
        <source>Print Composer</source>
        <translation>Criador de Mapas</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Desfazer</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation>Ctrl+Z</translation>
    </message>
    <message>
        <source>Undo the last operation</source>
        <translation>Desfazer a última operação</translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>Cor&amp;tar</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation type="unfinished">Ctrl+X</translation>
    </message>
    <message>
        <source>Cut the current selection&apos;s contents to the clipboard</source>
        <translation>Cortar o conteúdo da seleção atual para a área de tranferência</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;Copiar</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation type="unfinished">Ctrl+C</translation>
    </message>
    <message>
        <source>Copy the current selection&apos;s contents to the clipboard</source>
        <translation>Copiar o conteúdo da seleção atual para a área de transferência</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>Co&amp;lar</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Paste the clipboard&apos;s contents into the current selection</source>
        <translation>Colar o conteúdo da seleção atual para a área de transferência</translation>
    </message>
    <message>
        <source>M</source>
        <comment>











Measure a Line</comment>
        <translation type="obsolete">M</translation>
    </message>
    <message>
        <source>J</source>
        <comment>Measure an Area</comment>
        <translation>J</translation>
    </message>
    <message>
        <source>Zoom to Selection</source>
        <translation>Zoom na seleção</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <comment>











Zoom to Selection</comment>
        <translation type="obsolete">Ctrl+J</translation>
    </message>
    <message>
        <source>Zoom Actual Size</source>
        <translation>Zoom tamanho atual</translation>
    </message>
    <message>
        <source>Zoom to Actual Size</source>
        <translation>Zoom para tamanho atual</translation>
    </message>
    <message>
        <source>Add Vector Layer...</source>
        <translation>Adicionar camada vetorial...</translation>
    </message>
    <message>
        <source>Add Raster Layer...</source>
        <translation>Adicionar camada raster...</translation>
    </message>
    <message>
        <source>Add PostGIS Layer...</source>
        <translation>Adicionar camada PostGIS...</translation>
    </message>
    <message>
        <source>W</source>
        <comment>











Add a Web Mapping Server Layer</comment>
        <translation type="obsolete">W</translation>
    </message>
    <message>
        <source>Add a Web Mapping Server Layer</source>
        <translation>Adicionar uma camada de um Servidor de Mapas</translation>
    </message>
    <message>
        <source>Open Attribute Table</source>
        <translation>Abrir Tabela de Atributos</translation>
    </message>
    <message>
        <source>Save as Shapefile...</source>
        <translation>Salvar como Shapefile...</translation>
    </message>
    <message>
        <source>Save the current layer as a shapefile</source>
        <translation>Salvar a camada atual como Shapefile</translation>
    </message>
    <message>
        <source>Save Selection as Shapefile...</source>
        <translation>Salvar seleção como Shapefile...</translation>
    </message>
    <message>
        <source>Save the selection as a shapefile</source>
        <translation>Salvar a seleção como um Shapefile</translation>
    </message>
    <message>
        <source>Properties...</source>
        <translation>Propriedades...</translation>
    </message>
    <message>
        <source>Set properties of the current layer</source>
        <translation>Setar as propriedade para a camada atual</translation>
    </message>
    <message>
        <source>Add to Overview</source>
        <translation>Adicionar para a Visão Geral</translation>
    </message>
    <message>
        <source>Add All to Overview</source>
        <translation>Adicionar tudo para a Visão Geral</translation>
    </message>
    <message>
        <source>Manage Plugins...</source>
        <translation>Gerenciar Plugins...</translation>
    </message>
    <message>
        <source>Toggle Full Screen Mode</source>
        <translation>Mudar para Modo de Tela Inteira</translation>
    </message>
    <message>
        <source>Custom CRS...</source>
        <translation>CRS Personalizado...</translation>
    </message>
    <message>
        <source>Manage custom coordinate reference systems</source>
        <translation>Gerenciar sistema de referência de coordenadas personalizado</translation>
    </message>
    <message>
        <source>Minimize</source>
        <translation>Minimizar</translation>
    </message>
    <message>
        <source>Ctrl+M</source>
        <comment>











Minimize Window</comment>
        <translation type="obsolete">Ctrl+M</translation>
    </message>
    <message>
        <source>Minimizes the active window to the dock</source>
        <translation>Minimizar a janela ativa para a doca</translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation type="unfinished">Visualizar</translation>
    </message>
    <message>
        <source>Toggles between a predefined size and the window size set by the user</source>
        <translation>Trocar entre um tamanho predefinido para um tamanho de janela ajustado pelo usuário</translation>
    </message>
    <message>
        <source>Bring All to Front</source>
        <translation>Trazer Tudo para Frente</translation>
    </message>
    <message>
        <source>Bring forward all open windows</source>
        <translation>Trazer para frente todas as janelas abertas</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Editar</translation>
    </message>
    <message>
        <source>Panels</source>
        <translation>Painéis</translation>
    </message>
    <message>
        <source>Toolbars</source>
        <translation>Barra de Ferramentas</translation>
    </message>
    <message>
        <source>&amp;Window</source>
        <translation>&amp;Janela</translation>
    </message>
    <message>
        <source>Toggle extents and mouse position display</source>
        <translation>Ativar a visualização da extensão do mapa e a posição do mapa</translation>
    </message>
    <message>
        <source>This icon shows whether on the fly coordinate reference system transformation is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation>Este ícone mostra se a transformação do sistema de coordenadas em tempo real está habilitada ou não. Clique no ícone para abrir a janela de propriedade e alterar este comportamento.</translation>
    </message>
    <message>
        <source>CRS status - Click to open coordinate reference system dialog</source>
        <translation>CRS status - Clique para abrir a janela do sistema de referência de coordenadas</translation>
    </message>
    <message>
        <source>This release candidate includes over 60 bug fixes and enchancements over the QGIS 0.10.0 release. In addition we have added the following new features:</source>
        <translation>Este lançamento inclui mais de 60 correções e melhoramentos em relação à versão 0.10. Além disso, nós adicionamos as seguintes funcionalidades:</translation>
    </message>
    <message>
        <source>Revision of all dialogs for user interface consistancy</source>
        <translation>Revisão de todos os diálogos para a consistência da interface do usuário</translation>
    </message>
    <message>
        <source>Improvements to unique value renderer vector dialog</source>
        <translation>Melhoramentos para renderizar valores únicos de vetores</translation>
    </message>
    <message>
        <source>Symbol previews when defining vector classes</source>
        <translation>Pre-visualização quando definindo classes de vetores</translation>
    </message>
    <message>
        <source>Separation of python support into its own library</source>
        <translation>Separação do suporte à Python em sua própria biblioteca</translation>
    </message>
    <message>
        <source>List view and filter for GRASS toolbox to find tools more quickly</source>
        <translation>Busca na lista de ferramentas do GRASS (para achá-las mais rápido)</translation>
    </message>
    <message>
        <source>List view and filter for Plugin Manager to find plugins more easily</source>
        <translation>Busca na lista do Gerenciador de Plugins (para achá-los mais rápido)</translation>
    </message>
    <message>
        <source>Updated Spatial Reference System definitions</source>
        <translation>Atualização das definições do Sistema de Referência Espacial</translation>
    </message>
    <message>
        <source>QML Style support for rasters and database layers</source>
        <translation>Suporte ao estilo QML para camadas raster e de banco de dados</translation>
    </message>
    <message>
        <source>Choose a file name to save the QGIS project file as</source>
        <translation>Escolha um nome para salvar o projeto do QGIS</translation>
    </message>
    <message>
        <source>Choose a file name to save the map image as</source>
        <translation>Escolha um nome para salvar a imagem do mapa</translation>
    </message>
    <message>
        <source>Start editing failed</source>
        <translation type="unfinished">Falha ao iniciar a edição</translation>
    </message>
    <message>
        <source>Provider cannot be opened for editing</source>
        <translation type="unfinished">Provedor não pode ser aberto para edição</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation type="unfinished">Parar edição</translation>
    </message>
    <message>
        <source>Do you want to save the changes to layer %1?</source>
        <translation>Você quer salvar as mudanças para a camanha %1?</translation>
    </message>
    <message>
        <source>Could not commit changes to layer %1

Errors:  %2
</source>
        <translation>Não foi possível salvar as mudanças na camada %1

Erros:  %2</translation>
    </message>
    <message>
        <source>Problems during roll back</source>
        <translation type="unfinished">Problemas durante retorno</translation>
    </message>
    <message>
        <source>Python Console</source>
        <translation>Terminal Python</translation>
    </message>
    <message>
        <source>There was an error loading a plugin.The following diagnostic information may help the QGIS developers resolve the issue:
%1.</source>
        <translation>Houve um erro ao carregar o plugin. A informação de diagnóstico seguinte pode ser útil para os desenvolvedores do QGIS resolverem esse problema: %1.</translation>
    </message>
    <message>
        <source>Map coordinates for the current view extents</source>
        <translation>Mapear as coordenadas para corrente extensão da visão</translation>
    </message>
    <message>
        <source>Maptips require an active layer</source>
        <translation>Detalhes do Mapa requer uma camada ativada</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation type="unfinished">Ctrl+N</translation>
    </message>
    <message>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation type="unfinished">Ctrl+O</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation type="unfinished">Ctrl+S</translation>
    </message>
    <message>
        <source>Ctrl+P</source>
        <comment>Print Composer</comment>
        <translation type="unfinished">Ctrl+P</translation>
    </message>
    <message>
        <source>M</source>
        <comment>Measure a Line</comment>
        <translation type="unfinished">M</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <comment>Zoom to Selection</comment>
        <translation type="unfinished">Ctrl+J</translation>
    </message>
    <message>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation type="unfinished">Ctrl+B</translation>
    </message>
    <message>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation type="unfinished">N</translation>
    </message>
    <message>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation type="unfinished">V</translation>
    </message>
    <message>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation type="unfinished">R</translation>
    </message>
    <message>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation type="unfinished">D</translation>
    </message>
    <message>
        <source>W</source>
        <comment>Add a Web Mapping Server Layer</comment>
        <translation type="unfinished">W</translation>
    </message>
    <message>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation type="unfinished">+</translation>
    </message>
    <message>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation type="unfinished">-</translation>
    </message>
    <message>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation type="unfinished">S</translation>
    </message>
    <message>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation type="unfinished">H</translation>
    </message>
    <message>
        <source>Ctrl+M</source>
        <comment>Minimize Window</comment>
        <translation type="unfinished">Ctrl+M</translation>
    </message>
    <message>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation type="unfinished">F1</translation>
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
        <translation>Sobre o Quantum GIS</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <source>About</source>
        <translation>Sobre</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Versão</translation>
    </message>
    <message>
        <source>What&apos;s New</source>
        <translation>O que há de Novo</translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>Página do QGIS</translation>
    </message>
    <message>
        <source>Providers</source>
        <translation>Provedores</translation>
    </message>
    <message>
        <source>Developers</source>
        <translation>Desenvolvedores</translation>
    </message>
    <message>
        <source>Sponsors</source>
        <translation>Patrocinadores</translation>
    </message>
    <message>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation type="unfinished">Quantum GIS está sob a licença GNU General Public License</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Nome</translation>
    </message>
    <message>
        <source>Website</source>
        <translation type="obsolete">Página Web</translation>
    </message>
    <message>
        <source>&lt;p&gt;The following have sponsored QGIS by contributing money to fund development and other project costs&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;As seguintes pessoas tem financiado o QGIS contribuindo com dinheiro para financiar desenvolvimento e outros custos do projeto&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Available QGIS Data Provider Plugins</source>
        <translation type="obsolete">Plugins do QGIS disponíveis para provedores de dados</translation>
    </message>
    <message>
        <source>Available Qt Database Plugins</source>
        <translation type="obsolete">Plugins disponíveis de Qt para bases de dados</translation>
    </message>
    <message>
        <source>Available Qt Image Plugins</source>
        <translation type="obsolete">Plugins disponíveis de Qt para imagens</translation>
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
        <source>http://www.gnu.org/licenses</source>
        <translation type="unfinished">http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <source>Join our user mailing list</source>
        <translation>Junte-se à nossa lista de email dos usuários</translation>
    </message>
</context>
<context>
    <name>QgsAddAttrDialogBase</name>
    <message>
        <source>Add Attribute</source>
        <translation>Adicionar atributo</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Nome:</translation>
    </message>
    <message>
        <source>Type:</source>
        <translation>Tipo:</translation>
    </message>
</context>
<context>
    <name>QgsApplication</name>
    <message>
        <source>Exception</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation>Seleciona uma ação</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>Esta lista contém todas ações que podem ser definidas para a camada atual. Adicione ações entrando com detalhes nos controles abaixo e então, pressione o botão de inserção. Ações podem ser editadas aqui através de um duplo clique.</translation>
    </message>
    <message>
        <source>Move up</source>
        <translation>Mover para cima</translation>
    </message>
    <message>
        <source>Move the selected action up</source>
        <translation>Mover a ação selecionada para cima</translation>
    </message>
    <message>
        <source>Move down</source>
        <translation>Mover para baixo</translation>
    </message>
    <message>
        <source>Move the selected action down</source>
        <translation>Mover a ação selecionada para baixo</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Remover</translation>
    </message>
    <message>
        <source>Remove the selected action</source>
        <translation>Remover a ação selecionada</translation>
    </message>
    <message>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>Entre com o nome da ação aqui. O nome deve ser único (QGIS fará único se necessário).</translation>
    </message>
    <message>
        <source>Enter the action name here</source>
        <translation>Entre com o nome da ação aqui</translation>
    </message>
    <message>
        <source>Enter the action command here</source>
        <translation>Entre com o comando de ação aqui</translation>
    </message>
    <message>
        <source>Insert action</source>
        <translation>Inserir ação</translation>
    </message>
    <message>
        <source>Inserts the action into the list above</source>
        <translation>Insere a ação na lista acima</translation>
    </message>
    <message>
        <source>Update action</source>
        <translation>Atualiza ação</translation>
    </message>
    <message>
        <source>Update the selected action</source>
        <translation>Atualiza a ação selecionada</translation>
    </message>
    <message>
        <source>Insert field</source>
        <translation>Inserir campo</translation>
    </message>
    <message>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation>Insere o campo selecionado na ação, precedido por um %</translation>
    </message>
    <message>
        <source>The valid attribute names for this layer</source>
        <translation>Os nomes de atributos válidos para esta camada</translation>
    </message>
    <message>
        <source>Capture output</source>
        <translation>Capture emissor</translation>
    </message>
    <message>
        <source>Captures any output from the action</source>
        <translation>Captura qualquer saída da ação</translation>
    </message>
    <message>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation>Captura a saída padrão ou o erro gerado pela ação, e mostra isso em uma caixa de diálogo</translation>
    </message>
    <message>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation>Entre a ação aqui. Isto pode ser qualquer programa, script ou comando disponível no seu sistema. Quando a ação é chamada, qualquer conjunto de caracteres que começe por % e que tenha o nome de um campo será substituído pelo valor do campo. Os caracteres especiais %% serão substituídos pelo valor do campo selecionado. Aspas duplas marcam um grupo de texto em um simples argumento para o programa, script ou comando. Aspas duplas serão ignoradas se precedidas por uma contra-barra (\&quot;\&quot;)</translation>
    </message>
    <message>
        <source>Attribute Actions</source>
        <translation>Ações do Atributo</translation>
    </message>
    <message>
        <source>Action properties</source>
        <translation>Propriedades da Ação</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <source>Action</source>
        <translation type="unfinished">Ação</translation>
    </message>
    <message>
        <source>Browse for action</source>
        <translation>Escolha uma ação</translation>
    </message>
    <message>
        <source>Click to browse for an action</source>
        <translation>Clique para escolher uma ação</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Capture</source>
        <translation type="unfinished">Capturar</translation>
    </message>
    <message>
        <source>Clicking the button will let you select an application to use as the action</source>
        <translation>Clicando no botão irá permitir que você selecione uma aplicação para ser usada na ação</translation>
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
        <translation>.(txt)</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Select a file</source>
        <translation>Selecione um arquivo</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <source>Enter Attribute Values</source>
        <translation>Entre com os valores de atributos</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <source>Run action</source>
        <translation>Rodar ação</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <source>Attribute Table</source>
        <translation>Tabela de atributos </translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <source>Invert selection</source>
        <translation>Inverter seleção</translation>
    </message>
    <message>
        <source>Ctrl+T</source>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <source>Move selected to top</source>
        <translation>Mover selecionado para cima</translation>
    </message>
    <message>
        <source>Remove selection</source>
        <translation>Remover seleção</translation>
    </message>
    <message>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Copiar linhas selecionadas para a área de transferência (Ctrl+C)</translation>
    </message>
    <message>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Copia a linha selecionada para a área de transferência</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>in</source>
        <translation>no</translation>
    </message>
    <message>
        <source>Search</source>
        <translation>Procurar</translation>
    </message>
    <message>
        <source>Adva&amp;nced...</source>
        <translation>Ava&amp;nçado...</translation>
    </message>
    <message>
        <source>Alt+N</source>
        <translation>Alt+N</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows</source>
        <translation>Fazer zoom no mapa usando as linhas selecionadas</translation>
    </message>
    <message>
        <source>Search for</source>
        <translation>Procurar por</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows (Ctrl-J)</source>
        <translation>Fazer zoom no mapa usando as linhas selecionadas (Ctrl-J)</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <translation type="unfinished">Ctrl+J</translation>
    </message>
    <message>
        <source>Toggle editing mode</source>
        <translation>Ativar modo de edição</translation>
    </message>
    <message>
        <source>Click to toggle table editing</source>
        <translation>Clique para ativar edição da tabela</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <source>select</source>
        <translation>seleciona</translation>
    </message>
    <message>
        <source>select and bring to top</source>
        <translation>seleciona e coloca no topo</translation>
    </message>
    <message>
        <source>show only matching</source>
        <translation>mostrar apenas correspondentes</translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation>Procurar string de análise de erro</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation>Pesquisar resultados</translation>
    </message>
    <message>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>Você forneceu uma string vazia de pesquisa.</translation>
    </message>
    <message>
        <source>Error during search</source>
        <translation>Erro durante a pesquisa</translation>
    </message>
    <message>
        <source>No matching features found.</source>
        <translation>Nehuma característica produrada encontrada.</translation>
    </message>
    <message>
        <source>Attribute table - </source>
        <translation type="unfinished">Tabela de atributos - </translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>File</source>
        <translation type="unfinished">Arquivo</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="unfinished">Fechar</translation>
    </message>
    <message>
        <source>Ctrl+W</source>
        <translation>Ctrl+W</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="unfinished">Editar</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Desfazer</translation>
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
        <translation type="unfinished">Ctrl+X</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;Copiar</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation type="unfinished">Ctrl+C</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>&amp;Colar</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Apagar</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <source>Zoom to Selection</source>
        <translation>Zoom para a seleção</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <translation type="unfinished">Ctrl+J</translation>
    </message>
    <message>
        <source>Toggle Editing</source>
        <translation>Ativar Edição</translation>
    </message>
    <message>
        <source>Table</source>
        <translation type="unfinished">Tabela</translation>
    </message>
    <message>
        <source>Move to Top</source>
        <translation>Mover para o topo</translation>
    </message>
    <message>
        <source>Invert</source>
        <translation>Inverter</translation>
    </message>
    <message>
        <source>bad_alloc exception</source>
        <translation>Exceção de má alocação</translation>
    </message>
    <message>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation>Preencimento da tabela de atributos foi suspenso devido ao fato de não haver mais memória virtual livre</translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <source>Really Delete?</source>
        <translation>Quer mesmo excluir?</translation>
    </message>
    <message>
        <source>Are you sure you want to delete the </source>
        <translation>Certeza em querer excluir o </translation>
    </message>
    <message>
        <source> bookmark?</source>
        <translation> favorito?</translation>
    </message>
    <message>
        <source>Error deleting bookmark</source>
        <translation>Erro ao excluir favorito</translation>
    </message>
    <message>
        <source>Failed to delete the </source>
        <translation>Falha ao excluir o </translation>
    </message>
    <message>
        <source> bookmark from the database. The database said:
</source>
        <translation> favorito da base de dados. O base de dados disse:</translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation>&amp;Excluir</translation>
    </message>
    <message>
        <source>&amp;Zoom to</source>
        <translation>&amp;Zoom para</translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <source>Geospatial Bookmarks</source>
        <translation>Favoritos Geoespaciais</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <source>Project</source>
        <translation>Projeto</translation>
    </message>
    <message>
        <source>Extent</source>
        <translation>Extensão</translation>
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
        <translation type="obsolete"> para ler/escrever</translation>
    </message>
    <message>
        <source>Error in Print</source>
        <translation type="obsolete">Erro em imprimir</translation>
    </message>
    <message>
        <source>Cannot seek</source>
        <translation type="obsolete">Impossível procurar</translation>
    </message>
    <message>
        <source>Cannot overwrite BoundingBox</source>
        <translation type="obsolete">Impossível sobrescrever CaixaLimite</translation>
    </message>
    <message>
        <source>Cannot find BoundingBox</source>
        <translation type="obsolete">Impossível procurar CaixaLimite</translation>
    </message>
    <message>
        <source>Cannot overwrite translate</source>
        <translation type="obsolete">Impossível sobrescrever tradução</translation>
    </message>
    <message>
        <source>Cannot find translate</source>
        <translation type="obsolete">Impossível achar tradução</translation>
    </message>
    <message>
        <source>File IO Error</source>
        <translation type="obsolete">Erro de IO no arquivo</translation>
    </message>
    <message>
        <source>Paper does not match</source>
        <translation type="obsolete">Papel sem correspondência</translation>
    </message>
    <message>
        <source>The selected paper size does not match the composition size</source>
        <translation type="obsolete">O papel selecionado não tem correspondência com o tamanho da composição</translation>
    </message>
    <message>
        <source>Big image</source>
        <translation>Imagem grande</translation>
    </message>
    <message>
        <source>To create image </source>
        <translation>Para criar uma imagem</translation>
    </message>
    <message>
        <source> requires circa </source>
        <translation> requer cerca de </translation>
    </message>
    <message>
        <source> MB of memory</source>
        <translation> MB de memória</translation>
    </message>
    <message>
        <source>QGIS - print composer</source>
        <translation>QGIS - compositor de impressão</translation>
    </message>
    <message>
        <source>Map 1</source>
        <translation>Mapa 1</translation>
    </message>
    <message>
        <source>Couldn&apos;t open </source>
        <translation type="obsolete">Impossível abrir</translation>
    </message>
    <message>
        <source>format</source>
        <translation>formatar</translation>
    </message>
    <message>
        <source>SVG warning</source>
        <translation>Advertência SVG</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Não mostra esta mensagem novamente</translation>
    </message>
    <message>
        <source>SVG Format</source>
        <translation>Formato SVG</translation>
    </message>
    <message>
        <source>Move Content</source>
        <translation>Mover conteúdo</translation>
    </message>
    <message>
        <source>Move item content</source>
        <translation>Mover item do conteúdo</translation>
    </message>
    <message>
        <source>&amp;Group</source>
        <translation>A&amp;grupar</translation>
    </message>
    <message>
        <source>Group items</source>
        <translation>Agrupar items</translation>
    </message>
    <message>
        <source>&amp;Ungroup</source>
        <translation>&amp;Desagrupar</translation>
    </message>
    <message>
        <source>Ungroup items</source>
        <translation>Desagrupar items</translation>
    </message>
    <message>
        <source>Raise</source>
        <translation>Elevar</translation>
    </message>
    <message>
        <source>Raise selected items</source>
        <translation>Elevar items selecionados</translation>
    </message>
    <message>
        <source>Lower</source>
        <translation>Abaixar</translation>
    </message>
    <message>
        <source>Lower selected items</source>
        <translation>Abaixar items selecionados</translation>
    </message>
    <message>
        <source>Bring to Front</source>
        <translation>Trazer para frente</translation>
    </message>
    <message>
        <source>Move selected items to top</source>
        <translation>Mover items selecionados para o topo</translation>
    </message>
    <message>
        <source>Send to Back</source>
        <translation>Enviar para trás</translation>
    </message>
    <message>
        <source>Move selected items to bottom</source>
        <translation>Mover items selecionados para o fundo</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>File</source>
        <translation type="unfinished">Arquivo</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="unfinished">Fechar</translation>
    </message>
    <message>
        <source>Ctrl+W</source>
        <translation>Ctrl+W</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="unfinished">Editar</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Desfazer</translation>
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
        <translation type="unfinished">Ctrl+X</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;Copiar</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation type="unfinished">Ctrl+C</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>&amp;Colar</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Apagar</translation>
    </message>
    <message>
        <source>View</source>
        <translation>Visão</translation>
    </message>
    <message>
        <source>Layout</source>
        <translation>Modelo</translation>
    </message>
    <message>
        <source>Choose a file name to save the map image as</source>
        <translation>Escolha um nome para salvar a imagem do mapa como</translation>
    </message>
    <message>
        <source>Choose a file name to save the map as</source>
        <translation>Escolha um nome para salvar o mapa como</translation>
    </message>
    <message>
        <source>Project contains WMS layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Some WMS servers (e.g. UMN mapserver) have a limit for the WIDTH and HEIGHT parameter. Printing layers from such servers may exceed this limit. If this is the case, the WMS layer will not be printed</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <source>General</source>
        <translation>Geral</translation>
    </message>
    <message>
        <source>Composition</source>
        <translation>Composição</translation>
    </message>
    <message>
        <source>Item</source>
        <translation>Item</translation>
    </message>
    <message>
        <source>Save Template &amp;As...</source>
        <translation type="obsolete">Salvar modelo &amp;como</translation>
    </message>
    <message>
        <source>&amp;Print...</source>
        <translation>&amp;Imprimir</translation>
    </message>
    <message>
        <source>Add new map</source>
        <translation>Adicionar novo mapa</translation>
    </message>
    <message>
        <source>Add new label</source>
        <translation>Adicionar novo rótulo</translation>
    </message>
    <message>
        <source>Add new vect legend</source>
        <translation>Adicionar nova legenda vetorial</translation>
    </message>
    <message>
        <source>Select/Move item</source>
        <translation>Selecionar/mover item</translation>
    </message>
    <message>
        <source>Add new scalebar</source>
        <translation>Adicionar nova barra de escala</translation>
    </message>
    <message>
        <source>Refresh view</source>
        <translation>Atualizar visão</translation>
    </message>
    <message>
        <source>MainWindow</source>
        <translation>JanelaPrincipal</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Aproximar</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Afastar</translation>
    </message>
    <message>
        <source>Add Image</source>
        <translation>Adicionar imagem</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ajuda</translation>
    </message>
    <message>
        <source>&amp;Open Template...</source>
        <translation type="obsolete">&amp;Abrir Modelo...</translation>
    </message>
    <message>
        <source>Zoom Full</source>
        <translation type="unfinished">Ver tudo</translation>
    </message>
    <message>
        <source>Add Map</source>
        <translation>Adicionar mapa</translation>
    </message>
    <message>
        <source>Add Label</source>
        <translation>Adicionar rótulo</translation>
    </message>
    <message>
        <source>Add Vector Legend</source>
        <translation>Adicionar legenda para vetor</translation>
    </message>
    <message>
        <source>Move Item</source>
        <translation>Mover item</translation>
    </message>
    <message>
        <source>Export as Image...</source>
        <translation>Exportar como imagem...</translation>
    </message>
    <message>
        <source>Export as SVG...</source>
        <translation>Exportar como SVG...</translation>
    </message>
    <message>
        <source>Add Scalebar</source>
        <translation>Adicionar barra de escala</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation type="unfinished">Atualizar</translation>
    </message>
</context>
<context>
    <name>QgsComposerItemWidgetBase</name>
    <message>
        <source>Form</source>
        <translation>Formulário</translation>
    </message>
    <message>
        <source>Composer item properties</source>
        <translation>Propriedades do item no Compositor</translation>
    </message>
    <message>
        <source>Color:</source>
        <translation>Cor:</translation>
    </message>
    <message>
        <source>Frame...</source>
        <translation>Forma...</translation>
    </message>
    <message>
        <source>Background...</source>
        <translation>Fundo...</translation>
    </message>
    <message>
        <source>Opacity:</source>
        <translation>Opacidade</translation>
    </message>
    <message>
        <source>Outline width: </source>
        <translation>Espessura da borda: </translation>
    </message>
    <message>
        <source>Frame</source>
        <translation type="unfinished">Moldura</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelWidgetBase</name>
    <message>
        <source>Label Options</source>
        <translation type="unfinished">Opções de rótulo</translation>
    </message>
    <message>
        <source>Font</source>
        <translation type="unfinished">Fonte</translation>
    </message>
    <message>
        <source>Margin (mm):</source>
        <translation>Margem</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendItemDialogBase</name>
    <message>
        <source>Legend item properties</source>
        <translation>Propriedade da legenda do item</translation>
    </message>
    <message>
        <source>Item text:</source>
        <translation>Texto do item:</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendWidgetBase</name>
    <message>
        <source>Barscale Options</source>
        <translation type="unfinished">Opções da barra de escala</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="unfinished">Geral</translation>
    </message>
    <message>
        <source>Title:</source>
        <translation>Título:</translation>
    </message>
    <message>
        <source>Font:</source>
        <translation>Fonte:</translation>
    </message>
    <message>
        <source>Title...</source>
        <translation>Título...</translation>
    </message>
    <message>
        <source>Layer...</source>
        <translation>Camada...</translation>
    </message>
    <message>
        <source>Item...</source>
        <translation>Item...</translation>
    </message>
    <message>
        <source>Symbol width: </source>
        <translation>Espessura do símbolo: </translation>
    </message>
    <message>
        <source>Symbol height:</source>
        <translation>Altura do símbolo:</translation>
    </message>
    <message>
        <source>Layer space: </source>
        <translation>Espaço da camada: </translation>
    </message>
    <message>
        <source>Symbol space:</source>
        <translation>Espaço do símbolo:</translation>
    </message>
    <message>
        <source>Icon label space:</source>
        <translation>Espaço do rótulo do ícone:</translation>
    </message>
    <message>
        <source>Box space:</source>
        <translation>Espaço da caixa:</translation>
    </message>
    <message>
        <source>Legend items</source>
        <translation>Legenda dos itens</translation>
    </message>
    <message>
        <source>down</source>
        <translation>para baixo</translation>
    </message>
    <message>
        <source>up</source>
        <translation>para cima</translation>
    </message>
    <message>
        <source>remove</source>
        <translation>remover</translation>
    </message>
    <message>
        <source>edit...</source>
        <translation>editar...</translation>
    </message>
    <message>
        <source>update</source>
        <translation>atualizar</translation>
    </message>
    <message>
        <source>update all</source>
        <translation>atualizar tudo</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <source>Map</source>
        <translation type="unfinished">Mapa</translation>
    </message>
    <message>
        <source>Map will be printed here</source>
        <translation>O mapa será impresso aqui</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidget</name>
    <message>
        <source>Cache</source>
        <translation type="unfinished">Cache</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation type="unfinished">Retângulo</translation>
    </message>
    <message>
        <source>Render</source>
        <translation type="unfinished">Desenhar</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidgetBase</name>
    <message>
        <source>Map options</source>
        <translation type="unfinished">Opções de mapa</translation>
    </message>
    <message>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation type="unfinished">&lt;b&gt;Mapa&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Width</source>
        <translation type="unfinished">Largura</translation>
    </message>
    <message>
        <source>Height</source>
        <translation type="unfinished">Altura</translation>
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
        <translation>Extensão do mapa</translation>
    </message>
    <message>
        <source>X min:</source>
        <translation>X mínimo:</translation>
    </message>
    <message>
        <source>Y min:</source>
        <translation>Y mínimo:</translation>
    </message>
    <message>
        <source>X max:</source>
        <translation>X máximo:</translation>
    </message>
    <message>
        <source>Y max:</source>
        <translation>Y máximo:</translation>
    </message>
    <message>
        <source>set to map canvas extent</source>
        <translation>Setar a extensão da área</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation type="unfinished">Pré-visualização</translation>
    </message>
    <message>
        <source>Update preview</source>
        <translation>Atualizar pré-visualização</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidget</name>
    <message>
        <source>Select svg or image file</source>
        <translation>Selecionar SVG ou imagem</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidgetBase</name>
    <message>
        <source>Picture Options</source>
        <translation type="unfinished">Opções de figura</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Ver...</translation>
    </message>
    <message>
        <source>Width:</source>
        <translation type="unfinished">Largura:</translation>
    </message>
    <message>
        <source>Height:</source>
        <translation>Altura:</translation>
    </message>
    <message>
        <source>Rotation:</source>
        <translation>Rotação:</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBar</name>
    <message>
        <source>Single Box</source>
        <translation>Caixa simples</translation>
    </message>
    <message>
        <source>Double Box</source>
        <translation>Caixa dupla</translation>
    </message>
    <message>
        <source>Line Ticks Middle</source>
        <translation>Linhas tracejadas do meio</translation>
    </message>
    <message>
        <source>Line Ticks Down</source>
        <translation>Linhas tracejadas embaixo</translation>
    </message>
    <message>
        <source>Line Ticks Up</source>
        <translation>Linhas tracejadas em cima</translation>
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
        <translation>Caixa simples</translation>
    </message>
    <message>
        <source>Double Box</source>
        <translation>Caixa dupla</translation>
    </message>
    <message>
        <source>Line Ticks Middle</source>
        <translation>Linhas tracejadas do meio</translation>
    </message>
    <message>
        <source>Line Ticks Down</source>
        <translation>Linhas tracejadas embaixo</translation>
    </message>
    <message>
        <source>Line Ticks Up</source>
        <translation>Linhas tracejadas em cima</translation>
    </message>
    <message>
        <source>Numeric</source>
        <translation>Numérico</translation>
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
        <translation type="unfinished">Opções da barra de escala</translation>
    </message>
    <message>
        <source>Segment size (map units):</source>
        <translation>Tamanho do segmento (em unidades do mapa):</translation>
    </message>
    <message>
        <source>Map units per bar unit:</source>
        <translation>Unidades do mapa por unidades da barra:</translation>
    </message>
    <message>
        <source>Number of segments:</source>
        <translation>Número de segmentos:</translation>
    </message>
    <message>
        <source>Segments left:</source>
        <translation>Segmentos à esquerda:</translation>
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
        <translation>Espessura da linha:</translation>
    </message>
    <message>
        <source>Label space:</source>
        <translation>Espaço do rótulo:</translation>
    </message>
    <message>
        <source>Box space:</source>
        <translation>Espaço da caixa</translation>
    </message>
    <message>
        <source>Unit label:</source>
        <translation>Rótulo da unidade:</translation>
    </message>
    <message>
        <source>Font...</source>
        <translation>Fonte...</translation>
    </message>
    <message>
        <source>Color...</source>
        <translation>Cor...</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <source>Vector Legend Options</source>
        <translation>Opções da legenda de vetores</translation>
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
        <translation>Fonte</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Caixa</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Pré-visualização</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation type="unfinished">Camadas</translation>
    </message>
    <message>
        <source>Group</source>
        <translation type="unfinished">Grupo</translation>
    </message>
    <message>
        <source>ID</source>
        <translation type="unfinished">ID</translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <source>Composition</source>
        <translation>Composição</translation>
    </message>
    <message>
        <source>Paper</source>
        <translation>Papel</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Tamanho</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Unidades</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Largura</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Orientação</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidget</name>
    <message>
        <source>Landscape</source>
        <translation type="unfinished">Paisagem</translation>
    </message>
    <message>
        <source>Portrait</source>
        <translation type="unfinished">Retrato</translation>
    </message>
    <message>
        <source>Custom</source>
        <translation type="unfinished">Personalizado</translation>
    </message>
    <message>
        <source>A5 (148x210 mm)</source>
        <translation type="unfinished">A5 (148x210 mm)</translation>
    </message>
    <message>
        <source>A4 (210x297 mm)</source>
        <translation type="unfinished">A4 (210x297 mm)</translation>
    </message>
    <message>
        <source>A3 (297x420 mm)</source>
        <translation type="unfinished">A3 (297x420 mm)</translation>
    </message>
    <message>
        <source>A2 (420x594 mm)</source>
        <translation type="unfinished">A2 (420x594 mm)</translation>
    </message>
    <message>
        <source>A1 (594x841 mm)</source>
        <translation type="unfinished">A1 (594x841 mm)</translation>
    </message>
    <message>
        <source>A0 (841x1189 mm)</source>
        <translation type="unfinished">A1 (594x841 mm)</translation>
    </message>
    <message>
        <source>B5 (176 x 250 mm)</source>
        <translation type="unfinished">B5 (176 x 250 mm)</translation>
    </message>
    <message>
        <source>B4 (250 x 353 mm)</source>
        <translation type="unfinished">B4 (250 x 353 mm)</translation>
    </message>
    <message>
        <source>B3 (353 x 500 mm)</source>
        <translation type="unfinished">B3 (353 x 500 mm)</translation>
    </message>
    <message>
        <source>B2 (500 x 707 mm)</source>
        <translation type="unfinished">B2 (500 x 707 mm)</translation>
    </message>
    <message>
        <source>B1 (707 x 1000 mm)</source>
        <translation type="unfinished">B1 (707 x 1000 mm)</translation>
    </message>
    <message>
        <source>B0 (1000 x 1414 mm)</source>
        <translation type="unfinished">B0 (1000 x 1414 mm)</translation>
    </message>
    <message>
        <source>Letter (8.5x11 inches)</source>
        <translation type="unfinished">Carta (8.5x11 polegadas)</translation>
    </message>
    <message>
        <source>Legal (8.5x14 inches)</source>
        <translation type="unfinished">Legal (8.5x14 polegadas)</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidgetBase</name>
    <message>
        <source>Composition</source>
        <translation type="unfinished">Composição</translation>
    </message>
    <message>
        <source>Paper</source>
        <translation type="unfinished">Papel</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation type="unfinished">Orientação</translation>
    </message>
    <message>
        <source>Height</source>
        <translation type="unfinished">Altura</translation>
    </message>
    <message>
        <source>Width</source>
        <translation type="unfinished">Largura</translation>
    </message>
    <message>
        <source>Units</source>
        <translation type="unfinished">Unidades</translation>
    </message>
    <message>
        <source>Size</source>
        <translation type="unfinished">Tamanho</translation>
    </message>
    <message>
        <source>Print quality (dpi)</source>
        <translation>Qualidade da Impressão (em dpi)</translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <source>Continuous color</source>
        <translation>Cor Contínua</translation>
    </message>
    <message>
        <source>Maximum Value:</source>
        <translation>Valor Máximo:</translation>
    </message>
    <message>
        <source>Outline Width:</source>
        <translation>Espessura da borda:</translation>
    </message>
    <message>
        <source>Minimum Value:</source>
        <translation>Valor Mínimo:</translation>
    </message>
    <message>
        <source>Classification Field:</source>
        <translation>Campo de classificação:</translation>
    </message>
    <message>
        <source>Draw polygon outline</source>
        <translation>Desenha o contorno do polígono</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <source>Failed</source>
        <translation>Falhou</translation>
    </message>
    <message>
        <source>transform of</source>
        <translation>transformar de</translation>
    </message>
    <message>
        <source>with error: </source>
        <translation>com erro:</translation>
    </message>
    <message>
        <source>The source spatial reference system (CRS) is not valid. </source>
        <translation>O sistema de referência espacial de origem (CRS) não é válido. </translation>
    </message>
    <message>
        <source>The coordinates can not be reprojected. The CRS is: </source>
        <translation>As coordenadas não puderam ser reprojetadas. O CRS é: </translation>
    </message>
    <message>
        <source>The destination spatial reference system (CRS) is not valid. </source>
        <translation>O sistema de referência espacial de destino (CRS) não é válido. </translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>Inferior Esquerdo</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Superior Esquerdo</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Superior Direito</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Inferior Direito</translation>
    </message>
    <message>
        <source>&amp;Copyright Label</source>
        <translation>&amp;Etiqueta de Copyright</translation>
    </message>
    <message>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>Cria uma etiqueta de copyright que será mostrada na tela do mapa.</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Decorações</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <source>Copyright Label Plugin</source>
        <translation>Plugin do rótulo de Copyright</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Posicionamento</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Inferior Esquerdo</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Superior Esquerdo</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Inferior Direito</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Superior Direito</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Orientação</translation>
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
        <translation>Habilitar Rótulo de Copyright</translation>
    </message>
    <message>
        <source>Color</source>
        <translation type="unfinished">Cor</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Entre a sua marca registrada abaixo. Este plugin suporta html básico para formatar o rótulo. Por exemplo:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Bold text &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Italics &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(note: &amp;amp;copy; gives a copyright symbol)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&#xa9; QGIS 2008&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;© QGIS 2008&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
        <translation>Excluir definição de projeção?</translation>
    </message>
    <message>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Excluir a definição de projeção é irreversível. Quer mesmo excluir?</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation>Abortar</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Novo</translation>
    </message>
    <message>
        <source>QGIS Custom Projection</source>
        <translation>Projeção personalizada do QGIS</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Esta definição de projeção não é válida. Corrija antes de pressionar salvar.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Esta definição de projeção proj4 não é válida. Por Favor forneça um nome de projeção antes de pressionar salvar.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Esta definição de projeção proj4 não é válida. Por favor adicione os parâmetros antes de pressionar salvar.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Esta definição de projeção proj4 não é válida. Por favor adicione uma cláusula proj= antes de pressionar salvar</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Esta definição de projeção proj4 não é válida.</translation>
    </message>
    <message>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>N e E devem estar em formato decimal.</translation>
    </message>
    <message>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Erro interno (projeção fonte inválida?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
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
        <translation>&gt;</translation>
    </message>
    <message>
        <source>Define</source>
        <translation>Definir</translation>
    </message>
    <message>
        <source>Test</source>
        <translation>Testar</translation>
    </message>
    <message>
        <source>Geographic / WGS84</source>
        <translation>Geográfica / WGS84</translation>
    </message>
    <message>
        <source>Calculate</source>
        <translation>Calcular</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation type="unfinished">Parâmetros</translation>
    </message>
    <message>
        <source>*</source>
        <translation>*</translation>
    </message>
    <message>
        <source>S</source>
        <translation type="unfinished">S</translation>
    </message>
    <message>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <source>North</source>
        <translation type="unfinished">Norte</translation>
    </message>
    <message>
        <source>East</source>
        <translation type="unfinished">Leste</translation>
    </message>
    <message>
        <source>Custom Coordinate Reference System Definition</source>
        <translation>Definição de um Sistema de Referência de Coordenadas customizado</translation>
    </message>
    <message>
        <source>You can define your own custom Coordinate Reference System (CRS) here. The definition must conform to the proj4 format for specifying a CRS.</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Enter your copyright label below. This plugin supports basic html markup tags for formatting the label. For example:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Bold text &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Italics &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(note: &amp;amp;copy; gives a copyright symbol)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Use the text boxes below to test the CRS definition you are creating. Enter a coordinate where both the lat/long and the transformed result are known (for example by reading off a map). Then press the calculate button to see if the CRS definition you are creating is accurate.</source>
        <translation>Use a caixa de texto abaixo para testar a definição de CRS que você está criando. Entre as coordenadas onde ambos lat/long e o resultado da transformada são conhecidos (por exemplo, lendo do seu mapa). E então pressione o botão calcular para ser se a definição do CRS foi criada com precisão.</translation>
    </message>
    <message>
        <source>Destination CRS        </source>
        <translation>CRS de destino         </translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Tem certeza que deseja remover o </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation>conexão e todos os ajustes associados ?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Confirme a exclusão</translation>
    </message>
    <message>
        <source>Select Table</source>
        <translation>Selecionar Tabela</translation>
    </message>
    <message>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Você deve selecionar uma tabela para poder adicionar uma Camada.</translation>
    </message>
    <message>
        <source>Password for </source>
        <translation>Senha para</translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation>Por favor, entre com sua senha:</translation>
    </message>
    <message>
        <source>Connection failed</source>
        <translation>A conexão Falhou</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>Conexão para %1 em %2 falhou. A base de dados pode estar fora do ar ou suas configurações estão incorretas.%3Cheque seu nome de usuário e senha e tente novamente.%4A base de dados disse:%5%6</translation>
    </message>
    <message>
        <source>Wildcard</source>
        <translation>Coringa</translation>
    </message>
    <message>
        <source>RegExp</source>
        <translation>Expressão Regular</translation>
    </message>
    <message>
        <source>All</source>
        <translation type="unfinished">Tudo</translation>
    </message>
    <message>
        <source>Schema</source>
        <translation type="unfinished">Esquema</translation>
    </message>
    <message>
        <source>Table</source>
        <translation type="unfinished">Tabela</translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation>Coluna de geometria</translation>
    </message>
    <message>
        <source>Accessible tables could not be determined</source>
        <translation>Tabelas acessáveis não puderam ser determinadas</translation>
    </message>
    <message>
        <source>Database connection was successful, but the accessible tables could not be determined.

The error message from the database was:
%1
</source>
        <translation>A conexão com o banco de dados ocorreu, mas tabelas acessáveis não puderam ser determinadas. 

A mensagem de erro do banco de dados foi: 
%1</translation>
    </message>
    <message>
        <source>No accessible tables found</source>
        <translation>Não foram encontradas tabelas acessíveis</translation>
    </message>
    <message>
        <source>Database connection was successful, but no accessible tables were found.

Please verify that you have SELECT privilege on a table carrying PostGIS
geometry.</source>
        <translation>A conexão com o banco de dados ocorreu, mas tabelas acessíveis não puderam ser determinadas. 

Por favor verifique se você tem privilégios para usar o SELECT na tabela que contém geometrias do PostGIS.</translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelectBase</name>
    <message>
        <source>Add PostGIS Table(s)</source>
        <translation>Adicionar tabela(s) PostGIS</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Adicionar</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ajuda</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Conectar</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Novo</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Deletar</translation>
    </message>
    <message>
        <source>PostgreSQL Connections</source>
        <translation>Conexões PostgreSQL </translation>
    </message>
    <message>
        <source>Search:</source>
        <translation>Busca:</translation>
    </message>
    <message>
        <source>Search mode:</source>
        <translation>Modo de Busca:</translation>
    </message>
    <message>
        <source>Search in columns:</source>
        <translation>Procurar nas colunas:</translation>
    </message>
    <message>
        <source>Search options...</source>
        <translation>Opções de busca...</translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <source>Schema</source>
        <translation type="unfinished">Esquema</translation>
    </message>
    <message>
        <source>Table</source>
        <translation type="unfinished">Tabela</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="unfinished">Tipo</translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation>Coluna de geometria</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation type="unfinished">Sql</translation>
    </message>
    <message>
        <source>Point</source>
        <translation type="unfinished">Ponto</translation>
    </message>
    <message>
        <source>Multipoint</source>
        <translation>Multiponto</translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="unfinished">Linha</translation>
    </message>
    <message>
        <source>Multiline</source>
        <translation>Multilinha</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation type="unfinished">Polígono</translation>
    </message>
    <message>
        <source>Multipolygon</source>
        <translation>Multipolígono</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <source>Delete Attributes</source>
        <translation>Excluir atributos</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <source>&amp;Add Delimited Text Layer</source>
        <translation>&amp;Adicionar uma camada a partir de um texto delimitado</translation>
    </message>
    <message>
        <source>Add a delimited text file as a map layer. </source>
        <translation>Adiciona um texto delimitado como camada no mapa. </translation>
    </message>
    <message>
        <source>The file must have a header row containing the field names. </source>
        <translation>O arquivo deve ter cabeçalho de linha contendo os nomes dos campos. </translation>
    </message>
    <message>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>Campos X e Y são requeridos e devem conter coordenadas em unidades decimais.</translation>
    </message>
    <message>
        <source>&amp;Delimited text</source>
        <translation>&amp;Texto delimitado</translation>
    </message>
    <message>
        <source>DelimitedTextLayer</source>
        <translation>CamadaTextoDelimitado</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <source>No layer name</source>
        <translation>Sem nome de camada</translation>
    </message>
    <message>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Por favor, entre com o nome da camada antes de adicioná-la ao mapa</translation>
    </message>
    <message>
        <source>No delimiter</source>
        <translation>Sem delimitador</translation>
    </message>
    <message>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Por favor, especifique um delimitador antes de analisar o arquivo</translation>
    </message>
    <message>
        <source>Choose a delimited text file to open</source>
        <translation>Escolha um arquivos de texto delimitado para abrir</translation>
    </message>
    <message>
        <source>Parse</source>
        <translation>Analisar</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="unfinished">Descrição</translation>
    </message>
    <message>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation>Selecione um arquivo de texto delimitado, contendo um cabeçalho com uma ou mais linhas de coordenadas x e y, para serem usados como uma camada de pontos, e este plugin fará o serviço para você!</translation>
    </message>
    <message>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation>Use a caixa de entrada para especificar o nome da nova camada vetorial. Use a caixa do delimitador para especificar qual delimitador é usado em seu arquivo (p.ex. espaço, vírgual ou alguma expressão regular no estilo Perl). Após escolher o delimitador, pressione o botão \&quot;analisar\&quot; e selecione as colunas que contém os valores de x e y para a camada.</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <source>Create a Layer from a Delimited Text File</source>
        <translation>Criar uma camada a partir de arquivo de texto delimitado</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Campo X&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Name of the field containing x values</source>
        <translation>Nome do campo contendo valores de X</translation>
    </message>
    <message>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nome do campo contendo valores de X. Escolha um campo da lista. A lista é gerada a partir da linha de cabeçalho do arquivo.</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Campo Y&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Name of the field containing y values</source>
        <translation>Nome do campo contendo valores de Y</translation>
    </message>
    <message>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nome do campo contendo valores de Y. Escolha um campo da lista. A lista é gerada a partir da linha de cabeçalho do arquivo.</translation>
    </message>
    <message>
        <source>Layer name</source>
        <translation>Nome da camada</translation>
    </message>
    <message>
        <source>Name to display in the map legend</source>
        <translation>Nome para exibir na legenda do mapa</translation>
    </message>
    <message>
        <source>Name displayed in the map legend</source>
        <translation>Nome exibido na legenda do mapa</translation>
    </message>
    <message>
        <source>Delimiter</source>
        <translation>Delimitador</translation>
    </message>
    <message>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation>Delimitador usado ao separar os campos do arquivo texto. O delimitador pode possuir mais de um caractere.</translation>
    </message>
    <message>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Delimitador usado ao separar os campos do arquivo texto. O delimitador pode possuir um ou mais caracteres.</translation>
    </message>
    <message>
        <source>Delimited Text Layer</source>
        <translation>Camada de texto delimitado</translation>
    </message>
    <message>
        <source>Delimited text file</source>
        <translation>Arquivo texto delimitado</translation>
    </message>
    <message>
        <source>Full path to the delimited text file</source>
        <translation>Caminho completo para o arquivo de texto delimitado</translation>
    </message>
    <message>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Caminho completo para o arquivo texto delimitado. Para analisar apropriadamente os campos do arquivo, o delimitador deve ser escolhido antes do arquivo. Use o botão procurar ao lado deste campo para escolher um arquivo de entrada.</translation>
    </message>
    <message>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Procurar arquivo de texto delimitado para processamento</translation>
    </message>
    <message>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Utilize este botão para procurar o arquivo texto delimitado. O botão não será habilitado enquanto um delimitador não houver sido escolhido no campo &lt;i&gt;Delimitador&lt;/i&gt;. Depois de escolhido um arquivo, as caixas de seleção X e Y serão preenchidas com os campos do arquivo texto.</translation>
    </message>
    <message>
        <source>Sample text</source>
        <translation>Texto de exemplo</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Procurar...</translation>
    </message>
    <message>
        <source>The delimiter is taken as is</source>
        <translation>O delimitador reconhecido é</translation>
    </message>
    <message>
        <source>Plain characters</source>
        <translation>Caracteres planos</translation>
    </message>
    <message>
        <source>The delimiter is a regular expression</source>
        <translation>O delimitador é uma expressão regular</translation>
    </message>
    <message>
        <source>Regular expression</source>
        <translation>Expressão regular</translation>
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
        <translation>Nota: as linhas que seguem não foram carregadas porque o Qgis não está habilitado a determinar valores para coordenadas X e Y:</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Erro</translation>
    </message>
</context>
<context>
    <name>QgsDetailedItemWidgetBase</name>
    <message>
        <source>Form</source>
        <translation>Modelo</translation>
    </message>
    <message>
        <source>Heading Label</source>
        <translation>Rótulo do cabeçalho</translation>
    </message>
    <message>
        <source>Detail label</source>
        <translation>Rótulo do detalhe</translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <source>Buffer features</source>
        <translation>Feições do Buffer</translation>
    </message>
    <message>
        <source>Buffer distance in map units:</source>
        <translation>Distância dos Buffers em unidades do mapa:</translation>
    </message>
    <message>
        <source>Table name for the buffered layer:</source>
        <translation>Nome da tabela para a camada Buferizada:</translation>
    </message>
    <message>
        <source>Create unique object id</source>
        <translation>Criar ID de objeto único</translation>
    </message>
    <message>
        <source>public</source>
        <translation>público</translation>
    </message>
    <message>
        <source>Geometry column:</source>
        <translation>Geometria da Coluna:</translation>
    </message>
    <message>
        <source>Spatial reference ID:</source>
        <translation>ID de referência Espacial:</translation>
    </message>
    <message>
        <source>Unique field to use as feature id:</source>
        <translation>Campo absoluto para usar como ID de feição:</translation>
    </message>
    <message>
        <source>Schema:</source>
        <translation>Esquema:</translation>
    </message>
    <message>
        <source>Add the buffered layer to the map?</source>
        <translation>Adicionar a camada buferizada para o mapa?</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Buferizar as feições na camada: &lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Parâmetros</translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <source>Encoding:</source>
        <translation>Codificando:</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <source>New device %1</source>
        <translation>Novo dispositivo %1</translation>
    </message>
    <message>
        <source>Are you sure?</source>
        <translation>Tem certeza?</translation>
    </message>
    <message>
        <source>Are you sure that you want to delete this device?</source>
        <translation>Tem certeza que deseja excluir este dispositivo!</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <source>GPS Device Editor</source>
        <translation>Editor de dispositivo GPS</translation>
    </message>
    <message>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Este é o nome como o dispositivo irá aparecer na lista</translation>
    </message>
    <message>
        <source>Update device</source>
        <translation>Atualizar dispositivo</translation>
    </message>
    <message>
        <source>Delete device</source>
        <translation>Excluir dispositivo</translation>
    </message>
    <message>
        <source>New device</source>
        <translation>Novo dispositivo</translation>
    </message>
    <message>
        <source>Commands</source>
        <translation>Commandos</translation>
    </message>
    <message>
        <source>Waypoint download:</source>
        <translation>Descarregar waypoints:</translation>
    </message>
    <message>
        <source>Waypoint upload:</source>
        <translation>Carregar waypoints:</translation>
    </message>
    <message>
        <source>Route download:</source>
        <translation>Descarregar rotas:</translation>
    </message>
    <message>
        <source>Route upload:</source>
        <translation>Carregar rotas:</translation>
    </message>
    <message>
        <source>Track download:</source>
        <translation>Descarregar trilhas:</translation>
    </message>
    <message>
        <source>The command that is used to upload tracks to the device</source>
        <translation>O comando que é usado para carregar trilhas para o dispositivo</translation>
    </message>
    <message>
        <source>Track upload:</source>
        <translation>Carregar trilhas:</translation>
    </message>
    <message>
        <source>The command that is used to download tracks from the device</source>
        <translation>O comando que é usado para descarregar trilhas do dispositivo</translation>
    </message>
    <message>
        <source>The command that is used to upload routes to the device</source>
        <translation>O comando que é usado para carregar rotas para o dispositivo</translation>
    </message>
    <message>
        <source>The command that is used to download routes from the device</source>
        <translation>O comando que é usado para descarregar rotas do dispositivo</translation>
    </message>
    <message>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>O comando que é usado para carregar waypoints para o dispositivo</translation>
    </message>
    <message>
        <source>The command that is used to download waypoints from the device</source>
        <translation>O comando que é usado para descarregar waypoints do dispositivo</translation>
    </message>
    <message>
        <source>Device name</source>
        <translation>Nome do dispositivo</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Nos comandos de baixar e enviar podem haver palavras especiais que serão substituídas pelo QGIS quando os comandos forem usados. Essas palavras são:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - o caminho para o GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - o nome de arquivo GPX quando enviando ou a porta quando baixando&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - o nome de arquivo GPX quando enviando ou a porta quando baixando&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <source>&amp;Gps Tools</source>
        <translation>&amp;Ferramentas GPS</translation>
    </message>
    <message>
        <source>&amp;Create new GPX layer</source>
        <translation>&amp;Cria uma nova camada GPX</translation>
    </message>
    <message>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Cria uma nova camada GPX e mostra no mapa</translation>
    </message>
    <message>
        <source>&amp;Gps</source>
        <translation>&amp;Gps</translation>
    </message>
    <message>
        <source>Save new GPX file as...</source>
        <translation>Salvar novo arquivo arquivo GPX como...</translation>
    </message>
    <message>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>Arquivo de troca GPX (*.gpx)</translation>
    </message>
    <message>
        <source>Could not create file</source>
        <translation>Impossível criar arquivo</translation>
    </message>
    <message>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>Impossível criar um arauivo GPX com o nome fornecido.</translation>
    </message>
    <message>
        <source>Try again with another name or in another </source>
        <translation>Tentar novamente com outro nome ou em outro </translation>
    </message>
    <message>
        <source>directory.</source>
        <translation>diretório.</translation>
    </message>
    <message>
        <source>GPX Loader</source>
        <translation>Carregados GPX</translation>
    </message>
    <message>
        <source>Unable to read the selected file.
</source>
        <translation>Impossível ler o arquivo selecionado.</translation>
    </message>
    <message>
        <source>Please reselect a valid file.</source>
        <translation>Selecione novamente um arquivo válido.</translation>
    </message>
    <message>
        <source>Could not start process</source>
        <translation>Impossível iniciar o processo</translation>
    </message>
    <message>
        <source>Could not start GPSBabel!</source>
        <translation>Impossível iniciar GPSBabel!</translation>
    </message>
    <message>
        <source>Importing data...</source>
        <translation>Importando dados...</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>Could not import data from %1!

</source>
        <translation>Impossível importar dados de %1!</translation>
    </message>
    <message>
        <source>Error importing data</source>
        <translation>Erro ao importar dados</translation>
    </message>
    <message>
        <source>Not supported</source>
        <translation>Não suportado</translation>
    </message>
    <message>
        <source>This device does not support downloading </source>
        <translation>Este dispositivo não suporta descarregar </translation>
    </message>
    <message>
        <source>of </source>
        <translation>de </translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation>Descarregando dados...</translation>
    </message>
    <message>
        <source>Could not download data from GPS!

</source>
        <translation>Impossível descarregar dados do GPS!</translation>
    </message>
    <message>
        <source>Error downloading data</source>
        <translation>Erro ao descarregar dados</translation>
    </message>
    <message>
        <source>This device does not support uploading of </source>
        <translation>Este dispositivo não suporta carregar de </translation>
    </message>
    <message>
        <source>Uploading data...</source>
        <translation>Carregando dados...</translation>
    </message>
    <message>
        <source>Error while uploading data to GPS!

</source>
        <translation>Erro no carregamento de dados para o GPS!</translation>
    </message>
    <message>
        <source>Error uploading data</source>
        <translation>Erro ao carregar dados</translation>
    </message>
    <message>
        <source>Could not convert data from %1!

</source>
        <translation>Não foi possível converter dados de %1!</translation>
    </message>
    <message>
        <source>Error converting data</source>
        <translation>Erro na conversão dos dados</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>Formato de troca GPS (*.gpx)</translation>
    </message>
    <message>
        <source>Select GPX file</source>
        <translation>Selecione um arquivo GPX</translation>
    </message>
    <message>
        <source>Select file and format to import</source>
        <translation>Selecione o arquivo e formate para importar</translation>
    </message>
    <message>
        <source>Waypoints</source>
        <translation type="unfinished">Waypoints</translation>
    </message>
    <message>
        <source>Routes</source>
        <translation type="unfinished">Rotas</translation>
    </message>
    <message>
        <source>Tracks</source>
        <translation type="unfinished">Trilhas</translation>
    </message>
    <message>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation>GPX é o %1, que é usado para salvar informações de waypoints, rotas e trilhas.</translation>
    </message>
    <message>
        <source>GPS eXchange file format</source>
        <translation>Formato de arquivo GPS eXchange</translation>
    </message>
    <message>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation>Selecione um arquivo GPX e depois escolha o tipo de feição que você quer carregar.</translation>
    </message>
    <message>
        <source>This tool will help you download data from a GPS device.</source>
        <translation>Esse programa irá lhe ajudar a baixar os dados de um dispositivo de GPS.</translation>
    </message>
    <message>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation>Escolha o seu dispositivo GPS, a porta à qual ele está conectado, o tipo de feição que você quer baixar dele, o nome da nova camada, e o nome do arquivo GPX em qual serão salvos os dados.</translation>
    </message>
    <message>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation>Se o seu dispositivo não está listado, ou se você quer modificar alguns ajustes, você pode também editar os dispositivos.</translation>
    </message>
    <message>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation>Essa ferramenta usa o programa GPSBabel (%1) para transferir os dados.</translation>
    </message>
    <message>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation>Isto requer que você tenha o GPSBabel instalado em um lugar onde o QGIS possá achá-lo.</translation>
    </message>
    <message>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation>Essa ferramenta irá lhe ajudar a enviar dados de uma camada GPX para um dispositivo de GPS.</translation>
    </message>
    <message>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation>Escolha a camada que você quer enviar, o dispositivo de GPS e a porta que ele está conectado.</translation>
    </message>
    <message>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation>O QGIS pode apenas carregar arquivos GPX, mas muitos outros formatos podem ser convertidos para GPX usando GPSBabel (%1).</translation>
    </message>
    <message>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation>Nem todos os formatos podem salvar waypoints, rotas e trilhas, então algumas feições podem ser desabilidas em alguns formatos.</translation>
    </message>
    <message>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation>QGIS pode realizar conversões de arquivos GPX usando GPSBabel (%1) para realizar tais conversões.</translation>
    </message>
    <message>
        <source>Choose a file name to save under</source>
        <translation>Escolha um nome para o arquivo à ser salvo</translation>
    </message>
    <message>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX file name that you want to save the converted file as, and a name for the new layer.</source>
        <translation>Selecione um tipo de formato de arquivo GPS que você quer importar, o tipo de feição que você quer usar, um nome para o arquivo GPX que você irá salvar com a conversão, e um nome para a camada camada que será criada do resultado.</translation>
    </message>
    <message>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX file name that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation>Selecione um arquivo GPX de entrada, o tipo de conversão que quer realizar, um nome para o arquivo GPX que você irá salvar com a conversão, e um nome para a camada camada que será criada do resultado.</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <source>GPS Tools</source>
        <translation>Ferramentas de GPS</translation>
    </message>
    <message>
        <source>Load GPX file</source>
        <translation>Carregar arquivo GPX</translation>
    </message>
    <message>
        <source>File:</source>
        <translation>Arquivo:</translation>
    </message>
    <message>
        <source>Feature types:</source>
        <translation>Tipos de Feições:</translation>
    </message>
    <message>
        <source>Waypoints</source>
        <translation>Waypoints</translation>
    </message>
    <message>
        <source>Routes</source>
        <translation>Rotas</translation>
    </message>
    <message>
        <source>Tracks</source>
        <translation>Trilhas</translation>
    </message>
    <message>
        <source>Import other file</source>
        <translation>Importar outro arquivo</translation>
    </message>
    <message>
        <source>File to import:</source>
        <translation>Arquivo a ser importado:</translation>
    </message>
    <message>
        <source>Feature type:</source>
        <translation>Tipo de feição:</translation>
    </message>
    <message>
        <source>GPX output file:</source>
        <translation>Arquivo GPX de saída:</translation>
    </message>
    <message>
        <source>Layer name:</source>
        <translation>Nome da camada:</translation>
    </message>
    <message>
        <source>Download from GPS</source>
        <translation>Descarregar do GPS</translation>
    </message>
    <message>
        <source>Edit devices</source>
        <translation>Editar dispositivos</translation>
    </message>
    <message>
        <source>GPS device:</source>
        <translation>Dispositivo GPS:</translation>
    </message>
    <message>
        <source>Output file:</source>
        <translation>Arquivo de saída:</translation>
    </message>
    <message>
        <source>Port:</source>
        <translation>Porta:</translation>
    </message>
    <message>
        <source>Upload to GPS</source>
        <translation>Carregar no GPS</translation>
    </message>
    <message>
        <source>Data layer:</source>
        <translation>Camada de dados:</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Procurar...</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Salvar como...</translation>
    </message>
    <message>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation>(Aviso: Selecionar o tipo de arquivo correto é importante!)</translation>
    </message>
    <message>
        <source>GPX Conversions</source>
        <translation>Conversões GPX</translation>
    </message>
    <message>
        <source>Conversion:</source>
        <translation>Conversão</translation>
    </message>
    <message>
        <source>GPX input file:</source>
        <translation>Arquivo de entrada GPX:</translation>
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
        <translation type="unfinished">Atualizar</translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>URI ruim - você precisa especificar o tipo de feição.</translation>
    </message>
    <message>
        <source>GPS eXchange file</source>
        <translation>Arquivo de troca para GPS</translation>
    </message>
    <message>
        <source>Digitized in QGIS</source>
        <translation>Digitalizada no QGIS</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelector</name>
    <message>
        <source>Define this layer&apos;s projection:</source>
        <translation>Definir a projeção dessa camada:</translation>
    </message>
    <message>
        <source>This layer appears to have no projection specification.</source>
        <translation>Esta camada não parece ter alguma projeção especificada.</translation>
    </message>
    <message>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation>Por padrão, esta camada terá sua projeção especifida como sendo igual à do projeto, mas você pode mudar isso selecionando uma projeção diferente abaixo.</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelectorBase</name>
    <message>
        <source>Projection Selector</source>
        <translation type="unfinished">Seletor de Projeção</translation>
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
        <translation>Inteiro</translation>
    </message>
    <message>
        <source>String</source>
        <translation>Texto</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialogBase</name>
    <message>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Ponto</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Linha</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Polígono</translation>
    </message>
    <message>
        <source>New Vector Layer</source>
        <translation>Nova camada vetorial</translation>
    </message>
    <message>
        <source>File format</source>
        <translation>Formato do arquivo</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation type="unfinished">Atributos</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Delete selected attribute</source>
        <translation>Apagar atributo selecionado</translation>
    </message>
    <message>
        <source>Add attribute</source>
        <translation>Adicionar atributo</translation>
    </message>
</context>
<context>
    <name>QgsGeorefDescriptionDialogBase</name>
    <message>
        <source>Description georeferencer</source>
        <translation>Descrição do Georreferenciador</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Descrição&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;&quot;&gt;Este plugin serve para você georreferenciar suas imagens raster. Você seleciona pontos na imagem raster e fornece as coordenadas reais (novo plano de coordenadas), e o plugin irá armazená-los. Quanto mais pontos você informar, mais precisa será a georreferência.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPlugin</name>
    <message>
        <source>&amp;Georeferencer</source>
        <translation>&amp;Georreferenciador</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <source>Choose a raster file</source>
        <translation>Escolha um arquivo raster</translation>
    </message>
    <message>
        <source>Raster files (*.*)</source>
        <translation>Arquivos raster (*.*)</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="unfinished">Erro</translation>
    </message>
    <message>
        <source>The selected file is not a valid raster file.</source>
        <translation>O arquivo selecionado não é válido como raster.</translation>
    </message>
    <message>
        <source>World file exists</source>
        <translation>World file existe</translation>
    </message>
    <message>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation>&lt;p&gt;O arquivo selecionado já parece ter um</translation>
    </message>
    <message>
        <source>world file! Do you want to replace it with the </source>
        <translation>world file! Você quer substituí-lo com </translation>
    </message>
    <message>
        <source>new world file?&lt;/p&gt;</source>
        <translation>Novo World file?</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <source>Georeferencer</source>
        <translation>Georreferenciador</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Raster file:</source>
        <translation>Arquivo raster:</translation>
    </message>
    <message>
        <source>Arrange plugin windows</source>
        <translation>Arruma janelas de plugins</translation>
    </message>
    <message>
        <source>Description...</source>
        <translation>Descrição...</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialogBase</name>
    <message>
        <source>Warp options</source>
        <translation>Opções Warp</translation>
    </message>
    <message>
        <source>Resampling method:</source>
        <translation>Método de reamostragem:</translation>
    </message>
    <message>
        <source>Nearest neighbour</source>
        <translation>Vizinho mais próximo</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Linear</translation>
    </message>
    <message>
        <source>Cubic</source>
        <translation>Cúbico</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Use 0 for transparency when needed</source>
        <translation>Use 0 para transparência quando necessário</translation>
    </message>
    <message>
        <source>Compression:</source>
        <translation>Compressão:</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <source>Equal Interval</source>
        <translation>Intervalo Igual</translation>
    </message>
    <message>
        <source>Quantiles</source>
        <translation>Quantis</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>Vazio</translation>
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
        <translation>Excluir classe</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Classificar</translation>
    </message>
    <message>
        <source>Classification field</source>
        <translation>Campo de Classificação</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="unfinished">Modo</translation>
    </message>
    <message>
        <source>Number of classes</source>
        <translation>Número de classes</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>Column</source>
        <translation>Coluna</translation>
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
        <translation>ERRO</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <source>GRASS Attributes</source>
        <translation>Atributos do GRASS</translation>
    </message>
    <message>
        <source>Tab 1</source>
        <translation>Tabela 1</translation>
    </message>
    <message>
        <source>result</source>
        <translation>resultado</translation>
    </message>
    <message>
        <source>Update</source>
        <translation>Atualizar</translation>
    </message>
    <message>
        <source>Update database record</source>
        <translation>Atualiza a base de dados</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Novo</translation>
    </message>
    <message>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Adicionar categoria usando configurações da Caixa de ferramentas de edição do GRASS</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Excluir</translation>
    </message>
    <message>
        <source>Delete selected category</source>
        <translation>Excluir a categoria selecionada</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <source>Tools</source>
        <translation>Ferramentas</translation>
    </message>
    <message>
        <source>Add selected map to canvas</source>
        <translation>Adiciona o mapa selecionado à tela</translation>
    </message>
    <message>
        <source>Copy selected map</source>
        <translation>Copia o mapa selecionado</translation>
    </message>
    <message>
        <source>Rename selected map</source>
        <translation>Renomeia o mapa selecionado</translation>
    </message>
    <message>
        <source>Delete selected map</source>
        <translation>Exclui o mapa selecionado</translation>
    </message>
    <message>
        <source>Set current region to selected map</source>
        <translation>Atualiza uma região no mapa selecionado </translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Atualizar</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>Cannot copy map </source>
        <translation>Impossível copiar mapa </translation>
    </message>
    <message>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt; comando: </translation>
    </message>
    <message>
        <source>Cannot rename map </source>
        <translation>Impossível renomear mapa </translation>
    </message>
    <message>
        <source>Delete map &lt;b&gt;</source>
        <translation>Excluir mapa &lt;b&gt;</translation>
    </message>
    <message>
        <source>Cannot delete map </source>
        <translation>Impossível deletar mapa </translation>
    </message>
    <message>
        <source>Cannot write new region</source>
        <translation>Impossível gravar nova região</translation>
    </message>
    <message>
        <source>New name</source>
        <translation>Novo nome</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <source>New point</source>
        <translation>Novo ponto</translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation>Novo centróide</translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation>Excluir vértice</translation>
    </message>
    <message>
        <source>Left: </source>
        <translation>Esquerdo: </translation>
    </message>
    <message>
        <source>Middle: </source>
        <translation>Médio: </translation>
    </message>
    <message>
        <source>Edit tools</source>
        <translation>Ferramentas de edição</translation>
    </message>
    <message>
        <source>New line</source>
        <translation>Nova linha</translation>
    </message>
    <message>
        <source>New boundary</source>
        <translation>Nova borda</translation>
    </message>
    <message>
        <source>Move vertex</source>
        <translation>Mover vértice</translation>
    </message>
    <message>
        <source>Add vertex</source>
        <translation>Adicionar vértice</translation>
    </message>
    <message>
        <source>Move element</source>
        <translation>Mover elemento</translation>
    </message>
    <message>
        <source>Split line</source>
        <translation>Dividir linha</translation>
    </message>
    <message>
        <source>Delete element</source>
        <translation>Excluir elemento</translation>
    </message>
    <message>
        <source>Edit attributes</source>
        <translation>Editar atributos</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>Você não é usuário do mapa, impossível abrir o vetor para edição.</translation>
    </message>
    <message>
        <source>Cannot open vector for update.</source>
        <translation>Impossível abrir vetor para atualização.</translation>
    </message>
    <message>
        <source>Info</source>
        <translation>Info</translation>
    </message>
    <message>
        <source>The table was created</source>
        <translation>A tabela foi criada</translation>
    </message>
    <message>
        <source>Tool not yet implemented.</source>
        <translation>Ferramenta ainda não implementada.</translation>
    </message>
    <message>
        <source>Cannot check orphan record: </source>
        <translation>Impossível checar registro órfão: </translation>
    </message>
    <message>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation>Registro órfão ficou solto na tabela de atributos. &lt;br&gt; Excluir o registro?</translation>
    </message>
    <message>
        <source>Cannot delete orphan record: </source>
        <translation>Impossível excluir registro órfão: </translation>
    </message>
    <message>
        <source>Cannot describe table for field </source>
        <translation>Impossível descrever tabela para o campo </translation>
    </message>
    <message>
        <source>Background</source>
        <translation>Fundo</translation>
    </message>
    <message>
        <source>Highlight</source>
        <translation>Ênfase</translation>
    </message>
    <message>
        <source>Dynamic</source>
        <translation>Dinâmico</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Ponto</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Linha</translation>
    </message>
    <message>
        <source>Boundary (no area)</source>
        <translation>Limitar (sem área)</translation>
    </message>
    <message>
        <source>Boundary (1 area)</source>
        <translation>LImitar (1 área)</translation>
    </message>
    <message>
        <source>Boundary (2 areas)</source>
        <translation>LImitar (2 áreas)</translation>
    </message>
    <message>
        <source>Centroid (in area)</source>
        <translation>Centróide (na área)</translation>
    </message>
    <message>
        <source>Centroid (outside area)</source>
        <translation>Centróide (fora da área)</translation>
    </message>
    <message>
        <source>Centroid (duplicate in area)</source>
        <translation>Centróide (duplicado na área)</translation>
    </message>
    <message>
        <source>Node (1 line)</source>
        <translation>Nó (1 linha)</translation>
    </message>
    <message>
        <source>Node (2 lines)</source>
        <translation>Nó (2 linhas)</translation>
    </message>
    <message>
        <source>Next not used</source>
        <translation>Próximo não usado</translation>
    </message>
    <message>
        <source>Manual entry</source>
        <translation>Entrada manual</translation>
    </message>
    <message>
        <source>No category</source>
        <translation>Sem categoria</translation>
    </message>
    <message>
        <source>Right: </source>
        <translation>Direita: </translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <source>GRASS Edit</source>
        <translation>Editor GRASS</translation>
    </message>
    <message>
        <source>Category</source>
        <translation>Categoria</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Modo</translation>
    </message>
    <message>
        <source>Settings</source>
        <translation>Opções</translation>
    </message>
    <message>
        <source>Snapping in screen pixels</source>
        <translation>Snapping em pixels da tela</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Simbologia</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabela</translation>
    </message>
    <message>
        <source>Add Column</source>
        <translation>Adicionar coluna</translation>
    </message>
    <message>
        <source>Create / Alter Table</source>
        <translation>Criar/Alterar Tabela</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation>Largura da linha</translation>
    </message>
    <message>
        <source>Marker size</source>
        <translation>Tamanho do marcador</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <source>Disp</source>
        <translation type="unfinished">Disp</translation>
    </message>
    <message>
        <source>Color</source>
        <translation type="unfinished">Cor</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="unfinished">Tipo</translation>
    </message>
    <message>
        <source>Index</source>
        <translation type="unfinished">Índice</translation>
    </message>
    <message>
        <source>Column</source>
        <translation type="unfinished">Coluna</translation>
    </message>
    <message>
        <source>Length</source>
        <translation type="unfinished">Comprimento</translation>
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
        <translation>Ok</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Entre com um nome!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Este é o nome da fonte!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Existe!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation>Sobrescrever</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <source>Mapcalc tools</source>
        <translation>Ferramentas Mapcalc</translation>
    </message>
    <message>
        <source>Add map</source>
        <translation>Adiciona mapa</translation>
    </message>
    <message>
        <source>Add constant value</source>
        <translation>Adiciona um valor constante</translation>
    </message>
    <message>
        <source>Add operator or function</source>
        <translation>Adiciona operador ou função</translation>
    </message>
    <message>
        <source>Add connection</source>
        <translation>Adiciona conexão</translation>
    </message>
    <message>
        <source>Select item</source>
        <translation>Seleciona item</translation>
    </message>
    <message>
        <source>Delete selected item</source>
        <translation>Exclui item selecionado</translation>
    </message>
    <message>
        <source>Open</source>
        <translation>Abrir</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Salvar</translation>
    </message>
    <message>
        <source>Save as</source>
        <translation>Salvar como</translation>
    </message>
    <message>
        <source>Addition</source>
        <translation>Adição</translation>
    </message>
    <message>
        <source>Subtraction</source>
        <translation>Subtração</translation>
    </message>
    <message>
        <source>Multiplication</source>
        <translation>Multiplicação</translation>
    </message>
    <message>
        <source>Division</source>
        <translation>Divisão</translation>
    </message>
    <message>
        <source>Modulus</source>
        <translation>Módulo</translation>
    </message>
    <message>
        <source>Exponentiation</source>
        <translation>Exponenciação</translation>
    </message>
    <message>
        <source>Equal</source>
        <translation>Igual</translation>
    </message>
    <message>
        <source>Not equal</source>
        <translation>Não igual</translation>
    </message>
    <message>
        <source>Greater than</source>
        <translation>Maior que</translation>
    </message>
    <message>
        <source>Greater than or equal</source>
        <translation>Igual ou maior que</translation>
    </message>
    <message>
        <source>Less than</source>
        <translation>Menor que</translation>
    </message>
    <message>
        <source>Less than or equal</source>
        <translation>Igual ou menor que</translation>
    </message>
    <message>
        <source>And</source>
        <translation>E</translation>
    </message>
    <message>
        <source>Or</source>
        <translation>Ou</translation>
    </message>
    <message>
        <source>Absolute value of x</source>
        <translation>Valor absoluto para X</translation>
    </message>
    <message>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>Tangente inversa de x (resultado está em graus)</translation>
    </message>
    <message>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>Tangente inversa de y/x (resultado está em graus)</translation>
    </message>
    <message>
        <source>Current column of moving window (starts with 1)</source>
        <translation>Coluna atual de mover janela (inicia com 1)</translation>
    </message>
    <message>
        <source>Cosine of x (x is in degrees)</source>
        <translation>Cosseno de x (x está em graus)</translation>
    </message>
    <message>
        <source>Convert x to double-precision floating point</source>
        <translation>Convert x para ponto flutuante de dupla precisão </translation>
    </message>
    <message>
        <source>Current east-west resolution</source>
        <translation>Resolução leste-oeste atual</translation>
    </message>
    <message>
        <source>Exponential function of x</source>
        <translation>Função exponencial de x</translation>
    </message>
    <message>
        <source>x to the power y</source>
        <translation>x elevado na y</translation>
    </message>
    <message>
        <source>Convert x to single-precision floating point</source>
        <translation>Convert x para ponto flutuante de precisão simples</translation>
    </message>
    <message>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>Decisão: 1 se x diferente de zero, 0 caso contrário</translation>
    </message>
    <message>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>Decisão: a se x diferente de zero, 0 caso contrário</translation>
    </message>
    <message>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>Decisão: a se x diferente de zero, b caso contrário</translation>
    </message>
    <message>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>Decisão: a se x &gt; 0, b se x é zero, c se x &lt; 0</translation>
    </message>
    <message>
        <source>Convert x to integer [ truncates ]</source>
        <translation>Convert x para integer [ truncados ]</translation>
    </message>
    <message>
        <source>Check if x = NULL</source>
        <translation>Verifica se x = NULO</translation>
    </message>
    <message>
        <source>Natural log of x</source>
        <translation>Log natural de x</translation>
    </message>
    <message>
        <source>Log of x base b</source>
        <translation>Log de x base b</translation>
    </message>
    <message>
        <source>Largest value</source>
        <translation>Maior valor </translation>
    </message>
    <message>
        <source>Median value</source>
        <translation>Valor médio</translation>
    </message>
    <message>
        <source>Smallest value</source>
        <translation>Menor valor</translation>
    </message>
    <message>
        <source>Mode value</source>
        <translation>Valor de moda</translation>
    </message>
    <message>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1 se x é zero, 0 caso contrário</translation>
    </message>
    <message>
        <source>Current north-south resolution</source>
        <translation>Resolução norte-sul atual</translation>
    </message>
    <message>
        <source>NULL value</source>
        <translation>Valor NULO</translation>
    </message>
    <message>
        <source>Random value between a and b</source>
        <translation>Valor aleatório entre a e b</translation>
    </message>
    <message>
        <source>Round x to nearest integer</source>
        <translation>Arredondou x para o inteiro mais próximo</translation>
    </message>
    <message>
        <source>Current row of moving window (Starts with 1)</source>
        <translation>Coluna atual da janela móvel (Inicia com 1)</translation>
    </message>
    <message>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation>Seno de x (x está em graus)</translation>
    </message>
    <message>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>Raiz quadrada de x</translation>
    </message>
    <message>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation>Tangente de x (x está em graus)</translation>
    </message>
    <message>
        <source>Current x-coordinate of moving window</source>
        <translation>Coordenada x atual da moving window</translation>
    </message>
    <message>
        <source>Current y-coordinate of moving window</source>
        <translation>Coordenada y atual da moving window</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation>Impossível pegar região</translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation>Impossível verificar região do mapa</translation>
    </message>
    <message>
        <source>Cannot get region of map </source>
        <translation>Impossível pegar região do mapa</translation>
    </message>
    <message>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>Sem mapas raster do GRASS atualmente no QGIS</translation>
    </message>
    <message>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>Impossível criar diretório &apos;mapacalc&apos; no mapset atual.</translation>
    </message>
    <message>
        <source>New mapcalc</source>
        <translation>Novo mapcalc</translation>
    </message>
    <message>
        <source>Enter new mapcalc name:</source>
        <translation>Entre com o novonome de mapcalc:</translation>
    </message>
    <message>
        <source>Enter vector name</source>
        <translation>Entre com o nome do vetor</translation>
    </message>
    <message>
        <source>The file already exists. Overwrite? </source>
        <translation>O arquivo já existe. Sobrescrever?</translation>
    </message>
    <message>
        <source>Save mapcalc</source>
        <translation>Salvar mapcalc</translation>
    </message>
    <message>
        <source>File name empty</source>
        <translation>Nome de aruivo vazio</translation>
    </message>
    <message>
        <source>Cannot open mapcalc file</source>
        <translation>IMpossível abrir arquivo mapcalc</translation>
    </message>
    <message>
        <source>The mapcalc schema (</source>
        <translation>Esquema do mapcalc (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) não encontrado.</translation>
    </message>
    <message>
        <source>Cannot open mapcalc schema (</source>
        <translation>Impossível abrir esquema mapcalc (</translation>
    </message>
    <message>
        <source>Cannot read mapcalc schema (</source>
        <translation>Impossível ler esquema mapcalc (</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
na linha </translation>
    </message>
    <message>
        <source> column </source>
        <translation> coluna </translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Saída</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <source>MainWindow</source>
        <translation>JanelaPrincipal</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Saída</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <source>Run</source>
        <translation>Rodar</translation>
    </message>
    <message>
        <source>Stop</source>
        <translation>Parar</translation>
    </message>
    <message>
        <source>Module</source>
        <translation>Módulo</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>The module file (</source>
        <translation>O arquivo módulo (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) não foi encontrado.</translation>
    </message>
    <message>
        <source>Cannot open module file (</source>
        <translation>Impossível abrir arquivo módulo (</translation>
    </message>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>Cannot read module file (</source>
        <translation>Impossível ler arquivo módulo (</translation>
    </message>
    <message>
        <source>):
</source>
        <translation>):</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
na linha</translation>
    </message>
    <message>
        <source>Module </source>
        <translation>Módulo </translation>
    </message>
    <message>
        <source> not found</source>
        <translation> não encontrado</translation>
    </message>
    <message>
        <source>Cannot find man page </source>
        <translation>Não é possível encontrar a man page</translation>
    </message>
    <message>
        <source>Not available, cannot open description (</source>
        <translation>Não disponível, impossível abrir descrição (</translation>
    </message>
    <message>
        <source> column </source>
        <translation> coluna </translation>
    </message>
    <message>
        <source>Not available, incorrect description (</source>
        <translation>Não disponível, descrição incorreta (</translation>
    </message>
    <message>
        <source>Cannot get input region</source>
        <translation>Impossível obter região de entrada</translation>
    </message>
    <message>
        <source>Use Input Region</source>
        <translation>Use região de entrada</translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation>Impossível encontrar módulo </translation>
    </message>
    <message>
        <source>Cannot start module: </source>
        <translation>Impossível iniciar módulo: </translation>
    </message>
    <message>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation>&lt;B&gt;Finalizado com sucesso&lt;/B&gt;</translation>
    </message>
    <message>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation>&lt;B&gt;Finalizado com erro&lt;/B&gt;</translation>
    </message>
    <message>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation>&lt;B&gt;Módulo derrumado ou morto&lt;/B&gt;</translation>
    </message>
    <message>
        <source>Not available, description not found (</source>
        <translation>Não disponível, descrição não encontrada (</translation>
    </message>
    <message>
        <source>Please ensure you have the GRASS documentation installed.</source>
        <translation>Por favor se certifique que a documentação do GRASS está instalada.</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <source>GRASS Module</source>
        <translation>Módulo do GRASS</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opções</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Saída</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation>Manual</translation>
    </message>
    <message>
        <source>Run</source>
        <translation>Rodar</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <source>View output</source>
        <translation>Ver saída</translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation>EtiquetaTexto</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <source>Attribute field</source>
        <translation>Campo de atributo</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <source>File</source>
        <translation>Arquivo</translation>
    </message>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;valor perdido</translation>
    </message>
    <message>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation>:&amp;nbsp;diretório não existe</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>Cannot find layeroption </source>
        <translation>Impossível encontrar opção de camada </translation>
    </message>
    <message>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation>O driver PostGIS em OGR não suporta esquemas!&lt;br&gt;Apenas o nome da tabela pode ser usado.&lt;br&gt;Isto pode resultar em entradas estranhas se mais tabelas com o mesmo nome &lt;br&gt; estiverem presentes na base de dados.</translation>
    </message>
    <message>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;sem entrada</translation>
    </message>
    <message>
        <source>Cannot find whereoption </source>
        <translation>Impossível procurar opçãoonde</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>Cannot find typeoption </source>
        <translation>Impossível procurar opçãotipo</translation>
    </message>
    <message>
        <source>Cannot find values for typeoption </source>
        <translation>Impossível encontrar valores para opçãotipo </translation>
    </message>
    <message>
        <source>Cannot find layeroption </source>
        <translation>Impossível encontrar opçãocamada </translation>
    </message>
    <message>
        <source>GRASS element </source>
        <translation>Elemento do GRASS</translation>
    </message>
    <message>
        <source> not supported</source>
        <translation> não suportado</translation>
    </message>
    <message>
        <source>Use region of this map</source>
        <translation>Use região para este mapa</translation>
    </message>
    <message>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;sem entrada</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;valor perdido</translation>
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
        <translation>Atenção</translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation>Impossível encontrar módulo </translation>
    </message>
    <message>
        <source>Cannot start module </source>
        <translation>Impossível iniciar módulo </translation>
    </message>
    <message>
        <source>Cannot read module description (</source>
        <translation>Impossível ler a descrição do módulo (</translation>
    </message>
    <message>
        <source>):
</source>
        <translation>):</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
na linha </translation>
    </message>
    <message>
        <source> column </source>
        <translation> coluna </translation>
    </message>
    <message>
        <source>Cannot find key </source>
        <translation>Impossível encontrar chave </translation>
    </message>
    <message>
        <source>Item with id </source>
        <translation>Item com identificação </translation>
    </message>
    <message>
        <source> not found</source>
        <translation> não encontrado</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation>Impossível obter a região atual</translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation>Impossível verificar a região do mapa </translation>
    </message>
    <message>
        <source>Cannot set region of map </source>
        <translation>Impossível definir região do mapa </translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <source>Database</source>
        <translation>Banco de Dados</translation>
    </message>
    <message>
        <source>Location 2</source>
        <translation>Localização 2</translation>
    </message>
    <message>
        <source>User&apos;s mapset</source>
        <translation>Mapset do usuário</translation>
    </message>
    <message>
        <source>System mapset</source>
        <translation>Mapset do sistema</translation>
    </message>
    <message>
        <source>Location 1</source>
        <translation>Localização1</translation>
    </message>
    <message>
        <source>Enter path to GRASS database</source>
        <translation>Entre com o caminho da base de dados do GRASS</translation>
    </message>
    <message>
        <source>The directory doesn&apos;t exist!</source>
        <translation>O diretório não existe!</translation>
    </message>
    <message>
        <source>No writable locations, the database not writable!</source>
        <translation>Locais sem escrita, a base de dados não foi gravada!</translation>
    </message>
    <message>
        <source>Enter location name!</source>
        <translation>Entre com o nome da localização</translation>
    </message>
    <message>
        <source>The location exists!</source>
        <translation>A localização existe!</translation>
    </message>
    <message>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>A projeção selecionada não é suportada pelo GRASS!</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>Cannot create projection.</source>
        <translation>Impossível criar projeção.</translation>
    </message>
    <message>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation>Impossível reprojetar a região previamente definida, região padrão definida.</translation>
    </message>
    <message>
        <source>North must be greater than south</source>
        <translation>Norte deve ser maior que sul</translation>
    </message>
    <message>
        <source>East must be greater than west</source>
        <translation>Leste deve ser maior que oeste</translation>
    </message>
    <message>
        <source>Regions file (</source>
        <translation>Arquivo de regiões (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) não encontrado.</translation>
    </message>
    <message>
        <source>Cannot open locations file (</source>
        <translation>Impossível abrir arquivo de localizações (</translation>
    </message>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>Cannot read locations file (</source>
        <translation>Impossível ler arquivo de localizações (</translation>
    </message>
    <message>
        <source>):
</source>
        <translation>):</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
na linha </translation>
    </message>
    <message>
        <source> column </source>
        <translation> coluna </translation>
    </message>
    <message>
        <source>Cannot reproject selected region.</source>
        <translation>Impossível reprojetar a região selecionada.</translation>
    </message>
    <message>
        <source>Cannot reproject region</source>
        <translation>Impossível reprojetar região</translation>
    </message>
    <message>
        <source>Enter mapset name.</source>
        <translation>Entre com o nome do mapset.</translation>
    </message>
    <message>
        <source>The mapset already exists</source>
        <translation>O mapset já existe</translation>
    </message>
    <message>
        <source>Database: </source>
        <translation>Base de dados: </translation>
    </message>
    <message>
        <source>Location: </source>
        <translation>Localização: </translation>
    </message>
    <message>
        <source>Mapset: </source>
        <translation>Mapset: </translation>
    </message>
    <message>
        <source>Create location</source>
        <translation>Cria localização</translation>
    </message>
    <message>
        <source>Cannot create new location: </source>
        <translation>Impossível criar nova localização: </translation>
    </message>
    <message>
        <source>Create mapset</source>
        <translation>Criar Mapset</translation>
    </message>
    <message>
        <source>Cannot open DEFAULT_WIND</source>
        <translation>Impossível abrir DEFAULT_WIND</translation>
    </message>
    <message>
        <source>Cannot open WIND</source>
        <translation>Impossível abrir WIND</translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation>Novo mapset</translation>
    </message>
    <message>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>Novo mapset criado com sucesso, mas não pode ser aberto: </translation>
    </message>
    <message>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>Novo mapset criado com sucesso e definido como mapset de trabalho atual</translation>
    </message>
    <message>
        <source>Cannot create new mapset directory</source>
        <translation>Impossível criar novo diretório mapsete</translation>
    </message>
    <message>
        <source>Cannot create QgsCoordinateReferenceSystem</source>
        <translation>Impossívem criar ReferenciadeSistemadeCoordenadasQgs</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <source>Example directory tree:</source>
        <translation>Árvore de diretório de exemplo:</translation>
    </message>
    <message>
        <source>Database Error</source>
        <translation>Erro na base de dados</translation>
    </message>
    <message>
        <source>Database:</source>
        <translation>Base de dados:</translation>
    </message>
    <message>
        <source>Select existing directory or create a new one:</source>
        <translation>Selecione um diretório existente ou crie um:</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Localidade</translation>
    </message>
    <message>
        <source>Select location</source>
        <translation>Selecione localização</translation>
    </message>
    <message>
        <source>Create new location</source>
        <translation>Cria uma nova localização</translation>
    </message>
    <message>
        <source>Location Error</source>
        <translation>Erro de localização</translation>
    </message>
    <message>
        <source>Projection Error</source>
        <translation>Erro na projeção</translation>
    </message>
    <message>
        <source>Coordinate system</source>
        <translation>Sistema de coordenadas</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation>Projeção</translation>
    </message>
    <message>
        <source>Not defined</source>
        <translation>Não definido</translation>
    </message>
    <message>
        <source>Set current QGIS extent</source>
        <translation>Define extensão atual do QGIS</translation>
    </message>
    <message>
        <source>Set</source>
        <translation>Configurar</translation>
    </message>
    <message>
        <source>Region Error</source>
        <translation>Erro na região</translation>
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
        <translation>Novo mapset:</translation>
    </message>
    <message>
        <source>Mapset Error</source>
        <translation>Erro no mapset</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;center&quot;&gt;Mapsets existentes&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Location:</source>
        <translation>Localização:</translation>
    </message>
    <message>
        <source>Mapset:</source>
        <translation>Mapset:</translation>
    </message>
    <message>
        <source>New Mapset</source>
        <translation>Novo Mapset</translation>
    </message>
    <message>
        <source>GRASS Database</source>
        <translation>Base de dados GRASS</translation>
    </message>
    <message>
        <source>Tree</source>
        <translation type="unfinished">Árvore</translation>
    </message>
    <message>
        <source>Comment</source>
        <translation type="unfinished">Comentário</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Dados do GRASS ficam armazenados na estrutura da árvore de diretótuA base de dados GRASS está no nível superior desta árvore de diretóture.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Procurar...</translation>
    </message>
    <message>
        <source>GRASS Location</source>
        <translation>Localização do GRASS</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;A localização do GRASS é uma coleção de mapas para um território ou projeto particular.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Default GRASS Region</source>
        <translation type="unfinished">Região padrão do GRASS</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;A região do GRASS define um espaço de trabalho para módulos raster. A região padrão é válida para uma localização. É possível configurar uma região diferente em cada mapset. É possível mudar a região padrão mais tarde.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation>Mapset</translation>
    </message>
    <message>
        <source>Owner</source>
        <translation type="unfinished">Proprietário</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;O mapset GRASu é ima coleção de mapas usada por um usuário. Um usuário pode ler mapas de todos os mapsets em um lomas poderá escrever apenas em seu mapsetser).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Create New Mapset</source>
        <translation type="unfinished">Cria um novo Mapset</translation>
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
        <translation>Abrir mapset</translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation>Novo mapset</translation>
    </message>
    <message>
        <source>Close mapset</source>
        <translation>Fechar mapset</translation>
    </message>
    <message>
        <source>Add GRASS vector layer</source>
        <translation>Adiciona uma camada vetorial do GRASS</translation>
    </message>
    <message>
        <source>Add GRASS raster layer</source>
        <translation>Adiciona uma camada raster do GRASS</translation>
    </message>
    <message>
        <source>Open GRASS tools</source>
        <translation>Abrir ferramentas GRASS</translation>
    </message>
    <message>
        <source>Display Current Grass Region</source>
        <translation>Mostra a região atual do GRASS</translation>
    </message>
    <message>
        <source>Edit Current Grass Region</source>
        <translation>Edita região atual do GRASS</translation>
    </message>
    <message>
        <source>Edit Grass Vector layer</source>
        <translation>Edita camada vetorial do GRASS</translation>
    </message>
    <message>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>Adiciona uma camada vetorial do GRASS ao mapa</translation>
    </message>
    <message>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>Adiciona uma camada raster do GRASS ao mapa </translation>
    </message>
    <message>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Mostra a região atual de GRASS como retângulo no mapa</translation>
    </message>
    <message>
        <source>Edit the current GRASS region</source>
        <translation>Edita a região atual do GRASS</translation>
    </message>
    <message>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Edita a camada vetorial GRASS selecionada.</translation>
    </message>
    <message>
        <source>GrassVector</source>
        <translation>GrassVector</translation>
    </message>
    <message>
        <source>0.1</source>
        <translation>0.1</translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation>Camada do GRASS</translation>
    </message>
    <message>
        <source>Create new Grass Vector</source>
        <translation>Cria novo vetor do GRASS</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>GRASS Edit is already running.</source>
        <translation>Edição do GRASS já está rodando.</translation>
    </message>
    <message>
        <source>New vector name</source>
        <translation>Novo nome de vetor</translation>
    </message>
    <message>
        <source>Cannot create new vector: </source>
        <translation>Impossível criar novo vetor: </translation>
    </message>
    <message>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation>Novo vetor criado mas impossível de ser aberto pelo provedor de dados.</translation>
    </message>
    <message>
        <source>Cannot start editing.</source>
        <translation>Impossível iniciar edição.</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>GISDBASE, LOCATION_NAME ou MAPSET não definido, impossível mostrar região atual.</translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation>Impossível ler região atual: </translation>
    </message>
    <message>
        <source>Cannot open the mapset. </source>
        <translation>Impossível abrir o mapset. </translation>
    </message>
    <message>
        <source>Cannot close mapset. </source>
        <translation>Impossível fechar o mapset. </translation>
    </message>
    <message>
        <source>Cannot close current mapset. </source>
        <translation>IMpossível fechar mapset atual. </translation>
    </message>
    <message>
        <source>Cannot open GRASS mapset. </source>
        <translation>Impossível abrir GRASS mapset. </translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>GISDBASE, LOCATION_NAME or MAPSET não definido, impossível mostrar região atual.</translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation>Impossível ler região atual: </translation>
    </message>
    <message>
        <source>Cannot write region</source>
        <translation>Impossível gravar região</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <source>GRASS Region Settings</source>
        <translation>GRASS Configurações da Região</translation>
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
        <translation>N-S Res</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation>linhas</translation>
    </message>
    <message>
        <source>Cols</source>
        <translation>Colunas</translation>
    </message>
    <message>
        <source>E-W Res</source>
        <translation>E-W Res</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Cor</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Largura</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
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
        <translation>Seleciona camada vetorial GRASS</translation>
    </message>
    <message>
        <source>Select GRASS Raster Layer</source>
        <translation>Seleciona uma camada GRASS raster </translation>
    </message>
    <message>
        <source>Select GRASS mapcalc schema</source>
        <translation>Seleciona esquema GRASS mapcalc</translation>
    </message>
    <message>
        <source>Select GRASS Mapset</source>
        <translation>Seleciona GRASS mapset</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation>Impossível abrir vetor no nível 2 (topologia não disponível).</translation>
    </message>
    <message>
        <source>Choose existing GISDBASE</source>
        <translation>Escolha uma GISDBASE existente</translation>
    </message>
    <message>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>GISDBASE errada, sem localizações disponíveis.</translation>
    </message>
    <message>
        <source>Wrong GISDBASE</source>
        <translation>GISDBASE errada</translation>
    </message>
    <message>
        <source>Select a map.</source>
        <translation>Seleciona um mapa.</translation>
    </message>
    <message>
        <source>No map</source>
        <translation>Sem mapa</translation>
    </message>
    <message>
        <source>No layer</source>
        <translation>Sem camada</translation>
    </message>
    <message>
        <source>No layers available in this map</source>
        <translation>Sem camadas disponíveis neste mapa</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <source>Gisdbase</source>
        <translation>Fonte de Dados</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Localidade</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Exibir</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation>Conjunto de mapas</translation>
    </message>
    <message>
        <source>Map name</source>
        <translation>Nome do mapa</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Layer</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Selecione ou digite o nome do mapa (caracteres &apos;*&apos; e &apos;?&apos;  são aceitos para rasters)</translation>
    </message>
    <message>
        <source>Add GRASS Layer</source>
        <translation>Adicionar Camada GRASS</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
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
        <translation type="unfinished">Fechar</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <source>Browser</source>
        <translation>Navegador</translation>
    </message>
    <message>
        <source>GRASS Tools</source>
        <translation>Ferramentas GRASS</translation>
    </message>
    <message>
        <source>GRASS Tools: </source>
        <translation>Ferramentas GRASS: </translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>Cannot find MSYS (</source>
        <translation>Impossível encontrar MSYS (</translation>
    </message>
    <message>
        <source>GRASS Shell is not compiled.</source>
        <translation>GRASS shell não está compilado.</translation>
    </message>
    <message>
        <source>The config file (</source>
        <translation>O arquivo de configuração (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) não encontrado.</translation>
    </message>
    <message>
        <source>Cannot open config file (</source>
        <translation>Impossível abrir arquivo de configuração (</translation>
    </message>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>Cannot read config file (</source>
        <translation>Impossível ler arquivo de configuração (</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
na linha</translation>
    </message>
    <message>
        <source> column </source>
        <translation> coluna </translation>
    </message>
</context>
<context>
    <name>QgsGrassToolsBase</name>
    <message>
        <source>Grass Tools</source>
        <translation>Ferramentas GRASS</translation>
    </message>
    <message>
        <source>Modules Tree</source>
        <translation>Árvore de módulos</translation>
    </message>
    <message>
        <source>1</source>
        <translation type="unfinished">1</translation>
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
        <translation>&amp;Criador de Grelha</translation>
    </message>
    <message>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation>Cria uma grelha (malha) e armazena o resultado como um arquivo SHP</translation>
    </message>
    <message>
        <source>&amp;Graticules</source>
        <translation>&amp;Grelha</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <source>QGIS - Grid Maker</source>
        <translation>QGIS - Criador de Grade (Grid)</translation>
    </message>
    <message>
        <source>Please enter the file name before pressing OK!</source>
        <translation>Entre com o nome do arquivo antes de pressionar OK!</translation>
    </message>
    <message>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>ESRI Shapefile (*.shp)</translation>
    </message>
    <message>
        <source>Please enter intervals before pressing OK!</source>
        <translation>Entre com intervalos antes de pressionar OK!</translation>
    </message>
    <message>
        <source>Choose a file name to save under</source>
        <translation>Escolha um nome de arquivo para salvar sob</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <source>Graticule Builder</source>
        <translation>Gerador de Grade (Grid)</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Ponto</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Polígono</translation>
    </message>
    <message>
        <source>Origin (lower left)</source>
        <translation>Origem (canto inferior esquerdo)</translation>
    </message>
    <message>
        <source>End point (upper right)</source>
        <translation>Ponto final (canto superior direito)</translation>
    </message>
    <message>
        <source>Output (shape) file</source>
        <translation>Saída (arquivo SHP)</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Salvar como...</translation>
    </message>
    <message>
        <source>QGIS Graticule Creator</source>
        <translation>Gerador de Grade</translation>
    </message>
    <message>
        <source>Graticle size</source>
        <translation>Tamanho de Grade</translation>
    </message>
    <message>
        <source>Y Interval:</source>
        <translation>Intervalo Y</translation>
    </message>
    <message>
        <source>X Interval:</source>
        <translation>Intervalo X</translation>
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
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Este plugin ajudará você a construir uma grade em formato &quot;shapefile&quot; que poderá ser usada no seu mapa.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;ArialEntre com as coordenadas em graus decimais ou em metrosrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <source>Quantum GIS Help - </source>
        <translation>Ajuda do Quantum GIS - </translation>
    </message>
    <message>
        <source>Failed to get the help text from the database</source>
        <translation>Falha ao adquirir o texto da ajuda da base de dados</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Erro</translation>
    </message>
    <message>
        <source>The QGIS help database is not installed</source>
        <translation>A ajuda do QGIS não está instalada</translation>
    </message>
    <message>
        <source>This help file does not exist for your language</source>
        <translation>O arquivo de ajuda não existe para sua língua.</translation>
    </message>
    <message>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation>Se você deseja criá-lo, contate o time de desenvolvimento QGIS</translation>
    </message>
    <message>
        <source>Quantum GIS Help</source>
        <translation>Ajuda do Quantum GIS</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <source>QGIS Help</source>
        <translation>
Ajuda do QGIS</translation>
    </message>
    <message>
        <source>&amp;Home</source>
        <translation>&amp;Início</translation>
    </message>
    <message>
        <source>Alt+H</source>
        <translation>Alt+H</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>&amp;Próximo</translation>
    </message>
    <message>
        <source>Alt+F</source>
        <translation>Alt+F</translation>
    </message>
    <message>
        <source>&amp;Back</source>
        <translation>&amp;Anterior</translation>
    </message>
    <message>
        <source>Alt+B</source>
        <translation>Alt+B</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Fechar</translation>
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
        <translation>O servidor respondeu inexperadamente com HTTP Status Código %1 (%2)</translation>
    </message>
    <message>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>Resposta HTTP completa, entretanto houve um erro: %1</translation>
    </message>
    <message>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>Transação HTTP completa, entretanto houve um erro: %1</translation>
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
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Inverse Distance Weighting&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400;&quot;&gt;o único parâmetro para o método de interpolação IDW é o coeficiente que descreve o decréscimo de pesos com a distância.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Distance coefficient P:</source>
        <translation>Distância para coeficiente P:</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResults</name>
    <message>
        <source>Identify Results - </source>
        <translation>Identifica resultados - </translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>Feição</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Valor</translation>
    </message>
    <message>
        <source>Run action</source>
        <translation>Rodar ação</translation>
    </message>
    <message>
        <source>(Derived)</source>
        <translation>(Derivado)</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <source>Identify Results</source>
        <translation>Identificar Resultados</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ajudar</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialog</name>
    <message>
        <source>Triangular interpolation (TIN)</source>
        <translation>Interpolação triangular (TIN)</translation>
    </message>
    <message>
        <source>Inverse Distance Weighting (IDW)</source>
        <translation>Peso pelo inverso da distância (IDW)</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialogBase</name>
    <message>
        <source>Output</source>
        <translation type="unfinished">Saída</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Interpolation plugin</source>
        <translation>Plugin de interpolação</translation>
    </message>
    <message>
        <source>Input</source>
        <translation>Entrada</translation>
    </message>
    <message>
        <source>Input vector layer</source>
        <translation>Entrar com camada vetorial</translation>
    </message>
    <message>
        <source>Use z-Coordinate for interpolation</source>
        <translation>Use Coordenada-z para interpolação</translation>
    </message>
    <message>
        <source>Interpolation attribute </source>
        <translation>Atributo de interpolação </translation>
    </message>
    <message>
        <source>Interpolation method</source>
        <translation>Método de interpolação</translation>
    </message>
    <message>
        <source>Number of columns</source>
        <translation>Número de colunas</translation>
    </message>
    <message>
        <source>Number of rows</source>
        <translation>Número de linhas</translation>
    </message>
    <message>
        <source>Output file </source>
        <translation>Arquivo de saída</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationPlugin</name>
    <message>
        <source>&amp;Interpolation</source>
        <translation>&amp;Interpolação</translation>
    </message>
</context>
<context>
    <name>QgsLUDialogBase</name>
    <message>
        <source>Enter class bounds</source>
        <translation>Entre o limite das classes</translation>
    </message>
    <message>
        <source>Lower value</source>
        <translation>Valor inferior</translation>
    </message>
    <message>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <source>Upper value</source>
        <translation>Valor superior</translation>
    </message>
</context>
<context>
    <name>QgsLabelDialogBase</name>
    <message>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <source>Preview:</source>
        <translation>Prévia:</translation>
    </message>
    <message>
        <source>QGIS Rocks!</source>
        <translation>QGIS é animal!</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Fonte</translation>
    </message>
    <message>
        <source>Points</source>
        <translation>Pontos</translation>
    </message>
    <message>
        <source>Map units</source>
        <translation>Unidade do Mapa</translation>
    </message>
    <message>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <source>Transparency:</source>
        <translation>Transparência:</translation>
    </message>
    <message>
        <source>Position</source>
        <translation>Posição</translation>
    </message>
    <message>
        <source>Size:</source>
        <translation>Tamanho:</translation>
    </message>
    <message>
        <source>Size is in map units</source>
        <translation>Tamanho em unidades do mapa</translation>
    </message>
    <message>
        <source>Size is in points</source>
        <translation>Tamanho em pontos</translation>
    </message>
    <message>
        <source>Above</source>
        <translation>Acima</translation>
    </message>
    <message>
        <source>Over</source>
        <translation>Sobre</translation>
    </message>
    <message>
        <source>Left</source>
        <translation>Esquerda</translation>
    </message>
    <message>
        <source>Below</source>
        <translation>Abaixo</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>Direita</translation>
    </message>
    <message>
        <source>Above Right</source>
        <translation>Sobre à Direira</translation>
    </message>
    <message>
        <source>Below Right</source>
        <translation>Abaixo à Direita</translation>
    </message>
    <message>
        <source>Above Left</source>
        <translation>Sobre à Esquerda</translation>
    </message>
    <message>
        <source>Below Left</source>
        <translation>Abaixo à Esquerda</translation>
    </message>
    <message>
        <source>Font size units</source>
        <translation>Tamanho da fonte</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Posicionamento</translation>
    </message>
    <message>
        <source>Buffer</source>
        <translation>Buffer</translation>
    </message>
    <message>
        <source>Buffer size units</source>
        <translation>Tamanho do Buffer</translation>
    </message>
    <message>
        <source>Offset units</source>
        <translation>unidade de Offset</translation>
    </message>
    <message>
        <source>&#xb0;</source>
        <translation type="obsolete">°</translation>
    </message>
    <message>
        <source>Field containing label</source>
        <translation>Campo que contém rótulo</translation>
    </message>
    <message>
        <source>Default label</source>
        <translation>Rótulo padrão</translation>
    </message>
    <message>
        <source>Data defined style</source>
        <translation>Estilo definido de dados</translation>
    </message>
    <message>
        <source>Data defined alignment</source>
        <translation>Alinhamento definido de dados </translation>
    </message>
    <message>
        <source>Data defined buffer</source>
        <translation>Buffer definido de dados</translation>
    </message>
    <message>
        <source>Data defined position</source>
        <translation>Posição definida de dados</translation>
    </message>
    <message>
        <source>Font transparency</source>
        <translation>Transparência fonte</translation>
    </message>
    <message>
        <source>Color</source>
        <translation type="unfinished">Cor</translation>
    </message>
    <message>
        <source>Angle (deg)</source>
        <translation>Ângulo (graus)</translation>
    </message>
    <message>
        <source>Buffer labels?</source>
        <translation>Rótulos de buffer?</translation>
    </message>
    <message>
        <source>Buffer size</source>
        <translation>Tamanho de buffer</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Transparência</translation>
    </message>
    <message>
        <source>X Offset (pts)</source>
        <translation>Offset X (pontos)</translation>
    </message>
    <message>
        <source>Y Offset (pts)</source>
        <translation>Offset Y (pontos)</translation>
    </message>
    <message>
        <source>&amp;Font family</source>
        <translation>&amp;Família fonte</translation>
    </message>
    <message>
        <source>&amp;Bold</source>
        <translation>&amp;Negrito</translation>
    </message>
    <message>
        <source>&amp;Italic</source>
        <translation>&amp;Itálico</translation>
    </message>
    <message>
        <source>&amp;Underline</source>
        <translation>&amp;Sublinhado</translation>
    </message>
    <message>
        <source>&amp;Size</source>
        <translation>&amp;Tamanho</translation>
    </message>
    <message>
        <source>Size units</source>
        <translation>Unidades de tamanho</translation>
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
        <translation>Rótulos multilinhas?</translation>
    </message>
    <message encoding="UTF-8">
        <source>°</source>
        <translation type="unfinished"></translation>
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
        <translation>&amp;Remover</translation>
    </message>
    <message>
        <source>&amp;Make to toplevel item</source>
        <translation>&amp;Colocar o item no topo</translation>
    </message>
    <message>
        <source>Re&amp;name</source>
        <translation>Re&amp;nomear</translation>
    </message>
    <message>
        <source>&amp;Add group</source>
        <translation>&amp;Adiciona grupo</translation>
    </message>
    <message>
        <source>&amp;Expand all</source>
        <translation>&amp;Expande tudo</translation>
    </message>
    <message>
        <source>&amp;Collapse all</source>
        <translation>&amp;Fecha tudo</translation>
    </message>
    <message>
        <source>Show file groups</source>
        <translation>Mostrar grupos de arquivos</translation>
    </message>
    <message>
        <source>No Layer Selected</source>
        <translation>Nenhuma Camada Selecionada</translation>
    </message>
    <message>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Para abrir uma tabela de atributos, você precisa selecionar uma camada vetorial na legenda</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Vizualizar a camada na sua extensão</translation>
    </message>
    <message>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation>&amp;Visualizar na melhor escala (100%)</translation>
    </message>
    <message>
        <source>&amp;Show in overview</source>
        <translation>&amp;Mostrar no overview</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Remover</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Abrir tabela de atributos</translation>
    </message>
    <message>
        <source>Save as shapefile...</source>
        <translation>Salvar como arquivo SHP...</translation>
    </message>
    <message>
        <source>Save selection as shapefile...</source>
        <translation>Salva a seleção como arquivo SHP</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation>&amp;Propriedades</translation>
    </message>
    <message>
        <source>More layers</source>
        <translation type="obsolete">Mais camadas</translation>
    </message>
    <message>
        <source>This item contains more layer files. Displaying more layers in table is not supported.</source>
        <translation type="obsolete">Este item contém mais arquivos de camadas. Exibir mais camadas na tabela não é suportado.</translation>
    </message>
    <message>
        <source>Multiple layers</source>
        <translation>Camadas multilinhas</translation>
    </message>
    <message>
        <source>This item contains multiple layers. Displaying multiple layers in the table is not supported.</source>
        <translation>Este item contém múltiplas camadas. Não é possível mostrar múltiplas camadas na tabela.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <source>Attribute table - </source>
        <translation type="obsolete">Tabela de atributos - </translation>
    </message>
    <message>
        <source>Save layer as...</source>
        <translation>Salvar camada como...</translation>
    </message>
    <message>
        <source>Start editing failed</source>
        <translation type="obsolete">Falha ao iniciar a edição</translation>
    </message>
    <message>
        <source>Provider cannot be opened for editing</source>
        <translation type="obsolete">Provedor não pode ser aberto para edição</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation type="obsolete">Parar edição</translation>
    </message>
    <message>
        <source>Do you want to save the changes?</source>
        <translation type="obsolete">Você quer salvar as alterações?</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Erro</translation>
    </message>
    <message>
        <source>Could not commit changes</source>
        <translation type="obsolete">Impossível marcar mudanças</translation>
    </message>
    <message>
        <source>Problems during roll back</source>
        <translation type="obsolete">Problemas durante retorno</translation>
    </message>
    <message>
        <source>Not a vector layer</source>
        <translation type="obsolete">Não é uma camada vetorial</translation>
    </message>
    <message>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation type="obsolete">Para abrir uma tabela de atributos, você precisa selecionar uma camada vetorial na legenda</translation>
    </message>
    <message>
        <source>Saving done</source>
        <translation>Salvar feito</translation>
    </message>
    <message>
        <source>Export to Shapefile has been completed</source>
        <translation>Exportar para SHP quando completado</translation>
    </message>
    <message>
        <source>Driver not found</source>
        <translation>Driver não encontrado</translation>
    </message>
    <message>
        <source>ESRI Shapefile driver is not available</source>
        <translation>Driver SHP da ESRI não está disponível</translation>
    </message>
    <message>
        <source>Error creating shapefile</source>
        <translation>Erro ao criar arquivo SHP</translation>
    </message>
    <message>
        <source>The shapefile could not be created (</source>
        <translation>O arquivo SHP não pode ser criado (</translation>
    </message>
    <message>
        <source>Layer creation failed</source>
        <translation>Falha na criação da camada</translation>
    </message>
    <message>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Visualizar para extensão da camada</translation>
    </message>
    <message>
        <source>&amp;Show in overview</source>
        <translation>&amp;Mostrar no overview</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Remover</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Abrir tabela de atributos</translation>
    </message>
    <message>
        <source>Save as shapefile...</source>
        <translation>Salvar como shapefile...</translation>
    </message>
    <message>
        <source>Save selection as shapefile...</source>
        <translation>Salva seleção como arquivo SHP...</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation>&amp;Propriedades</translation>
    </message>
    <message>
        <source>bad_alloc exception</source>
        <translation type="obsolete">bad_alloc exception</translation>
    </message>
    <message>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation type="obsolete">O preenchimento dos atributos de tabela parou por insuficiência de memória virtual (seu HD deve estar cheio)</translation>
    </message>
    <message>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation>A tabela de atributos de camada contém um tipo de dados não suportado</translation>
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
        <translation>Impossível desenhar</translation>
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
        <translation>%1 na linha %2 coluna %3</translation>
    </message>
    <message>
        <source>User database could not be opened.</source>
        <translation>Base de dados do usuário não pode ser aberta</translation>
    </message>
    <message>
        <source>The style table could not be created.</source>
        <translation>A tabela de estilos não pode ser criada.</translation>
    </message>
    <message>
        <source>The style %1 was saved to database</source>
        <translation>O estilo %1 foi salvo na base de dados</translation>
    </message>
    <message>
        <source>The style %1 was updated in the database.</source>
        <translation>O estilo %1 foi atualizado na base de dados.</translation>
    </message>
    <message>
        <source>The style %1 could not be updated in the database.</source>
        <translation>O estilo %1 não pode ser atualizado na base de dados.</translation>
    </message>
    <message>
        <source>The style %1 could not be inserted into database.</source>
        <translation>O estilo %1 não pode ser inserido na base de dados.</translation>
    </message>
    <message>
        <source>style not found in database</source>
        <translation>estilo não encontrado na base de dados</translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <source>(clicked coordinate)</source>
        <translation>(coordenada clicada)</translation>
    </message>
    <message>
        <source>WMS identify result for %1
%2</source>
        <translation>WMS identifica resultado para %1
%2</translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <source>Split error</source>
        <translation>Erro de partida</translation>
    </message>
    <message>
        <source>An error occured during feature splitting</source>
        <translation>Um erro ocorreu durante a arrancada</translation>
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
        <translation>Tolerância de aproximação</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished">Não mostra esta mensagem novamente</translation>
    </message>
    <message>
        <source>Could not snap segment.</source>
        <translation>Impossível aproximar seguimento</translation>
    </message>
    <message>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>Você configurou a tolerância em Configurações&gt;Propriedades do Projeto&gt;Geral?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <source>Name for the map file</source>
        <translation>Nome para o arquivo de mapa</translation>
    </message>
    <message>
        <source>Choose the QGIS project file</source>
        <translation>Escolha o arquivo de projeto QGIS</translation>
    </message>
    <message>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>Arquivos de projeto QGIS  (*.qgs);;Todos arquivos (*.*)</translation>
    </message>
    <message>
        <source>Overwrite File?</source>
        <translation>Sobrescrever arquivo?</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation> existe. 
Sobrescrevê-lo?</translation>
    </message>
    <message>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>MapServer arquivos de mapa (*.map);;Todos arquivos (*.*)</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>








a fileName is prepended to this text, and appears in a dialog box</comment>
        <translation type="obsolete"> existe. 
Sobrescrevê-lo?</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a fileName is prepended to this text, and appears in a dialog box</comment>
        <translation type="unfinished"> existe. 
Sobrescrevê-lo?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <source>Export to Mapserver</source>
        <translation>Exportar para Mapserver</translation>
    </message>
    <message>
        <source>Map file</source>
        <translation>Arquivo Map</translation>
    </message>
    <message>
        <source>Export LAYER information only</source>
        <translation>Exportar apenas a informação da CAMADA</translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Altura</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Largura</translation>
    </message>
    <message>
        <source>dd</source>
        <translation>dd</translation>
    </message>
    <message>
        <source>feet</source>
        <translation>pés</translation>
    </message>
    <message>
        <source>meters</source>
        <translation>metros</translation>
    </message>
    <message>
        <source>miles</source>
        <translation>milhas</translation>
    </message>
    <message>
        <source>inches</source>
        <translation>polegadas</translation>
    </message>
    <message>
        <source>kilometers</source>
        <translation>quilômetros</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Unidades</translation>
    </message>
    <message>
        <source>Image type</source>
        <translation>Tipo de imagem</translation>
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
        <translation>Definição do usuário</translation>
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
        <translation>Escala Máxima</translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation>Prefixo anexado ao mapa, barra de escalas e legendas arquivos GIF criados usando este MapFile. Deve ser mantido brevemente.</translation>
    </message>
    <message>
        <source>Web Interface Definition</source>
        <translation>Definição de interface Web </translation>
    </message>
    <message>
        <source>Header</source>
        <translation>Cabeçalho</translation>
    </message>
    <message>
        <source>Footer</source>
        <translation>Rodapé</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Modelo</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Ajuda</translation>
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
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation>Nome para o arquivo de mapa a ser criado de um arquivo de projeto QGIS</translation>
    </message>
    <message>
        <source>If checked, only the layer information will be processed</source>
        <translation>se verificado, apenas a informação da camada será processada</translation>
    </message>
    <message>
        <source>Path to the MapServer template file</source>
        <translation>Caminho para o arquivo modelo do MapServer</translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation>Prefixo anexado ao mapa, barra de escala e legenda GIF criados usando este arquivo de mapa</translation>
    </message>
    <message>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation>Caminho completo do arquivo de projeto QGIS para exportar para o formato MapServer</translation>
    </message>
    <message>
        <source>QGIS project file</source>
        <translation>Arquivo de projeto QGIS</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Procurar...</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Salvar como...</translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <source>Measure</source>
        <translation>Medida</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Novo</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ajuda</translation>
    </message>
    <message>
        <source>Cl&amp;ose</source>
        <translation>Fe&amp;char</translation>
    </message>
    <message>
        <source>Total:</source>
        <translation>Total:</translation>
    </message>
    <message>
        <source>Segments</source>
        <translation type="unfinished">Segmentos</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <source>Segments (in meters)</source>
        <translation>Seguimentos (em metros)</translation>
    </message>
    <message>
        <source>Segments (in feet)</source>
        <translation>Seguimentos (em pés)</translation>
    </message>
    <message>
        <source>Segments (in degrees)</source>
        <translation>Segmentos (em graus)</translation>
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
        <translation>Resultados de medidas incorretos</translation>
    </message>
    <message>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation>&lt;p&gt;Este mapa é definido com um sistema de coordenadas geográficas (latitude/longitude) porém os limites do mapa sugerem que este é efetivamente um sistema de coordenadas projetado (e.g., Mercator). Se for, os resultados de medidade de linhas ou áreas poderão estar incorretos.&lt;/p&gt;&lt;p&gt;Para definir este sistema de coordenadas, defina um mapa com sistema de coordenadas apropriado usando o menu&lt;tt&gt;Configurações:Propriedades do Projeto&lt;/tt&gt;.</translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <source>QGIS Message</source>
        <translation>Mensagem do QGIS</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Não mostra esta mensagem novamente</translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <source>Test connection</source>
        <translation>Testar conexão</translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>A conexão falhou. Revise sua configuração e tente novamente. 

Informações adicionais do erro:</translation>
    </message>
    <message>
        <source>Connection to %1 was successful</source>
        <translation>Conexão com %1 foi completada</translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <source>Create a New PostGIS connection</source>
        <translation>Criar nova conexão PostGIS</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ajuda</translation>
    </message>
    <message>
        <source>Connection Information</source>
        <translation>Informação da Conexão</translation>
    </message>
    <message>
        <source>Host</source>
        <translation>Máquina</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>Banco de Dados</translation>
    </message>
    <message>
        <source>Username</source>
        <translation>Usuário</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation>Nome da nova conexão</translation>
    </message>
    <message>
        <source>Password</source>
        <translation>Senha</translation>
    </message>
    <message>
        <source>Test Connect</source>
        <translation>Testar Conexão</translation>
    </message>
    <message>
        <source>Save Password</source>
        <translation>Salvar Senha</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>Porta</translation>
    </message>
    <message>
        <source>5432</source>
        <translation>5432</translation>
    </message>
    <message>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation>Apenas olhar no esquema público</translation>
    </message>
    <message>
        <source>Only look in the geometry_columns table</source>
        <translation>Apenas olhar na tabela geometry_columns</translation>
    </message>
    <message>
        <source>Restrict the search to the public schema for spatial tables not in the geometry_columns table</source>
        <translation>Restringe a busca para o esquema público para tabelas espaciais na tabela de geometria_de_colunas</translation>
    </message>
    <message>
        <source>When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)</source>
        <translation>Quando buscar por tabelas espaciais que não estão nas tabelas de geometrias_de_colunas, restrinja a busca para tabelas que estão no esquema público (para algumas base de dados este pode salvar porções de tempo)</translation>
    </message>
    <message>
        <source>Restrict the displayed tables to those that are in the geometry_columns table</source>
        <translation>Restringir tabelas mostradas para aquelas que estão na tabela geometria_de_colunas</translation>
    </message>
    <message>
        <source>Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.</source>
        <translation>Restringir tabelas mostradas para aquelas que estão na tabela geometria_de_colunas. Isto pode acelerar a visualização inicial de tabelas espaciais.</translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation>Nome da nova conexão</translation>
    </message>
    <message>
        <source>HTTP address of the Web Map Server</source>
        <translation>Endereço HTTP do Web Map Server</translation>
    </message>
    <message>
        <source>Create a new WMS connection</source>
        <translation>Cria uma nova conexão WMS</translation>
    </message>
    <message>
        <source>Connection details</source>
        <translation>Detalhes da conexão</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>Inferior Esquerdo</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Superior Direito</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Inferior Direito</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Superior Esquerdo</translation>
    </message>
    <message>
        <source>&amp;North Arrow</source>
        <translation>&amp;Seta Norte</translation>
    </message>
    <message>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>Cria uma seta norte que será mostrada na tela.</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Decorações</translation>
    </message>
    <message>
        <source>North arrow pixmap not found</source>
        <translation>Pixmap da seta norte não encontrado</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <source>Pixmap not found</source>
        <translation>Pixmap não encontrado</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <source>North Arrow Plugin</source>
        <translation>Plugin de Rosa dos Ventos</translation>
    </message>
    <message>
        <source>Properties</source>
        <translation>Propriedades</translation>
    </message>
    <message>
        <source>Angle</source>
        <translation>Ângulo</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Posicionamento</translation>
    </message>
    <message>
        <source>Set direction automatically</source>
        <translation>Configurar a direção automaticamente</translation>
    </message>
    <message>
        <source>Enable North Arrow</source>
        <translation>Habilitar Rosa dos Ventos</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Superior Esquerdo</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Superior Direito</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Inferior Esquerdo</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Inferior Direito</translation>
    </message>
    <message>
        <source>Placement on screen</source>
        <translation>Posicionamento na Tela</translation>
    </message>
    <message>
        <source>Preview of north arrow</source>
        <translation>Pré-visualização da rosa dos ventos</translation>
    </message>
    <message>
        <source>Icon</source>
        <translation>Ícone</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Buscar...</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <source>Detected active locale on your system: </source>
        <translation>Detecta localização ativa do seu sistema: </translation>
    </message>
    <message>
        <source>to vertex</source>
        <translation>para vértice</translation>
    </message>
    <message>
        <source>to segment</source>
        <translation>para segmento</translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation>para vértice e segmento</translation>
    </message>
    <message>
        <source>Semi transparent circle</source>
        <translation>Círculo semi-transparente</translation>
    </message>
    <message>
        <source>Cross</source>
        <translation>Cruzar</translation>
    </message>
    <message>
        <source>Show all features</source>
        <translation type="obsolete">Mostrar todas feições</translation>
    </message>
    <message>
        <source>Show selected features</source>
        <translation type="obsolete">Mostrar feições selecionadas</translation>
    </message>
    <message>
        <source>Show features in current canvas</source>
        <translation type="obsolete">Mostrar feições presentes na tela atual</translation>
    </message>
</context>
<context>
    <name>QgsOptionsBase</name>
    <message>
        <source>QGIS Options</source>
        <translation>Opções do QGIS</translation>
    </message>
    <message>
        <source>Hide splash screen at startup</source>
        <translation>Não exibir a tela inicial (splash screen)</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Nota:&lt;/b&gt;Mudanças no tema somente terão efeito na próxima vez que o QGIS for iniciado</translation>
    </message>
    <message>
        <source>&amp;Rendering</source>
        <translation>&amp;Renderizar</translation>
    </message>
    <message>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>A visualização do mapa será atualizada (desenhada) após este número de feições ter sido lido</translation>
    </message>
    <message>
        <source>Select Global Default ...</source>
        <translation>Selecione o Padrão Global</translation>
    </message>
    <message>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>Faz as linhas apareceram com menos definição para não perder performance do desenho</translation>
    </message>
    <message>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Por padrão novas camadas adicionadas ao mapa podem ser mostradas</translation>
    </message>
    <message>
        <source>Measure tool</source>
        <translation>Ferramenta de medida</translation>
    </message>
    <message>
        <source>Search radius</source>
        <translation>Procurar raio</translation>
    </message>
    <message>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Problemas fixos com polígonos incorretamente preenchidos</translation>
    </message>
    <message>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Continuamente redesenhar o mapa quando arrastar a divisor legenda/mapa</translation>
    </message>
    <message>
        <source>&amp;Map tools</source>
        <translation>&amp;Ferramentas de mapas</translation>
    </message>
    <message>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <source>Panning and zooming</source>
        <translation>Movendo e aproximando/afastando</translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation>Visualizar</translation>
    </message>
    <message>
        <source>Zoom and recenter</source>
        <translation>Visualizar e centralizar</translation>
    </message>
    <message>
        <source>Nothing</source>
        <translation>Nada</translation>
    </message>
    <message>
        <source>&amp;General</source>
        <translation>&amp;Geral</translation>
    </message>
    <message>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation>Selecionar isso irá deselecionar as linhas feitas menos as marcadas</translation>
    </message>
    <message>
        <source>Zoom to mouse cursor</source>
        <translation>Aproximar ao cursor do mouse</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation type="unfinished">Digitalizar</translation>
    </message>
    <message>
        <source>Snapping</source>
        <translation>Atrair</translation>
    </message>
    <message>
        <source>Rubberband</source>
        <translation>Elástico</translation>
    </message>
    <message>
        <source>Line width in pixels</source>
        <translation type="unfinished">Espessura da linha em pixels </translation>
    </message>
    <message>
        <source>Locale</source>
        <translation>Região</translation>
    </message>
    <message>
        <source>Locale to use instead</source>
        <translation>Região usada ao invés de</translation>
    </message>
    <message>
        <source>Additional Info</source>
        <translation>Informação adicional</translation>
    </message>
    <message>
        <source>Detected active locale on your system:</source>
        <translation>Região detectada no seu sistema:</translation>
    </message>
    <message>
        <source>Project files</source>
        <translation>Arquivos de projeto</translation>
    </message>
    <message>
        <source>Prompt to save project changes when required</source>
        <translation>Apto a salvar projeto quando requerido</translation>
    </message>
    <message>
        <source>Warn when opening a project file saved with an older version of QGIS</source>
        <translation>Lembrar quando abrir um projeto salvo com uma versão antiga do QGIS</translation>
    </message>
    <message>
        <source>Default Map Appearance (overridden by project properties)</source>
        <translation>Aparência padrão do Mapa (overriden pelas propriedades do projeto)</translation>
    </message>
    <message>
        <source>Selection color</source>
        <translation>Seleção de cor</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Cor de fundo</translation>
    </message>
    <message>
        <source>&amp;Application</source>
        <translation>&amp;Aplicação</translation>
    </message>
    <message>
        <source>Icon theme</source>
        <translation>Tema de ícones</translation>
    </message>
    <message>
        <source>Capitalise layer names in legend</source>
        <translation>&quot;Captalise&quot; nomes de camadas na legenda</translation>
    </message>
    <message>
        <source>Display classification attribute names in legend</source>
        <translation>Mostrar nomes de atributos de classificação na legenda</translation>
    </message>
    <message>
        <source>Rendering behavior</source>
        <translation>Renderizar comportamento</translation>
    </message>
    <message>
        <source>Number of features to draw before updating the display</source>
        <translation>Números de feições para desenhar antes de atualizar a visualização</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Use zero to prevent display updates until all features have been rendered</source>
        <translation>&lt;b&gt;Note:&lt;/b&gt; Use zero para prevenir mostrar atualizações até que todas as feições sejam renderizadas</translation>
    </message>
    <message>
        <source>Rendering quality</source>
        <translation>Qualidade da renderização</translation>
    </message>
    <message>
        <source>Zoom factor</source>
        <translation>Fator de aproximação</translation>
    </message>
    <message>
        <source>Mouse wheel action</source>
        <translation>Ação da roda do mouse</translation>
    </message>
    <message>
        <source>Rubberband color</source>
        <translation>Cor do elástico</translation>
    </message>
    <message>
        <source>Ellipsoid for distance calculations</source>
        <translation>Elipsóide para cálculos de distância</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Specify the search radius as a percentage of the map width</source>
        <translation>&lt;b&gt;Note:&lt;/b&gt; Especificar o raio de busca como uma porcentagem da largura do mapa</translation>
    </message>
    <message>
        <source>Search radius for identifying features and displaying map tips</source>
        <translation>Procurar raio para identificar feições e mostrar no mapa</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation type="unfinished">Largura da linha</translation>
    </message>
    <message>
        <source>Line colour</source>
        <translation>Cor da linha</translation>
    </message>
    <message>
        <source>Default snap mode</source>
        <translation>Modo de aproximação padrão</translation>
    </message>
    <message>
        <source>Default snapping tolerance in layer units</source>
        <translation>Tolerância de aproximação padrão na unidade da camada</translation>
    </message>
    <message>
        <source>Search radius for vertex edits in layer units</source>
        <translation>Procurar raio para edição de vértice na unidade da camada</translation>
    </message>
    <message>
        <source>Vertex markers</source>
        <translation>Marcadores de vértice</translation>
    </message>
    <message>
        <source>Marker style</source>
        <translation>Estilo de marcadores</translation>
    </message>
    <message>
        <source>Override system locale</source>
        <translation>Sobrepor região do sistema</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Enabling / changing overide on local requires an application restart</source>
        <translation>&lt;b&gt;Note:&lt;/b&gt; Habilitar / trocar a região requer reiniciar a aplicação</translation>
    </message>
    <message>
        <source>Proxy</source>
        <translation>Proxy</translation>
    </message>
    <message>
        <source>Use proxy for web access</source>
        <translation>Usar proxy para acessar a web</translation>
    </message>
    <message>
        <source>Host</source>
        <translation>Hospedeiro</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="unfinished">Porta</translation>
    </message>
    <message>
        <source>User</source>
        <translation>Usuário</translation>
    </message>
    <message>
        <source>Leave this blank if no proxy username / password are required</source>
        <translation>Deixe este branco se não é requerido usuário ou senha para o proxy</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="unfinished">Senha</translation>
    </message>
    <message>
        <source>Open attribute table in a dock window</source>
        <translation>Abrir tabela de atributos em nova janela</translation>
    </message>
    <message>
        <source>Attribute table behaviour</source>
        <translation type="obsolete">Comportamento da tebela de atributos</translation>
    </message>
    <message>
        <source>CRS</source>
        <translation>SRC</translation>
    </message>
    <message>
        <source>When layer is loaded that has no coordinate reference system (CRS)</source>
        <translation>Quando a camada é carregada não possui sistema de referência de coordenadas (SRC)</translation>
    </message>
    <message>
        <source>Prompt for CRS</source>
        <translation>Apto para SRC</translation>
    </message>
    <message>
        <source>Project wide default CRS will be used</source>
        <translation>Será usada SRC ampla como padrão</translation>
    </message>
    <message>
        <source>Global default CRS displa&amp;yed below will be used</source>
        <translation>SRC global padrão mos&amp;trada abaixo será usada</translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <source>Paste Transformations</source>
        <translation>Colar transformações</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation>&lt;b&gt;Note: Esta função não pode ser usada ainda!&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Source</source>
        <translation>Fonte</translation>
    </message>
    <message>
        <source>Destination</source>
        <translation>Destino</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Ajuda</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Add New Transfer</source>
        <translation>Adicionar nova transferência</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <source>Buffer features in layer %1</source>
        <translation>Feições de buffer na camada %1</translation>
    </message>
    <message>
        <source>Error connecting to the database</source>
        <translation>Erro na conexão com o banco de dados</translation>
    </message>
    <message>
        <source>&amp;Buffer features</source>
        <translation>&amp;Feições do Buffer</translation>
    </message>
    <message>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation>Uma nova camada é criada na base de dados com as feições &apos;bufferadas&apos;</translation>
    </message>
    <message>
        <source>&amp;Geoprocessing</source>
        <translation>&amp;Geoprocessamento</translation>
    </message>
    <message>
        <source>Unable to add geometry column</source>
        <translation>Impossível adicionar geometria de coluna</translation>
    </message>
    <message>
        <source>Unable to add geometry column to the output table </source>
        <translation>Impossível adicionar geometria de coluna para a tabela de saída </translation>
    </message>
    <message>
        <source>Unable to create table</source>
        <translation>Impossível criar tabela</translation>
    </message>
    <message>
        <source>Failed to create the output table </source>
        <translation>Falha ao criar tabela de saída</translation>
    </message>
    <message>
        <source>No GEOS support</source>
        <translation>Sem suporte GEOS</translation>
    </message>
    <message>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation>Função buffer requer suporte GEOS no PostGIS</translation>
    </message>
    <message>
        <source>No Active Layer</source>
        <translation>Sem camada ativa</translation>
    </message>
    <message>
        <source>You must select a layer in the legend to buffer</source>
        <translation>Você deve selecionar uma camada na legenda para &apos;buffer&apos;</translation>
    </message>
    <message>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation>Cria um buffer para uma camada PostgreSQL</translation>
    </message>
    <message>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation>Não é uma camada PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation> não é uma camada PostgreSQL/PostGIS layer.
</translation>
    </message>
    <message>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation>Geoprocessar funções é apenas possível para camadas PostgreSQL/PostGIS</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Tabela &lt;b&gt;%1&lt;/b&gt; no banco de dados &lt;b&gt;%2&lt;/b&gt; no servidor &lt;b&gt;%3&lt;/b&gt;, usuário &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Connection Failed</source>
        <translation>A Conexão Falhou</translation>
    </message>
    <message>
        <source>Connection to the database failed:</source>
        <translation>Conexão com o banco de dados falhou:</translation>
    </message>
    <message>
        <source>Database error</source>
        <translation>Erro no banco de dados</translation>
    </message>
    <message>
        <source>Query Result</source>
        <translation>Resultado da Consulta</translation>
    </message>
    <message>
        <source>The where clause returned </source>
        <translation>A cláusula where retornou</translation>
    </message>
    <message>
        <source> rows.</source>
        <translation> colunas.</translation>
    </message>
    <message>
        <source>Query Failed</source>
        <translation>A Consulta Falhou</translation>
    </message>
    <message>
        <source>An error occurred when executing the query:</source>
        <translation>Um erro ocorreu quando foi executada a consulta:</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation>Nenhum registro encontrado</translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>A consulta que você especificou retornou que não há nenhum registro encontrado. Camadas válidas do PostgreSQL precisam ter pelo menos uma feição.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Falha ao obter amostra de valores de campo usando SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <source>No Query</source>
        <translation>Sem pergunta</translation>
    </message>
    <message>
        <source>You must create a query before you can test it</source>
        <translation>Você deve criar uma pergunta antes de testá-la</translation>
    </message>
    <message>
        <source>Error in Query</source>
        <translation>Erro na pergunta</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <source>PostgreSQL Query Builder</source>
        <translation>Construtor de Consulta PostgreSQL</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Limpar</translation>
    </message>
    <message>
        <source>Test</source>
        <translation>Testar</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>Ok</translation>
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
        <translation>Tudo</translation>
    </message>
    <message>
        <source>Sample</source>
        <translation>Amostra</translation>
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
        <translation>Cláusula SQL where</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Retrieve &lt;span style=&quot; font-weight:600;&quot;&gt;all&lt;/span&gt; the record in the vector file (&lt;span style=&quot; font-style:italic;&quot;&gt;if the table is big, the operation can consume some time&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0pxResgatarve &lt;span style=&quot; font-weight:600todosall&lt;/spaos registros no arquivo vetorialile (&lt;span style=&quot; font-style:italicquanto maior for a tabela, maior será o tempo de operaçãoime&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Take a &lt;span style=&quot; font-weight:600;&quot;&gt;sample&lt;/span&gt; of records in the vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Aceitar &lt;span style=&quot; font-weight:600;&quot;&gt;amostras&lt;/span&gt; de registros no arquivo vetorial&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of values for the current field.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Lista de valores para o campo atual.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of fields in this vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Lista de campneste arquivo vetorialos &lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Datasource</source>
        <translation>Fonte de dados</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerDialog</name>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation>Instalador do Plugin Python</translation>
    </message>
    <message>
        <source>QGIS Plugin Installer</source>
        <translation>Instalador de plugin do QGIS</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation type="unfinished">Plugins</translation>
    </message>
    <message>
        <source>List of available and installed plugins</source>
        <translation>Lista de plugins disponíveis e instalados</translation>
    </message>
    <message>
        <source>Filter:</source>
        <translation>Filtro:</translation>
    </message>
    <message>
        <source>Display only plugins containing this word in their metadata</source>
        <translation>Mostra apenas plugins que contenham esta palavra em suas informações</translation>
    </message>
    <message>
        <source>Display only plugins from given repository</source>
        <translation>Mostra apenas plugins de um dado repositório</translation>
    </message>
    <message>
        <source>all repositories</source>
        <translation>todos repositórios</translation>
    </message>
    <message>
        <source>Display only plugins with matching status</source>
        <translation>Mostra apenas plugins com situação combinada</translation>
    </message>
    <message>
        <source>Status</source>
        <translation type="unfinished">Situação</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">Nome</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="unfinished">Versão</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="unfinished">Descrição</translation>
    </message>
    <message>
        <source>Author</source>
        <translation>Autor</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation>Repositório</translation>
    </message>
    <message>
        <source>Install, reinstall or upgrade the selected plugin</source>
        <translation>Instala, reinstala ou atualiza o plugin selecionado</translation>
    </message>
    <message>
        <source>Install/upgrade plugin</source>
        <translation>Instala/atualiza plugin</translation>
    </message>
    <message>
        <source>Uninstall the selected plugin</source>
        <translation>Desinstala o plugin selecionado</translation>
    </message>
    <message>
        <source>Uninstall plugin</source>
        <translation>Desinstala plugin</translation>
    </message>
    <message>
        <source>Repositories</source>
        <translation>Repositórios</translation>
    </message>
    <message>
        <source>List of plugin repositories</source>
        <translation>Lista de repositórios de plugins</translation>
    </message>
    <message>
        <source>URL</source>
        <translation type="unfinished">URL</translation>
    </message>
    <message>
        <source>Allow the Installer to look for updates and news in enabled repositories on QGIS startup</source>
        <translation>Permitir o instalador localizar atualizações e novidades em repositórios habilitados quando o QGIS iniciar</translation>
    </message>
    <message>
        <source>Check for updates on startup</source>
        <translation>Verificar por atualizações quando iniciar</translation>
    </message>
    <message>
        <source>Add third party plugin repositories to the list</source>
        <translation>Adiciona terceiro grupo de repositórios de plugins à lista</translation>
    </message>
    <message>
        <source>Add 3rd party repositories</source>
        <translation>Adicionar terceiro grupo de repositórios</translation>
    </message>
    <message>
        <source>Add a new plugin repository</source>
        <translation>Adiciona um novo repositório de plugin</translation>
    </message>
    <message>
        <source>Add...</source>
        <translation>Adicionar...</translation>
    </message>
    <message>
        <source>Edit the selected repository</source>
        <translation>Edita o repositório selecionado</translation>
    </message>
    <message>
        <source>Edit...</source>
        <translation>Editar...</translation>
    </message>
    <message>
        <source>Remove the selected repository</source>
        <translation>Remove o repositório selecionado</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Exclui</translation>
    </message>
    <message>
        <source>The plugins will be installed to ~/.qgis/python/plugins</source>
        <translation>Os plugins serão instalados em 
~/.qgis/python/plugins</translation>
    </message>
    <message>
        <source>Close the Installer window</source>
        <translation>Fechar a janela do instalador</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="unfinished">Fechar</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerFetchingDialog</name>
    <message>
        <source>Fetching repositories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Overall progress:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Abort fetching</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Repository</source>
        <translation type="unfinished">Repositório</translation>
    </message>
    <message>
        <source>State</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerInstallingDialog</name>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation>Instalador do Plugin Python</translation>
    </message>
    <message>
        <source>Installing plugin:</source>
        <translation>Instalar plugin:</translation>
    </message>
    <message>
        <source>Connecting...</source>
        <translation>Conectando...</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerPluginErrorDialog</name>
    <message>
        <source>Error loading plugin</source>
        <translation>Erro ao carregar plugin</translation>
    </message>
    <message>
        <source>The plugin seems to be invalid or have unfulfilled dependencies. It has been installed, but can&apos;t be loaded. If you really need this plugin, you can contact its author or &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;QGIS users group&lt;/a&gt; and try to solve the problem. If not, you can just uninstall it. Here is the error message below:</source>
        <translation>O plugin parece ser inválido ou tem dependências não preenchidas. Ele pode estar instalado, mas não foi carregado. Se você realmente necessita desse plugin você terá de consultar o autor ou &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;QGIS users group&lt;/a&gt; e tentar resolver o problema. Se não for usar, desinstale-o. Aqui está a mensagem de erro:</translation>
    </message>
    <message>
        <source>Do you want to uninstall this plugin now? If you&apos;re unsure, probably you would like to do this.</source>
        <translation>Você gostaria de desinstalar este plugin agora? Se você está em dúvida, provavelmente você deve fazer isso.</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerRepositoryDetailsDialog</name>
    <message>
        <source>Repository details</source>
        <translation>Detalhes do repositório</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation type="unfinished">Nome:</translation>
    </message>
    <message>
        <source>Enter a name for the repository</source>
        <translation>Entre com um nome para o repositório</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <source>Enter the repository URL, beginning with &quot;http://&quot;</source>
        <translation>Entre com a URL do repositório, iniciar com &quot;http://&quot;</translation>
    </message>
    <message>
        <source>Enable or disable the repository (disabled repositories will be omitted)</source>
        <translation>Habilita ou desabilita o repositório (repositórios desabilitados serão omitidos)</translation>
    </message>
    <message>
        <source>Enabled</source>
        <translation>Habilitado</translation>
    </message>
    <message>
        <source>[place for a warning message]</source>
        <translation>[lugar para uma mensagem de alerta] </translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <source>No Plugins</source>
        <translation>Sem Plugins</translation>
    </message>
    <message>
        <source>No QGIS plugins found in </source>
        <translation>Nenhum plugin QGIS encontrado em</translation>
    </message>
    <message>
        <source>&amp;Select All</source>
        <translation type="unfinished">&amp;Selecionar Tudo</translation>
    </message>
    <message>
        <source>&amp;Clear All</source>
        <translation>&amp;Apagar tudo</translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <source>QGIS Plugin Manager</source>
        <translation>Gerenciador de Plugins do QGIS</translation>
    </message>
    <message>
        <source>To enable / disable a plugin, click its checkbox or description</source>
        <translation>Para habilitar . desabilitar um plugin, clique em sua caixa de seleção ou descrição</translation>
    </message>
    <message>
        <source>&amp;Filter</source>
        <translation>&amp;Filtro</translation>
    </message>
    <message>
        <source>Plugin Directory:</source>
        <translation>Diretório de Plugin:</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation type="unfinished">Diretório</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <source>Zoom In</source>
        <translation>Aproximar</translation>
    </message>
    <message>
        <source>z</source>
        <translation>z</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Afastar</translation>
    </message>
    <message>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <source>Zoom To Layer</source>
        <translation>Visualizar a camada</translation>
    </message>
    <message>
        <source>Zoom to Layer</source>
        <translation>Visualiza a camada</translation>
    </message>
    <message>
        <source>Pan Map</source>
        <translation>Panoramica no mapa</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation>Panoramica no mapa</translation>
    </message>
    <message>
        <source>Add Point</source>
        <translation>Adicionar ponto</translation>
    </message>
    <message>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <source>Capture Points</source>
        <translation>Capturar pontos</translation>
    </message>
    <message>
        <source>Delete Point</source>
        <translation>Excluir ponto</translation>
    </message>
    <message>
        <source>Delete Selected</source>
        <translation>Excluir selecionado</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Linear</translation>
    </message>
    <message>
        <source>Helmert</source>
        <translation>Helmert</translation>
    </message>
    <message>
        <source>Choose a name for the world file</source>
        <translation>Escolha um nome para o arquivo &apos;world&apos;</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Atenção</translation>
    </message>
    <message>
        <source>Affine</source>
        <translation>Aumentar precisão</translation>
    </message>
    <message>
        <source>Not implemented!</source>
        <translation>Não implementado!</translation>
    </message>
    <message>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Uma transformação precisa requer mudanças no raster original. Isto ainda não é suportado.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt;A </translation>
    </message>
    <message>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation> transformação ainda não é suportada.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Erro</translation>
    </message>
    <message>
        <source>Could not write to </source>
        <translation>Impossível gravar para </translation>
    </message>
    <message>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>Atualmente todos arquivos modificados serão gravados no formato TIFF.</translation>
    </message>
    <message>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation>&lt;p&gt;A transformação Helmert requer modificações na camada raster.&lt;/p&gt;&lt;p&gt;O raster modificado será salvo com um outro nome e o arquivo WLD será gerado para este novo arquivo.&lt;/p&gt;&lt;p&gt;Você tem certeza de que é isso que você quer fazer?&lt;/p&gt;</translation>
    </message>
    <message>
        <source>-modified</source>
        <comment>








Georeferencer:QgsPointDialog.cpp - used to modify a user given file name</comment>
        <translation type="obsolete">-modificado</translation>
    </message>
    <message>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given file name</comment>
        <translation type="unfinished">-modificado</translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <source>Transform type:</source>
        <translation>Tipo de Transformação:</translation>
    </message>
    <message>
        <source>Zoom in</source>
        <translation>Aumentar Zoom</translation>
    </message>
    <message>
        <source>Zoom out</source>
        <translation>Menos Zoom</translation>
    </message>
    <message>
        <source>Zoom to the raster extents</source>
        <translation>Zoom para a extensão do raster</translation>
    </message>
    <message>
        <source>Pan</source>
        <translation>Movimentar</translation>
    </message>
    <message>
        <source>Add points</source>
        <translation>Adicionar pontos</translation>
    </message>
    <message>
        <source>Delete points</source>
        <translation>Apagar pontos</translation>
    </message>
    <message>
        <source>World file:</source>
        <translation>World file:</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Modified raster:</source>
        <translation>Raster modificado:</translation>
    </message>
    <message>
        <source>Reference points</source>
        <translation>Pontos de referência</translation>
    </message>
    <message>
        <source>Create</source>
        <translation>Criar</translation>
    </message>
    <message>
        <source>Create and load layer</source>
        <translation>Criar e carregar camada</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <source>Unable to access relation</source>
        <translation>Impossível de acessar relação</translation>
    </message>
    <message>
        <source>Unable to access the </source>
        <translation>Impossível de acessar o </translation>
    </message>
    <message>
        <source> relation.
The error message from the database was:
</source>
        <translation> relação. 
A mensagem de erro do banco de dados foi:</translation>
    </message>
    <message>
        <source>No suitable key column in table</source>
        <translation>Sem chave de coluna adequada na tabela</translation>
    </message>
    <message>
        <source>The table has no column suitable for use as a key.

Qgis requires that the table either has a column of type
int4 with a unique constraint on it (which includes the
primary key) or has a PostgreSQL oid column.
</source>
        <translation>A tabela não possui coluna adequada para ser usada como uma chave. 

Qgis requer que qualquer tabela tenha uma coluna tipo int4 com um único &apos;constraint&apos; nela (deve incluir chave primária) ou ter uma coluna &apos;oid&apos; PostgreSQL.</translation>
    </message>
    <message>
        <source>The unique index on column</source>
        <translation>O único índice na coluna</translation>
    </message>
    <message>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation>é inadequado porque Qgis ainda não possui suporte a tipos de colunas diferentes de int4 como chave na tabela.</translation>
    </message>
    <message>
        <source>and </source>
        <translation>e </translation>
    </message>
    <message>
        <source>The unique index based on columns </source>
        <translation>O único índice baseado em colunas </translation>
    </message>
    <message>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation>é inadequado porque Qgis ainda não possui suporte a tipos de colunas diferentes de int4 como chave na tabela.</translation>
    </message>
    <message>
        <source>Unable to find a key column</source>
        <translation>Impossível encontrar uma coluna chave</translation>
    </message>
    <message>
        <source> derives from </source>
        <translation> derivada de </translation>
    </message>
    <message>
        <source>and is suitable.</source>
        <translation>e é adequado.</translation>
    </message>
    <message>
        <source>and is not suitable </source>
        <translation>e é inadequado </translation>
    </message>
    <message>
        <source>type is </source>
        <translation>tipo é </translation>
    </message>
    <message>
        <source> and has a suitable constraint)</source>
        <translation> e tem uma &apos;constraint&apos; adequada)</translation>
    </message>
    <message>
        <source> and does not have a suitable constraint)</source>
        <translation> e não possui uma &apos;constraint&apos;adequada)</translation>
    </message>
    <message>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation>A exibição selecionada tem as seguintes colunas, nenhum dos quais preenchem as condições acima:</translation>
    </message>
    <message>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation>Qgis exige que a visao tem uma coluna que pode ser usado como uma chave única. Essa coluna deve ser obtido a partir de uma tabela coluna do tipo int4 e ser uma chave primária, têm um único constrangimento sobre o mesmo, ou seja um PostgreSQL oid coluna. Para melhorar o desempenho da coluna também deve ser indexada.</translation>
    </message>
    <message>
        <source>The view </source>
        <translation>A visão</translation>
    </message>
    <message>
        <source>has no column suitable for use as a unique key.
</source>
        <translation>não tem colunas adequadas para usar uma chave única.</translation>
    </message>
    <message>
        <source>No suitable key column in view</source>
        <translation>Sem chave de coluna adequada na visão</translation>
    </message>
    <message>
        <source>Unknown geometry type</source>
        <translation>Tipo de geometria desconhecida</translation>
    </message>
    <message>
        <source>Column </source>
        <translation>Coluna</translation>
    </message>
    <message>
        <source> in </source>
        <translation> em </translation>
    </message>
    <message>
        <source> has a geometry type of </source>
        <translation> tem uma geometria do tipo </translation>
    </message>
    <message>
        <source>, which Qgis does not currently support.</source>
        <translation>, nas quais Qgis não possui ainda suporte.</translation>
    </message>
    <message>
        <source>. The database communication log was:
</source>
        <translation>. O registro de comunicação da base de dados foi:</translation>
    </message>
    <message>
        <source>Unable to get feature type and srid</source>
        <translation>Impossível obter tipos de feições e &apos;srid&apos;</translation>
    </message>
    <message>
        <source>Note: </source>
        <translation>Nota: </translation>
    </message>
    <message>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation>inicialmente parecia adequada mas não contém dados únicos, então não é adequado.</translation>
    </message>
    <message>
        <source>Unable to determine table access privileges for the </source>
        <translation>Impossível determinar privilégios de acesso a tabela para o </translation>
    </message>
    <message>
        <source>Error while adding features</source>
        <translation>Erro durante a adição de feições</translation>
    </message>
    <message>
        <source>Error while deleting features</source>
        <translation>Erro durante a exclusão de feições</translation>
    </message>
    <message>
        <source>Error while adding attributes</source>
        <translation>Erro durante a adição de atributos</translation>
    </message>
    <message>
        <source>Error while deleting attributes</source>
        <translation>Erro durante a exclusão de atributos</translation>
    </message>
    <message>
        <source>Error while changing attributes</source>
        <translation>Erro durante mudança de atributos</translation>
    </message>
    <message>
        <source>Error while changing geometry values</source>
        <translation>Erro durante mudança de valores de geometria</translation>
    </message>
    <message>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation>Impossível para o QGIS determinar o tipo e SRID da coluna</translation>
    </message>
    <message>
        <source>unexpected PostgreSQL error</source>
        <translation>erro não esperado de PostgreSQL</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider::Conn</name>
    <message>
        <source>No GEOS Support!</source>
        <translation type="unfinished">Sem suporte ao GEOS!</translation>
    </message>
    <message>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>Sua instalação do PostGIS suporta GEOS. 
Feições de seleção e identificação podem funcionar inadequadamente. 
Por favor, instale o PostGIS com suporte GEOS (http://geos.refractions.net)</translation>
    </message>
</context>
<context>
    <name>QgsProjectPropertiesBase</name>
    <message>
        <source>Project Properties</source>
        <translation>Propriedades do Projeto</translation>
    </message>
    <message>
        <source>Meters</source>
        <translation>Metros</translation>
    </message>
    <message>
        <source>Feet</source>
        <translation>Pés</translation>
    </message>
    <message>
        <source>Decimal degrees</source>
        <translation>Graus Decimais</translation>
    </message>
    <message>
        <source>Default project title</source>
        <translation>Título padrão do projeto</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Geral</translation>
    </message>
    <message>
        <source>Automatic</source>
        <translation>Automática</translation>
    </message>
    <message>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>Ajustar automaticamente o número de casas decimais na posição (geográfica) mostrada pelo mouse</translation>
    </message>
    <message>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>O número de casas decimais que são usadas para mostrar a posição (geográfica) do mouse é automaticamente ajustada de forma a ser suficientemente precisa, dessa forma, ao mover na tela o mouse em um pixel resulta em uma mudança na posição.</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation>Manual</translation>
    </message>
    <message>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Ajusta o número de casas decimais á ser usada para mostrar a posição (geográfica) do mouse</translation>
    </message>
    <message>
        <source>The number of decimal places for the manual option</source>
        <translation>O número de casas decimais para a opção manual</translation>
    </message>
    <message>
        <source>decimal places</source>
        <translation>casas decimais</translation>
    </message>
    <message>
        <source>Precision</source>
        <translation>Precisão</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitalizar</translation>
    </message>
    <message>
        <source>Descriptive project name</source>
        <translation>Descreve o nome do projeto</translation>
    </message>
    <message>
        <source>Enable topological editing</source>
        <translation>Habilita edição de topologia</translation>
    </message>
    <message>
        <source>Snapping options...</source>
        <translation>Opções de ajuste...</translation>
    </message>
    <message>
        <source>Avoid intersections of new polygons</source>
        <translation>Interseções vazias de novos polígonos</translation>
    </message>
    <message>
        <source>Title and colors</source>
        <translation>Título e cores</translation>
    </message>
    <message>
        <source>Project title</source>
        <translation>Título do Projeto</translation>
    </message>
    <message>
        <source>Selection color</source>
        <translation>Seleção de cor</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Cor do plano de fundo</translation>
    </message>
    <message>
        <source>Map units</source>
        <translation type="unfinished">Unidade do Mapa</translation>
    </message>
    <message>
        <source>Coordinate Reference System (CRS)</source>
        <translation>Sistema de referência de coordenadas (SRC)</translation>
    </message>
    <message>
        <source>Enable &apos;on the fly&apos; CRS transformation</source>
        <translation>Habilita transformação SRC &quot;on the fly&quot;</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelector</name>
    <message>
        <source>User Defined Coordinate Systems</source>
        <translation>Sistema de coordenadas definida pelo usuário</translation>
    </message>
    <message>
        <source>Geographic Coordinate Systems</source>
        <translation>Sistema de coordenadas geográficas</translation>
    </message>
    <message>
        <source>Projected Coordinate Systems</source>
        <translation>Sistema projetado de coordenadas</translation>
    </message>
    <message>
        <source>Resource Location Error</source>
        <translation>Erro no recurso de localização</translation>
    </message>
    <message>
        <source>Error reading database file from: 
 %1
Because of this the projection selector will not work...</source>
        <translation>Erro ao ler arquivo de base de dados de: 
 %1 
Devido ao seletor de projeção não estar em funcionamento...</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelectorBase</name>
    <message>
        <source>Search</source>
        <translation>Procurar</translation>
    </message>
    <message>
        <source>Find</source>
        <translation>Encontrar</translation>
    </message>
    <message>
        <source>EPSG ID</source>
        <translation>EPSG ID</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <source>Coordinate Reference System Selector</source>
        <translation>Seletor de Sistema de Referência de Coordenadas</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Sistema de Referência de Coordenadas</translation>
    </message>
    <message>
        <source>EPSG</source>
        <translation>EPSG</translation>
    </message>
    <message>
        <source>ID</source>
        <translation type="unfinished">ID</translation>
    </message>
</context>
<context>
    <name>QgsPythonDialog</name>
    <message>
        <source>Python console</source>
        <translation>Console phyton</translation>
    </message>
    <message>
        <source>&gt;&gt;&gt;</source>
        <translation>&gt;&gt;&gt;</translation>
    </message>
    <message>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation>Ao acessar o ambiente Quantum deste console python use objetos de escopo global com uma instância de classe d e QgisInterface.&lt;br&gt;Usage e.g.: iface.zoomFull()</translation>
    </message>
</context>
<context>
    <name>QgsQuickPrint</name>
    <message>
        <source> km</source>
        <translation type="unfinished"> km</translation>
    </message>
    <message>
        <source> mm</source>
        <translation type="unfinished"> mm</translation>
    </message>
    <message>
        <source> cm</source>
        <translation type="unfinished"> cm</translation>
    </message>
    <message>
        <source> m</source>
        <translation type="unfinished"> m</translation>
    </message>
    <message>
        <source> miles</source>
        <translation> milhas</translation>
    </message>
    <message>
        <source> mile</source>
        <translation type="unfinished"> milha</translation>
    </message>
    <message>
        <source> inches</source>
        <translation> polegadas</translation>
    </message>
    <message>
        <source> foot</source>
        <translation> pé</translation>
    </message>
    <message>
        <source> feet</source>
        <translation> pés</translation>
    </message>
    <message>
        <source> degree</source>
        <translation type="unfinished"> graus</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation type="unfinished"> graus</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation type="unfinished"> desconhecido</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <source>Not Set</source>
        <translation>Não Ajustado</translation>
    </message>
    <message>
        <source>Driver:</source>
        <translation>Driver:</translation>
    </message>
    <message>
        <source>Dimensions:</source>
        <translation>Dimensões:</translation>
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
        <source>Origin:</source>
        <translation>Origem:</translation>
    </message>
    <message>
        <source>Pixel Size:</source>
        <translation>Tamanho do Pixel:</translation>
    </message>
    <message>
        <source>Raster Extent: </source>
        <translation type="obsolete">Extensão do Raster: </translation>
    </message>
    <message>
        <source>Clipped area: </source>
        <translation type="obsolete">Area clipada: </translation>
    </message>
    <message>
        <source>Pyramid overviews:</source>
        <translation>Visões da Pirâmide:</translation>
    </message>
    <message>
        <source>Band</source>
        <translation>Banda</translation>
    </message>
    <message>
        <source>Band No</source>
        <translation>No da Banda</translation>
    </message>
    <message>
        <source>No Stats</source>
        <translation>Sem Informações</translation>
    </message>
    <message>
        <source>No stats collected yet</source>
        <translation>Nenhuma informação coletada até o momento</translation>
    </message>
    <message>
        <source>Min Val</source>
        <translation>Val Min</translation>
    </message>
    <message>
        <source>Max Val</source>
        <translation>Val Máx</translation>
    </message>
    <message>
        <source>Range</source>
        <translation>Intervalo</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Média</translation>
    </message>
    <message>
        <source>Sum of squares</source>
        <translation>Soma das raízes</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Desvio Padrão</translation>
    </message>
    <message>
        <source>Sum of all cells</source>
        <translation>Soma de todas as células</translation>
    </message>
    <message>
        <source>Cell Count</source>
        <translation>Número de Células</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Tipo de Dados:</translation>
    </message>
    <message>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte - Inteiro de 8 bits sem sinal</translation>
    </message>
    <message>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 - Inteiro de 16 bits sem sinal</translation>
    </message>
    <message>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 - Inteiro de 16 bits</translation>
    </message>
    <message>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 - Inteiro de 32 bits sem sinal</translation>
    </message>
    <message>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 - Inteiro de 32 bits</translation>
    </message>
    <message>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 - Ponto flutuante de 32 bits</translation>
    </message>
    <message>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 - Ponto flutuante de 64 bits</translation>
    </message>
    <message>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 - Inteiro complexo de 16 bits</translation>
    </message>
    <message>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 - Inteiro complexo de 32 bits</translation>
    </message>
    <message>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 - Ponto flutuante complexo de 32 bits</translation>
    </message>
    <message>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 - Ponto flutuante complexto de 64 bits</translation>
    </message>
    <message>
        <source>Could not determine raster data type.</source>
        <translation>Não foi possível determinar o tipo de dados do raster.</translation>
    </message>
    <message>
        <source>Average Magphase</source>
        <translation>Magphase Média</translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Média</translation>
    </message>
    <message>
        <source>Layer Spatial Reference System: </source>
        <translation>Sistema de Referência Espacial da Camada: </translation>
    </message>
    <message>
        <source>out of extent</source>
        <translation>fora da extensão</translation>
    </message>
    <message>
        <source>null (no data)</source>
        <translation>nulo (sem dado)</translation>
    </message>
    <message>
        <source>Dataset Description</source>
        <translation>Descrição do dataset</translation>
    </message>
    <message>
        <source>No Data Value</source>
        <translation>Sem valor de dados</translation>
    </message>
    <message>
        <source>and all other files</source>
        <translation>e todos outros arquivos</translation>
    </message>
    <message>
        <source>NoDataValue not set</source>
        <translation>SemValordeDados não configurado</translation>
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
        <translation>Escalas de cinza</translation>
    </message>
    <message>
        <source>Pseudocolor</source>
        <translation>Pseudocores</translation>
    </message>
    <message>
        <source>Freak Out</source>
        <translation>Barbarizar</translation>
    </message>
    <message>
        <source>Not Set</source>
        <translation>Não configurado</translation>
    </message>
    <message>
        <source>Columns: </source>
        <translation>Colunas: </translation>
    </message>
    <message>
        <source>Rows: </source>
        <translation>Linhas: </translation>
    </message>
    <message>
        <source>No-Data Value: </source>
        <translation>Sem valores de dados: </translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>n/a</translation>
    </message>
    <message>
        <source>Write access denied</source>
        <translation>Acesso a escrita negado</translation>
    </message>
    <message>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation>Acesso a escrita negado. Ajuste as permissões do arquivo e tente novamente.

</translation>
    </message>
    <message>
        <source>Building pyramids failed.</source>
        <translation>Falha ao construir pirâmides.</translation>
    </message>
    <message>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation>Construir &apos;overviews&apos; em pirâmide não é suportado neste tipo de raster.</translation>
    </message>
    <message>
        <source>No Stretch</source>
        <translation>Sem extensão</translation>
    </message>
    <message>
        <source>Stretch To MinMax</source>
        <translation>Extender para MinMax</translation>
    </message>
    <message>
        <source>Stretch And Clip To MinMax</source>
        <translation>Extender e cortar para MinMax</translation>
    </message>
    <message>
        <source>Clip To MinMax</source>
        <translation>Cortar para MinMax</translation>
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
        <translation>Quantis</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="unfinished">Descrição</translation>
    </message>
    <message>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation>Rasters de alta resolução podem tornar lenta a navegação no QGIS</translation>
    </message>
    <message>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation>Ao criar cópias de baixa resolução (pirâmide) poderá haver um ganho em performance, pois o QGIS selecionará a resolução de acordo com o nível de aproximação.</translation>
    </message>
    <message>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation>Você deve ter acesso de escrita no diretório onde está a imagem original para criar pirâmides.</translation>
    </message>
    <message>
        <source>Red</source>
        <translation>Vermelho</translation>
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
        <translation>Transparência (%)</translation>
    </message>
    <message>
        <source>Gray</source>
        <translation type="unfinished">Cinza</translation>
    </message>
    <message>
        <source>Indexed Value</source>
        <translation>Valor indexado</translation>
    </message>
    <message>
        <source>User Defined</source>
        <translation>Definido pelo usuário</translation>
    </message>
    <message>
        <source>No-Data Value: Not Set</source>
        <translation>Sem valores de dados: não configurado</translation>
    </message>
    <message>
        <source>Save file</source>
        <translation>Salva arquivo</translation>
    </message>
    <message>
        <source>Textfile (*.txt)</source>
        <translation>Arquivo de texto (*.txt)</translation>
    </message>
    <message>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation>Exportar arquivo com pixel transparente gerado pelo QGIS</translation>
    </message>
    <message>
        <source>Open file</source>
        <translation>Abrir arquivo</translation>
    </message>
    <message>
        <source>Import Error</source>
        <translation>Erro ao importar</translation>
    </message>
    <message>
        <source>The following lines contained errors

</source>
        <translation>As linhas seguintes contém erros</translation>
    </message>
    <message>
        <source>Read access denied</source>
        <translation>Acesso de leitura negado</translation>
    </message>
    <message>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation>Acesso de leitura negado. Ajuste as permissões de arquivo e tente novamente.</translation>
    </message>
    <message>
        <source>Color Ramp</source>
        <translation>Escala de cor</translation>
    </message>
    <message>
        <source>Default Style</source>
        <translation>Estilo padrão</translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Arquivo de camada do QGIS (*.qml)</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation>Estilo de formato desconhecido: </translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation>Mapa de cores</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation type="unfinished">Linear</translation>
    </message>
    <message>
        <source>Exact</source>
        <translation>Exato</translation>
    </message>
    <message>
        <source>Please note that building internal pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation>Note que ao criar pirâmide interna o arquivo original será alterado e, uma vez criada, não há como retroceder!</translation>
    </message>
    <message>
        <source>Please note that building internal pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation>Note que ao criar uma pirâmide interna sua imagem poderá ser corrompida - sempre faça uma cópia de reserva de seus dados antes!</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>Padrão</translation>
    </message>
    <message>
        <source>The file was not writeable. Some formats do not support pyramid overviews. Consult the GDAL documentation if in doubt.</source>
        <translation>O arquivo não pode ser escrito. Alguns formatos não suportam visões em pirâmide. consulte a documentação GDAL em caso de dúvida.</translation>
    </message>
    <message>
        <source>Custom color map entry</source>
        <translation>Entrada do mapa de cor personalizada</translation>
    </message>
    <message>
        <source>QGIS Generated Color Map Export File</source>
        <translation>Arquivo de cor de mapa do QGIS gerado</translation>
    </message>
    <message>
        <source>Load Color Map</source>
        <translation>Carregar mapa de cores</translation>
    </message>
    <message>
        <source>Saved Style</source>
        <translation>Estilo salvo</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <source>Raster Layer Properties</source>
        <translation>Propriedades da camada Raster</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Geral</translation>
    </message>
    <message>
        <source>No Data:</source>
        <translation>Nenhum Dado:</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Simbologia</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Cheio&lt;/p&gt;</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Nenhum</translation>
    </message>
    <message>
        <source>Metadata</source>
        <translation>Metadata</translation>
    </message>
    <message>
        <source>Pyramids</source>
        <translation>Pirâmides</translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Média</translation>
    </message>
    <message>
        <source>Nearest Neighbour</source>
        <translation>Vizinho mais Próximo</translation>
    </message>
    <message>
        <source>Thumbnail</source>
        <translation>Pré-Visualização</translation>
    </message>
    <message>
        <source>Columns:</source>
        <translation>Colunas:</translation>
    </message>
    <message>
        <source>Rows:</source>
        <translation>Linhas:</translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Escala máxima em que a esta camada será mostrada. </translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Escala mínima em que esta camada será mostrada. </translation>
    </message>
    <message>
        <source>Histogram</source>
        <translation>Histograma</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opções</translation>
    </message>
    <message>
        <source>Chart Type</source>
        <translation>Tipo de Carta</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Atualizar</translation>
    </message>
    <message>
        <source>Max</source>
        <translation type="unfinished">Máx</translation>
    </message>
    <message>
        <source>Min</source>
        <translation type="unfinished">Mín</translation>
    </message>
    <message>
        <source> 00%</source>
        <translation> 00%</translation>
    </message>
    <message>
        <source>Render as</source>
        <translation>Renderizar como</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation>Mapa de cores</translation>
    </message>
    <message>
        <source>Delete entry</source>
        <translation>Excluir entrada</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Classificar</translation>
    </message>
    <message>
        <source>1</source>
        <translation type="unfinished">1</translation>
    </message>
    <message>
        <source>2</source>
        <translation type="unfinished">2</translation>
    </message>
    <message>
        <source>Single band gray</source>
        <translation>Banda cinza simples</translation>
    </message>
    <message>
        <source>Three band color</source>
        <translation>Três bandas de cores</translation>
    </message>
    <message>
        <source>RGB mode band selection and scaling</source>
        <translation>Modo RGB de seleção de bandas e escala</translation>
    </message>
    <message>
        <source>Red band</source>
        <translation>Banda vermelha</translation>
    </message>
    <message>
        <source>Green band</source>
        <translation>Banda Verde</translation>
    </message>
    <message>
        <source>Blue band</source>
        <translation>Banda Azul</translation>
    </message>
    <message>
        <source>Custom min / max values</source>
        <translation>Personalizar valores min / max</translation>
    </message>
    <message>
        <source>Red min</source>
        <translation>Vermelho min</translation>
    </message>
    <message>
        <source>Red max</source>
        <translation>Vermelho max</translation>
    </message>
    <message>
        <source>Green min</source>
        <translation>Verde min</translation>
    </message>
    <message>
        <source>Green max</source>
        <translation>Verde max</translation>
    </message>
    <message>
        <source>Blue min</source>
        <translation>Azul min</translation>
    </message>
    <message>
        <source>Blue max</source>
        <translation>Azul max</translation>
    </message>
    <message>
        <source>Single band properties</source>
        <translation>Propriedades de bandas simples</translation>
    </message>
    <message>
        <source>Gray band</source>
        <translation>Banda cinza</translation>
    </message>
    <message>
        <source>Color map</source>
        <translation>Mapa de cor</translation>
    </message>
    <message>
        <source>Invert color map</source>
        <translation>Inverter mapa de cor</translation>
    </message>
    <message>
        <source>Use standard deviation</source>
        <translation>Use desvio padrão</translation>
    </message>
    <message>
        <source>Note:</source>
        <translation>Note:</translation>
    </message>
    <message>
        <source>Load min / max values from band</source>
        <translation>Carregar valores min / max da banda</translation>
    </message>
    <message>
        <source>Estimate (faster)</source>
        <translation>Estimado (rápido)</translation>
    </message>
    <message>
        <source>Actual (slower)</source>
        <translation>Real (mais lento)</translation>
    </message>
    <message>
        <source>Load</source>
        <translation>Carrega</translation>
    </message>
    <message>
        <source>Contrast enhancement</source>
        <translation>Melhorar contraste</translation>
    </message>
    <message>
        <source>Current</source>
        <translation>Atual</translation>
    </message>
    <message>
        <source>Save current contrast enhancement algorithm as default. This setting will be persistent between QGIS sessions.</source>
        <translation>Salvar a melhoria de contraste atual como padrão. Este ambiente será usado nas próximas sessões do QGIS</translation>
    </message>
    <message>
        <source>Saves current contrast enhancement algorithm as a default. This setting will be persistent between QGIS sessions.</source>
        <translation>Salva a melhoria de contraste atual como padrão. Este ambiente será usado nas próximas sessões do QGIS</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>Padrão</translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation type="unfinished">EtiquetaTexto</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Transparência</translation>
    </message>
    <message>
        <source>Global transparency</source>
        <translation>Transparência global</translation>
    </message>
    <message>
        <source>No data value</source>
        <translation>Sem valores de dados</translation>
    </message>
    <message>
        <source>Reset no data value</source>
        <translation>Reset valores sem dados</translation>
    </message>
    <message>
        <source>Custom transparency options</source>
        <translation>Opções de transparência personalizada</translation>
    </message>
    <message>
        <source>Transparency band</source>
        <translation>Banda de transparência</translation>
    </message>
    <message>
        <source>Transparent pixel list</source>
        <translation>LIsta de pixel transparente</translation>
    </message>
    <message>
        <source>Add values manually</source>
        <translation>Adiciona valores manualmente</translation>
    </message>
    <message>
        <source>Add Values from display</source>
        <translation>Adiciona valores da tela</translation>
    </message>
    <message>
        <source>Remove selected row</source>
        <translation>Remove linha selecionada</translation>
    </message>
    <message>
        <source>Default values</source>
        <translation>Valores padrão</translation>
    </message>
    <message>
        <source>Import from file</source>
        <translation>Importar do arquivo</translation>
    </message>
    <message>
        <source>Export to file</source>
        <translation>Exportar para o arquivo</translation>
    </message>
    <message>
        <source>Number of entries</source>
        <translation>Número de entradas</translation>
    </message>
    <message>
        <source>Color interpolation</source>
        <translation>Cor de interpolação</translation>
    </message>
    <message>
        <source>Classification mode</source>
        <translation>Modo de classificação</translation>
    </message>
    <message>
        <source>Scale dependent visibility</source>
        <translation>Escala dependente da visibilidade</translation>
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
        <translation>Camada fonte</translation>
    </message>
    <message>
        <source>Display name</source>
        <translation type="unfinished">Nome de Identificação</translation>
    </message>
    <message>
        <source>Pyramid resolutions</source>
        <translation>Resoluções de pirâmide</translation>
    </message>
    <message>
        <source>Resampling method</source>
        <translation>Médoto de reamostragem</translation>
    </message>
    <message>
        <source>Build pyramids</source>
        <translation>Construir pirâmides</translation>
    </message>
    <message>
        <source>Line graph</source>
        <translation>Gráfico de linha</translation>
    </message>
    <message>
        <source>Bar chart</source>
        <translation>Gráfico de barras</translation>
    </message>
    <message>
        <source>Column count</source>
        <translation>Contador de colunas</translation>
    </message>
    <message>
        <source>Out of range OK?</source>
        <translation>Fora do alcance OK?</translation>
    </message>
    <message>
        <source>Allow approximation</source>
        <translation>Permitir aproximação</translation>
    </message>
    <message>
        <source>Restore Default Style</source>
        <translation>Restaurar estilo padrão</translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation>Salvar como padrão</translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation>Carregar estilo...</translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation>Salvar estilo ...</translation>
    </message>
    <message>
        <source>Default R:1 G:2 B:3</source>
        <translation>Padrão R:1 G:2 B:3</translation>
    </message>
    <message>
        <source>Add entry</source>
        <translation>Adiciona entrada</translation>
    </message>
    <message>
        <source>Sort</source>
        <translation>Classifica</translation>
    </message>
    <message>
        <source>Load color map from band</source>
        <translation>Carregar mapa de cor da banda</translation>
    </message>
    <message>
        <source>Load color map from file</source>
        <translation>Carregar mapa de cor do arquivo</translation>
    </message>
    <message>
        <source>Export color map to file</source>
        <translation>Exportar mapa de cores para arquivo</translation>
    </message>
    <message>
        <source>Generate new color map</source>
        <translation>Gerar novo mapa de cores</translation>
    </message>
    <message>
        <source>Coordinate reference system</source>
        <translation>Sistema de referência de coordenadas</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation type="unfinished">Mudar ...</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation type="unfinished">Legenda</translation>
    </message>
    <message>
        <source>Palette</source>
        <translation type="unfinished">Paleta</translation>
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
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Build pyramids internally if possible</source>
        <translation>Construir pirâmides internamnente se possível</translation>
    </message>
</context>
<context>
    <name>QgsRunProcess</name>
    <message>
        <source>Unable to run command</source>
        <translation>Impossível executar comando</translation>
    </message>
    <message>
        <source>Starting</source>
        <translation>Iniciando</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Feito</translation>
    </message>
    <message>
        <source>Action</source>
        <translation type="unfinished">Ação</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPlugin</name>
    <message>
        <source> metres/km</source>
        <translation> metros/km</translation>
    </message>
    <message>
        <source> feet</source>
        <translation>pés</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation> graus</translation>
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
        <translation> pés</translation>
    </message>
    <message>
        <source> degree</source>
        <translation> graus</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation> desconhecido</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Superior Esquerdo</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Inferior Esquerdo</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Superior Direito</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Inferior Direito</translation>
    </message>
    <message>
        <source>Tick Down</source>
        <translation>Marca (tick) acima</translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation>Marca (tick) abaixo</translation>
    </message>
    <message>
        <source>Bar</source>
        <translation>Barra</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Caixa</translation>
    </message>
    <message>
        <source>&amp;Scale Bar</source>
        <translation>&amp;Barra de escala</translation>
    </message>
    <message>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation>Cria uma barra de escala que é exibida na tela</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Decorações</translation>
    </message>
    <message>
        <source> feet/miles</source>
        <translation> pés/milhas</translation>
    </message>
    <message>
        <source> miles</source>
        <translation> milhas</translation>
    </message>
    <message>
        <source> mile</source>
        <translation type="unfinished"> milha</translation>
    </message>
    <message>
        <source> inches</source>
        <translation> polegadas</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <source>Scale Bar Plugin</source>
        <translation>Plugin de Barra de Escala</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Superior Esquerdo</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Superior Direito</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Inferior Esquerdo</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Inferior Direito</translation>
    </message>
    <message>
        <source>Size of bar:</source>
        <translation>Tamanho da barra:</translation>
    </message>
    <message>
        <source>Placement:</source>
        <translation>Posicionamento:</translation>
    </message>
    <message>
        <source>Tick Down</source>
        <translation>Marca (tick) abaixo</translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation>Marca (tick) acima</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Caixa</translation>
    </message>
    <message>
        <source>Bar</source>
        <translation>Barra</translation>
    </message>
    <message>
        <source>Select the style of the scale bar</source>
        <translation>Selecione o estilo da barra de escala</translation>
    </message>
    <message>
        <source>Colour of bar:</source>
        <translation>Cor da barra:</translation>
    </message>
    <message>
        <source>Scale bar style:</source>
        <translation>Estilo da barra de escala:</translation>
    </message>
    <message>
        <source>Enable scale bar</source>
        <translation>Habilitar barra de escala</translation>
    </message>
    <message>
        <source>Automatically snap to round number on resize</source>
        <translation>Arredondar números automaticamente ao redimensionar</translation>
    </message>
    <message>
        <source>Click to select the colour</source>
        <translation>Clique para selecionar a cor</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Este plugin desenha uma barra de escala no mapa. Note que a opção de tamanho é a preferida e pode ser modificada pelo QGIS dependendo do nível de visualização. O tamanho é medido de acordo com as unidades do mapa especificadas nas propriedades do projeto.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message>
        <source>No matching features found.</source>
        <translation>Nenhuma correspondência de feições encontrada.</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation>Procurar resultados</translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation>Erro de análise na procura de string</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation>Nenhum registro encontrado</translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned.</source>
        <translation>A consulta que você especificou retornou nenhum registro.</translation>
    </message>
    <message>
        <source>Search query builder</source>
        <translation>Procurar construtor de questões</translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Tem certeza que deseja remover o </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> conexão e todos os ajustes associados?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Confirme a exclusão</translation>
    </message>
    <message>
        <source>WMS Provider</source>
        <translation>Provedor WMS</translation>
    </message>
    <message>
        <source>Could not open the WMS Provider</source>
        <translation>Impossível abrir o provedor WMS</translation>
    </message>
    <message>
        <source>Select Layer</source>
        <translation>Selecionar camada</translation>
    </message>
    <message>
        <source>You must select at least one layer first.</source>
        <translation>Você deve selecionar pelo menos uma camada primeiro.</translation>
    </message>
    <message>
        <source>Could not understand the response.  The</source>
        <translation>Impossível entender a resposta. O</translation>
    </message>
    <message>
        <source>provider said</source>
        <translation>provedor disse </translation>
    </message>
    <message>
        <source>WMS proxies</source>
        <translation>proxys WMS</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Sistema de referência de coordenadas</translation>
    </message>
    <message>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation>Não existe um sistema de referências de coordenadas habilitadopara definir as camadas que você selecionou.</translation>
    </message>
    <message>
        <source>Several WMS servers have been added to the server list. Note that if you access the internet via a web proxy, you will need to set the proxy settings in the QGIS options dialog.</source>
        <translation>Muitos servidores WMS foram adicionados a lista de servidores. Note que se você acessa a internet via proxy, será necessário acertar as configurações proxy no diálogo de opções do QGIS</translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelectBase</name>
    <message>
        <source>Add Layer(s) from a Server</source>
        <translation>Adiciona camadas de um servidor</translation>
    </message>
    <message>
        <source>C&amp;lose</source>
        <translation>F&amp;echar</translation>
    </message>
    <message>
        <source>Alt+L</source>
        <translation>Alt+L</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Ajuda</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Image encoding</source>
        <translation>Codificando a imagem</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation>Camadas</translation>
    </message>
    <message>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Resumo</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Adiciona</translation>
    </message>
    <message>
        <source>Alt+A</source>
        <translation>Alt+A</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation>Conexões com servidor</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation>&amp;Novo</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Excluir</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>C&amp;onectar</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation>Pronto</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Sistema de referência de coordenadas</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Mudar ...</translation>
    </message>
    <message>
        <source>Adds a few example WMS servers</source>
        <translation>Adiciona algum exemplo de servidor WMS</translation>
    </message>
    <message>
        <source>Add default servers</source>
        <translation>Adiciona servidores padrões</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <source>The database gave an error while executing this SQL:</source>
        <translation>A base de dados apresentou erro durante a execução desta SQL:</translation>
    </message>
    <message>
        <source>The error was:</source>
        <translation>O erro foi:</translation>
    </message>
    <message>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation>... (rest of SQL trimmed)</translation>
    </message>
    <message>
        <source>Scanning </source>
        <translation>Rastrear </translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <source>Solid Line</source>
        <translation>Linha sólida</translation>
    </message>
    <message>
        <source>Dash Line</source>
        <translation>Linha tracejada</translation>
    </message>
    <message>
        <source>Dot Line</source>
        <translation>Llinha de pontos</translation>
    </message>
    <message>
        <source>Dash Dot Line</source>
        <translation>Linha como traços e pontos</translation>
    </message>
    <message>
        <source>Dash Dot Dot Line</source>
        <translation>Linha com traço, ponto e ponto</translation>
    </message>
    <message>
        <source>No Pen</source>
        <translation>Sem caneta</translation>
    </message>
    <message>
        <source>No Brush</source>
        <translation>Sem pincel</translation>
    </message>
    <message>
        <source>Solid</source>
        <translation>Sólido</translation>
    </message>
    <message>
        <source>Horizontal</source>
        <translation type="unfinished">Horizontal</translation>
    </message>
    <message>
        <source>Vertical</source>
        <translation type="unfinished">Vertical</translation>
    </message>
    <message>
        <source>Cross</source>
        <translation>Transpor</translation>
    </message>
    <message>
        <source>BDiagonal</source>
        <translation>BDiagonal</translation>
    </message>
    <message>
        <source>FDiagonal</source>
        <translation>FDiagonal</translation>
    </message>
    <message>
        <source>Diagonal X</source>
        <translation>Diagonal X</translation>
    </message>
    <message>
        <source>Dense1</source>
        <translation>Denso1</translation>
    </message>
    <message>
        <source>Dense2</source>
        <translation>Denso2</translation>
    </message>
    <message>
        <source>Dense3</source>
        <translation>Denso3</translation>
    </message>
    <message>
        <source>Dense4</source>
        <translation>Denso4</translation>
    </message>
    <message>
        <source>Dense5</source>
        <translation>Denso5</translation>
    </message>
    <message>
        <source>Dense6</source>
        <translation>Denso6</translation>
    </message>
    <message>
        <source>Dense7</source>
        <translation>Denso7</translation>
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
        <translation>Símbolo simples</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Tamanho</translation>
    </message>
    <message>
        <source>Point Symbol</source>
        <translation>Símbolo de ponto</translation>
    </message>
    <message>
        <source>Area scale field</source>
        <translation>Campo de escala da área</translation>
    </message>
    <message>
        <source>Rotation field</source>
        <translation>Campo de rotação</translation>
    </message>
    <message>
        <source>Style Options</source>
        <translation>Opções e estilo</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Outline style</source>
        <translation>Estilo de contorno</translation>
    </message>
    <message>
        <source>Outline color</source>
        <translation>Cor do contorno</translation>
    </message>
    <message>
        <source>Outline width</source>
        <translation>Espessura do contorno</translation>
    </message>
    <message>
        <source>Fill color</source>
        <translation>Cor de preenchimento</translation>
    </message>
    <message>
        <source>Fill style</source>
        <translation>Estilo de preenchimento</translation>
    </message>
    <message>
        <source>Label</source>
        <translation type="unfinished">Rótulo</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <source>to vertex</source>
        <translation>ao vértice</translation>
    </message>
    <message>
        <source>to segment</source>
        <translation>ao segmento</translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation>ao vértice e segmento</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <source>Snapping options</source>
        <translation>Opções de aproximação</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="unfinished">Modo</translation>
    </message>
    <message>
        <source>Tolerance</source>
        <translation>Tolerância</translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <source>Are you sure you want to remove the [</source>
        <translation>Você tem certeza que quer remover a [</translation>
    </message>
    <message>
        <source>] connection and all associated settings?</source>
        <translation>] conexão e todos os ajustes associados?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Confirme a exclusão</translation>
    </message>
    <message>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>O seguinte(s) Shapefile(s) não foi (foram) carregado(s):</translation>
    </message>
    <message>
        <source>REASON: File cannot be opened</source>
        <translation>MOTIVO: O arquivo não pode ser aberto</translation>
    </message>
    <message>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>RAZÃO: Um ou ambos arquivos do Shapefile (*.dbf, *.shx) não foram encontrados</translation>
    </message>
    <message>
        <source>General Interface Help:</source>
        <translation>Interface Geral de Ajuda:</translation>
    </message>
    <message>
        <source>PostgreSQL Connections:</source>
        <translation>Conexões PostgreSQL:</translation>
    </message>
    <message>
        <source>[New ...] - create a new connection</source>
        <translation>[Novo...] - criar uma nova conexão</translation>
    </message>
    <message>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation>[Editar] - editar a conexão atualmente selecionada</translation>
    </message>
    <message>
        <source>[Remove] - remove the currently selected connection</source>
        <translation>[Remover] - Remove a conexão atualmente selecionada</translation>
    </message>
    <message>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation>-você precisa selecionar a conexão que funciona (conecta corretamente) para conseguir importar arquivos</translation>
    </message>
    <message>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation>-quando mudar conexões o \&quot;Global Schema\&quot; também mudará de acordo</translation>
    </message>
    <message>
        <source>Shapefile List:</source>
        <translation>Lista de Shapefiles:</translation>
    </message>
    <message>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation>[Adicionar...] - abrir uma caixa de diálogo e selecionar o(s) arquivo(s) para importar</translation>
    </message>
    <message>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation>[Remover] - remove os arquivos atualmente selecionados da lista</translation>
    </message>
    <message>
        <source>[Remove All] - remove all the files in the list</source>
        <translation>[Remove Todos] - remove todos os arquivos da lista</translation>
    </message>
    <message>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] - Referência ID para os shapefiles à serem importados</translation>
    </message>
    <message>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation>[Usar Padrão (SRID)] - setar SRID para -1</translation>
    </message>
    <message>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation>[Nome da Coluna Geometria] - nome da coluna geometria no banco de dados</translation>
    </message>
    <message>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation>[Usar Padrão (Nome da Coluna Geometria)] - seta o nome para &apos;the_geom&apos;</translation>
    </message>
    <message>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation>[Glogal Schema] - seta o esquema para todos os arquivos à serem importados em</translation>
    </message>
    <message>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[Importar] - Importa os shapefiles atualmente na lista</translation>
    </message>
    <message>
        <source>[Quit] - quit the program
</source>
        <translation>[Sair] - sai do programa</translation>
    </message>
    <message>
        <source>[Help] - display this help dialog</source>
        <translation>[Ajuda] - mostra essa caixa de ajuda</translation>
    </message>
    <message>
        <source>Import Shapefiles</source>
        <translation>Omportar Shapefiles</translation>
    </message>
    <message>
        <source>You need to specify a Connection first</source>
        <translation>Você precisa especificar uma Conexão primeiro</translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again</source>
        <translation>A conexão falhou - Cheque a sua configuração e tente novamente</translation>
    </message>
    <message>
        <source>You need to add shapefiles to the list first</source>
        <translation>Você precisa adicionar shapefiles para a lista primeiro</translation>
    </message>
    <message>
        <source>Importing files</source>
        <translation>Importando arquivos</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>Progress</source>
        <translation>Progresso</translation>
    </message>
    <message>
        <source>Problem inserting features from file:</source>
        <translation>Problema inserindo feições do arquivo:</translation>
    </message>
    <message>
        <source>Invalid table name.</source>
        <translation>Nome de tabela inválido.</translation>
    </message>
    <message>
        <source>No fields detected.</source>
        <translation>Nenhum campo detectado.</translation>
    </message>
    <message>
        <source>The following fields are duplicates:</source>
        <translation>Os seguintes campos estão duplicados:</translation>
    </message>
    <message>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>Importar Shapefiles - Existe Relação</translation>
    </message>
    <message>
        <source>The Shapefile:</source>
        <translation>O Shapefile:</translation>
    </message>
    <message>
        <source>will use [</source>
        <translation>irá usar [</translation>
    </message>
    <message>
        <source>] relation for its data,</source>
        <translation>] relação com estes dados,</translation>
    </message>
    <message>
        <source>which already exists and possibly contains data.</source>
        <translation>e que já existem e possivelmente contém dados.</translation>
    </message>
    <message>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>Para evitar a perda de dados mude o &quot;DB Relation Name&quot;</translation>
    </message>
    <message>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation>para este Shapefile na lista da caixa de diálogo principal.</translation>
    </message>
    <message>
        <source>Do you want to overwrite the [</source>
        <translation>Você quer salvar sobre a relação do arquivo [</translation>
    </message>
    <message>
        <source>] relation?</source>
        <translation>] ?</translation>
    </message>
    <message>
        <source>File Name</source>
        <translation>Nome do Arquivo</translation>
    </message>
    <message>
        <source>Feature Class</source>
        <translation>Classe da Feição</translation>
    </message>
    <message>
        <source>Features</source>
        <translation>Feições</translation>
    </message>
    <message>
        <source>DB Relation Name</source>
        <translation>Nome Relacional do DB</translation>
    </message>
    <message>
        <source>Schema</source>
        <translation>Esquema</translation>
    </message>
    <message>
        <source>Add Shapefiles</source>
        <translation>Adiciona arquivos SHP</translation>
    </message>
    <message>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation>arquivos do tipo shape (*.shp);;Todos arquivos (*.*)</translation>
    </message>
    <message>
        <source>PostGIS not available</source>
        <translation>PostGIS não disponível</translation>
    </message>
    <message>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;A base de dados escolhida não tem PostGIS instalado,mas isto é necessário para armazenar os dados.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Erro enquanto executava a SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation>&lt;/p&gt;&lt;p&gt;A base de dados disse:</translation>
    </message>
    <message>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation>%1 de %2 shapefiles não puderam ser importados.</translation>
    </message>
    <message>
        <source>Password for </source>
        <translation type="unfinished">Senha para</translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation type="unfinished">Por favor, entre com sua senha:</translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT - Ferramenta de importação Shapefile para PostGIS</translation>
    </message>
    <message>
        <source>PostgreSQL Connections</source>
        <translation>Conexões PostgreSQL </translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Remover</translation>
    </message>
    <message>
        <source>Remove All</source>
        <translation>Remover Tudo</translation>
    </message>
    <message>
        <source>Global Schema</source>
        <translation>Esquema global</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Adicionar</translation>
    </message>
    <message>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Adicionar um shapefile para a lista de arquivos a serem importados</translation>
    </message>
    <message>
        <source>Remove the selected shapefile from the import list</source>
        <translation>Remover o shapefile selecionado da lista de importação</translation>
    </message>
    <message>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Remover todos os shapefiles da lista de importação</translation>
    </message>
    <message>
        <source>Set the SRID to the default value</source>
        <translation>Definir o SRID para valor padrão</translation>
    </message>
    <message>
        <source>Set the geometry column name to the default value</source>
        <translation>Definir o nome da coluna de geometria como valor padrão</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Novo</translation>
    </message>
    <message>
        <source>Create a new PostGIS connection</source>
        <translation>Criar uma nova conexão PostGIS</translation>
    </message>
    <message>
        <source>Remove the current PostGIS connection</source>
        <translation>Remover a atual conexão PostGIS</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="unfinished">Conectar</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <source>Edit the current PostGIS connection</source>
        <translation>Editar a conexão PostGIS atual</translation>
    </message>
    <message>
        <source>Import options and shapefile list</source>
        <translation>Importar opções e lista de shapefile</translation>
    </message>
    <message>
        <source>Use Default SRID or specify here</source>
        <translation>Use o SRID padrão ou especifique aqui</translation>
    </message>
    <message>
        <source>Use Default Geometry Column Name or specify here</source>
        <translation>Use o nome padrão de geometria de coluna ou especifique aqui</translation>
    </message>
    <message>
        <source>Primary Key Column Name</source>
        <translation>Nome da coluna Chave Primária</translation>
    </message>
    <message>
        <source>Connect to PostGIS</source>
        <translation>Conectar ao PostGIS</translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>&amp;Importa arquivos SHP para PostgreSQL</translation>
    </message>
    <message>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>Importa arquivos SHP para uma base de dados PostGis-enabled PostgreSQL. O esquema e os nomes de campo podem ser personalizados na importação</translation>
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
        <translation>Interpolação linear</translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialogBase</name>
    <message>
        <source>Triangle based interpolation</source>
        <translation>Interpolação baseada na triangulação</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This interpolator provides different methods for interpolation in a triangular irregular network (TIN).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Este interpolador fornece diferentes métodos de interpolação em uma rede triangular irregular (TIN).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Interpolation method:</source>
        <translation>Método de interpolação:</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialog</name>
    <message>
        <source>Confirm Delete</source>
        <translation type="unfinished">Confirme a exclusão</translation>
    </message>
    <message>
        <source>The classification field was changed from &apos;%1&apos; to &apos;%2&apos;.
Should the existing classes be deleted before classification?</source>
        <translation>O campo de classificação foi mudado de &apos;%1&apos; para &apos;%2&apos;. 
Existem classes que poderiam ser excluídas antes da classificação?</translation>
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
        <translation>Classifica</translation>
    </message>
    <message>
        <source>Classification field</source>
        <translation>Campo de classificação</translation>
    </message>
    <message>
        <source>Add class</source>
        <translation>Adiciona classe</translation>
    </message>
    <message>
        <source>Delete classes</source>
        <translation>Exclui Classe</translation>
    </message>
    <message>
        <source>Randomize Colors</source>
        <translation>Mistura cores</translation>
    </message>
    <message>
        <source>Reset Colors</source>
        <translation>Reset cores</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <source>ERROR: no provider</source>
        <translation>ERRO: sem provedor</translation>
    </message>
    <message>
        <source>ERROR: layer not editable</source>
        <translation>ERRO: camada não editável</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attributes added.</source>
        <translation>SUCESSO: %1 atributos adicionados</translation>
    </message>
    <message>
        <source>ERROR: %1 new attributes not added</source>
        <translation>ERRO: %1 novos atributos não adicionados</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attributes deleted.</source>
        <translation>SUCESSO: %1 atributos excluídos</translation>
    </message>
    <message>
        <source>ERROR: %1 attributes not deleted.</source>
        <translation>ERRO: %1 atributos não excluídos</translation>
    </message>
    <message>
        <source>SUCCESS: attribute %1 was added.</source>
        <translation>SUCESSO: atributo %1 foi adicionado.</translation>
    </message>
    <message>
        <source>ERROR: attribute %1 not added</source>
        <translation>ERRO: atributo %1 não adicionado</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attribute values changed.</source>
        <translation>SUCESSO: %1 valores de atributo modificado.</translation>
    </message>
    <message>
        <source>ERROR: %1 attribute value changes not applied.</source>
        <translation>ERRO: %1 mudanças de atributo não aplicada.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 features added.</source>
        <translation>SUCESSO: %1 feições adicionadas.</translation>
    </message>
    <message>
        <source>ERROR: %1 features not added.</source>
        <translation>ERRO: %1 feições não adicionadas.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 geometries were changed.</source>
        <translation>SUCESSO: %1 geometrias foram modificadas.</translation>
    </message>
    <message>
        <source>ERROR: %1 geometries not changed.</source>
        <translation>ERRO: %1 geometrias não foram modificadas.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 features deleted.</source>
        <translation>SUCESSO: %1 feições excluídas.</translation>
    </message>
    <message>
        <source>ERROR: %1 features not deleted.</source>
        <translation>ERRO: %1 feições não excluídas</translation>
    </message>
    <message>
        <source>No renderer object</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Classification field not found</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <source>Transparency: </source>
        <translation>Transparência: </translation>
    </message>
    <message>
        <source>Single Symbol</source>
        <translation>Símbolo simples</translation>
    </message>
    <message>
        <source>Graduated Symbol</source>
        <translation>Símbolo Graduado</translation>
    </message>
    <message>
        <source>Continuous Color</source>
        <translation>Cor Contínua</translation>
    </message>
    <message>
        <source>Unique Value</source>
        <translation>Valor Único</translation>
    </message>
    <message>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation>Este botão abre a ferramenta de consulta do PostgreSQL e permite que você filtre os seus dados e crie um novo conjunto para visualizá-los no mapa, evitando assim a visualização de todas as informações</translation>
    </message>
    <message>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>A consulta usada para limitar as feições é mostrada aqui. Isto é atualmente suportado apenas nas camadas do PostgreSQL. Pressione criar ou modifique a consulta clique no botão Ferramenta de Consulta</translation>
    </message>
    <message>
        <source>Spatial Index</source>
        <translation>Índice espacial</translation>
    </message>
    <message>
        <source>Creation of spatial index failed</source>
        <translation>A criação do Índice Espacial falhou</translation>
    </message>
    <message>
        <source>General:</source>
        <translation>Geral:</translation>
    </message>
    <message>
        <source>Storage type of this layer : </source>
        <translation>Tipode armazenamento desta camada : </translation>
    </message>
    <message>
        <source>Source for this layer : </source>
        <translation>Fonte ṕara esta camada: </translation>
    </message>
    <message>
        <source>Geometry type of the features in this layer : </source>
        <translation>Tipo de geometria das feições nesta camada:</translation>
    </message>
    <message>
        <source>The number of features in this layer : </source>
        <translation>Número de feições nesta camada: </translation>
    </message>
    <message>
        <source>Editing capabilities of this layer : </source>
        <translation>Editar capacidades para esta camada: </translation>
    </message>
    <message>
        <source>Extents:</source>
        <translation>Extensão:</translation>
    </message>
    <message>
        <source>In layer spatial reference system units : </source>
        <translation>Sistema de unidades de referência espacial na camada : </translation>
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
        <translation>Sistema de unidades de referência espacial no projeto : </translation>
    </message>
    <message>
        <source>Layer Spatial Reference System:</source>
        <translation>Sistema de referência espacial da camada:</translation>
    </message>
    <message>
        <source>Attribute field info:</source>
        <translation>Informação de atributo no campo:</translation>
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
        <translation>Comprimento</translation>
    </message>
    <message>
        <source>Precision</source>
        <translation>Precisão</translation>
    </message>
    <message>
        <source>Layer comment: </source>
        <translation>Comentário da camada: </translation>
    </message>
    <message>
        <source>Comment</source>
        <translation>Comentário</translation>
    </message>
    <message>
        <source>Default Style</source>
        <translation>Estilo padrão</translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Arquivo de Estilo de Camada do QGIS (*.qml)</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation>Formato de estilo desconhecido: </translation>
    </message>
    <message>
        <source>id</source>
        <translation>id</translation>
    </message>
    <message>
        <source>name</source>
        <translation>nome</translation>
    </message>
    <message>
        <source>type</source>
        <translation>tipo</translation>
    </message>
    <message>
        <source>length</source>
        <translation>comprimento</translation>
    </message>
    <message>
        <source>precision</source>
        <translation>precisão</translation>
    </message>
    <message>
        <source>comment</source>
        <translation>comentário</translation>
    </message>
    <message>
        <source>edit widget</source>
        <translation>editar widget</translation>
    </message>
    <message>
        <source>values</source>
        <translation>valores</translation>
    </message>
    <message>
        <source>line edit</source>
        <translation>editar linha</translation>
    </message>
    <message>
        <source>unique values</source>
        <translation>valores únicos</translation>
    </message>
    <message>
        <source>unique values (editable)</source>
        <translation>valores únicos (editável)</translation>
    </message>
    <message>
        <source>value map</source>
        <translation>valores do mapa</translation>
    </message>
    <message>
        <source>classification</source>
        <translation>classificação</translation>
    </message>
    <message>
        <source>range (editable)</source>
        <translation>alcance (editável)</translation>
    </message>
    <message>
        <source>range (slider)</source>
        <translation>alcance (slider)</translation>
    </message>
    <message>
        <source>file name</source>
        <translation>nome do arquivo</translation>
    </message>
    <message>
        <source>Name conflict</source>
        <translation type="unfinished">Conflito de nomes</translation>
    </message>
    <message>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation>O atributo não pode ser inserido. O nome já existe da tabela.</translation>
    </message>
    <message>
        <source>Creation of spatial index successful</source>
        <translation>Sucesso na criação do índice espacial</translation>
    </message>
    <message>
        <source>Saved Style</source>
        <translation>Estilo Salvo</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <source>Layer Properties</source>
        <translation>Propriedades da Camada</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Simbologia</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Geral</translation>
    </message>
    <message>
        <source>Use scale dependent rendering</source>
        <translation>Use escala dependente a renderização</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Escala mínima em que essa camada pode ser exibida. </translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Escala máxima em que essa camada pode ser exibida. </translation>
    </message>
    <message>
        <source>Display name</source>
        <translation>Nome de Identificação</translation>
    </message>
    <message>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation>Use este controle para selecionar qual campo é colocado no nível mais alto na caixa de diálogo Identifique Resultados</translation>
    </message>
    <message>
        <source>Display field for the Identify Results dialog box</source>
        <translation>Mostra campo na caixa de diálogo Identifique Resultados</translation>
    </message>
    <message>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation>Isso seleciona o campo a ser mostrado na caixa de diálogo Identifique Resultados</translation>
    </message>
    <message>
        <source>Display field</source>
        <translation>Exibir campo</translation>
    </message>
    <message>
        <source>Subset</source>
        <translation>Subset</translation>
    </message>
    <message>
        <source>Query Builder</source>
        <translation>Ferramenta de Consulta</translation>
    </message>
    <message>
        <source>Create Spatial Index</source>
        <translation>Criar índice espacial</translation>
    </message>
    <message>
        <source>Metadata</source>
        <translation>Metadados</translation>
    </message>
    <message>
        <source>Labels</source>
        <translation>Rótulos</translation>
    </message>
    <message>
        <source>Display labels</source>
        <translation>Mostrar rótulos</translation>
    </message>
    <message>
        <source>Actions</source>
        <translation>Ações</translation>
    </message>
    <message>
        <source>Restore Default Style</source>
        <translation>Restaura Estilo Padrão</translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation>Salvar como Padrão</translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation>Carregar Estilo ...</translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation>Salva Estilo ...</translation>
    </message>
    <message>
        <source>Legend type</source>
        <translation>Tipo de legenda</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Transparência</translation>
    </message>
    <message>
        <source>Options</source>
        <translation type="unfinished">Opções</translation>
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
        <translation>Mudar SRC</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation type="unfinished">Atributos</translation>
    </message>
    <message>
        <source>New column</source>
        <translation type="unfinished">Nova coluna</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <translation type="unfinished">Ctrl+N</translation>
    </message>
    <message>
        <source>Delete column</source>
        <translation type="unfinished">Excluir coluna</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation type="unfinished">Ctrl+X</translation>
    </message>
    <message>
        <source>Toggle editing mode</source>
        <translation>Ativar modo de edição</translation>
    </message>
    <message>
        <source>Click to toggle table editing</source>
        <translation>Clique para ativar edição de tabela</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <source>&amp;Add WFS layer</source>
        <translation>&amp;Adiciona camada WFS</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <source>unknown</source>
        <translation>deconhecido</translation>
    </message>
    <message>
        <source>received %1 bytes from %2</source>
        <translation>recebidos %1 bytes de %2</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Tem certeza que deseja remover o </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> conexão e todos os ajustes associados?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Confirme a exclusão</translation>
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
        <translation>Nome</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Resumo</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Sistema de referência de coordenadas</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Mudar ...</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation>Conexões do servidor</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation>&amp;Novo</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Excluir</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>C&amp;onectar</translation>
    </message>
    <message>
        <source>Add WFS Layer from a Server</source>
        <translation>Adiciona camada WFS de um Servidor</translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <source>Tried URL: </source>
        <translation>URL tentada: </translation>
    </message>
    <message>
        <source>HTTP Exception</source>
        <translation>Exceção HTTP</translation>
    </message>
    <message>
        <source>WMS Service Exception</source>
        <translation>Exceção de serviço WMS</translation>
    </message>
    <message>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>Impossível obter capacidades WMS:  %1 na linha %2 coluna %3</translation>
    </message>
    <message>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>Isto é provavelmente devido a uma URL incorreta do servidor WMS.</translation>
    </message>
    <message>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>Impossível obter capacidades WMS neste formato (DTD): no %1 ou %2 encontrado</translation>
    </message>
    <message>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>Impossível obter oo serviço WMS. Exceção em %1: %2 na linha %3 coluna %4</translation>
    </message>
    <message>
        <source>Request contains a Format not offered by the server.</source>
        <translation>Requisição contém um formato não oferecido pelo servidor</translation>
    </message>
    <message>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Requisição contém um CRS não oferecido pelo servidor para uma ou mais camadas.</translation>
    </message>
    <message>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Requisição contém um SRS não oferecido pelo servidor para uma ou mais camadas.</translation>
    </message>
    <message>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation>A requisição GetMap é para uma camada não oferecida pelo servidor, ou a requisição GetFeatureInfo é para uma camada não mostrada no mapa.</translation>
    </message>
    <message>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>Requisição é para uma camada em um estilo não oferecido pelo servidor.</translation>
    </message>
    <message>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>GetFeatureInfo pedido é aplicado a uma camada declarada como não pesquisável.</translation>
    </message>
    <message>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>GetFeatureInfo pedido contém um valor inválido de X e Y </translation>
    </message>
    <message>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation>O valor do parâmetro (opcional) em GetCapabilities é igual ao atual valor de serviço.</translation>
    </message>
    <message>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation>O valor do parâmetro (opcional) em GetCapabilities deve ser maior do que o atual valor de serviço.</translation>
    </message>
    <message>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation>Requisição não inclui um valor de dimensão de amostra, o servidor não pode atribuir um valor padrão para esta dimensão.</translation>
    </message>
    <message>
        <source>Request contains an invalid sample dimension value.</source>
        <translation>Requisição contém um valor inválido de dimensão de amostra. </translation>
    </message>
    <message>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>Requisição é para uma operação opcional que não é suportada pelo servidor.</translation>
    </message>
    <message>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(Código de erro desconhecido pela postagem - 1.3 WMS server)</translation>
    </message>
    <message>
        <source>The WMS vendor also reported: </source>
        <translation>O vendedor WMS também reportou: </translation>
    </message>
    <message>
        <source>Server Properties:</source>
        <translation>Propriedades do servidor:</translation>
    </message>
    <message>
        <source>Property</source>
        <translation>Propriedade</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Valor</translation>
    </message>
    <message>
        <source>WMS Version</source>
        <translation>Versão WMS</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Resumo</translation>
    </message>
    <message>
        <source>Keywords</source>
        <translation>Palavras-chave</translation>
    </message>
    <message>
        <source>Online Resource</source>
        <translation>Recurso on-line</translation>
    </message>
    <message>
        <source>Contact Person</source>
        <translation>Contato pessoal</translation>
    </message>
    <message>
        <source>Fees</source>
        <translation>Taxa</translation>
    </message>
    <message>
        <source>Access Constraints</source>
        <translation>Acesso reservado</translation>
    </message>
    <message>
        <source>Image Formats</source>
        <translation>Formatos de imagem</translation>
    </message>
    <message>
        <source>Identify Formats</source>
        <translation>Identifica fomatos</translation>
    </message>
    <message>
        <source>Layer Count</source>
        <translation>Contar camadas</translation>
    </message>
    <message>
        <source>Layer Properties: </source>
        <translation>Propriedades da camada: </translation>
    </message>
    <message>
        <source>Selected</source>
        <translation>Selecionado</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Sim</translation>
    </message>
    <message>
        <source>No</source>
        <translation>Não</translation>
    </message>
    <message>
        <source>Visibility</source>
        <translation>Visibilidade</translation>
    </message>
    <message>
        <source>Visible</source>
        <translation>Visível</translation>
    </message>
    <message>
        <source>Hidden</source>
        <translation>Oculto</translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>n/a</translation>
    </message>
    <message>
        <source>Can Identify</source>
        <translation>Pode Identificar</translation>
    </message>
    <message>
        <source>Can be Transparent</source>
        <translation>Pode ser transparente</translation>
    </message>
    <message>
        <source>Can Zoom In</source>
        <translation>Pode aproximar</translation>
    </message>
    <message>
        <source>Cascade Count</source>
        <translation>Contador cascata</translation>
    </message>
    <message>
        <source>Fixed Width</source>
        <translation>Largura fixada</translation>
    </message>
    <message>
        <source>Fixed Height</source>
        <translation>Altura fixada</translation>
    </message>
    <message>
        <source>WGS 84 Bounding Box</source>
        <translation>Caixa de contorno WGS 84</translation>
    </message>
    <message>
        <source>Available in CRS</source>
        <translation>Disponível em CRS</translation>
    </message>
    <message>
        <source>Available in style</source>
        <translation>Disponível no estilo</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <source>Layer cannot be queried.</source>
        <translation>Camada não pode ser consultada.</translation>
    </message>
    <message>
        <source>Dom Exception</source>
        <translation>Exceção Dom</translation>
    </message>
</context>
<context>
    <name>QuickPrintGui</name>
    <message>
        <source>Portable Document Format (*.pdf)</source>
        <translation>Formato Portátil de Documento (*.pdf)</translation>
    </message>
    <message>
        <source>quickprint</source>
        <translation>imprimirrápido</translation>
    </message>
    <message>
        <source>Unknown format: </source>
        <translation>Formato Desconhecido: </translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <source>QGIS Quick Print Plugin</source>
        <translation>Plugin Imprimir Rápido</translation>
    </message>
    <message>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation>Note: se você quizer mais controle sobre o layout do mapa, por favor use a função compositor de mapas do QGIS.</translation>
    </message>
    <message>
        <source>Output</source>
        <translation type="unfinished">Saída</translation>
    </message>
    <message>
        <source>Use last filename but incremented.</source>
        <translation>Use o último nome de arquivo incrementado.</translation>
    </message>
    <message>
        <source>last used filename but incremented will be shown here</source>
        <translation>Último nome de arquivo usado será mostrado aqui</translation>
    </message>
    <message>
        <source>Prompt for file name</source>
        <translation>Pronto para o nome de arquivo</translation>
    </message>
    <message>
        <source>Page Size</source>
        <translation>Tamanho de página</translation>
    </message>
    <message>
        <source>Copyright</source>
        <translation>Copyright</translation>
    </message>
    <message>
        <source>Map Name e.g. Water Features</source>
        <translation>Nome do mapa e.g. Feições Hídricas</translation>
    </message>
    <message>
        <source>Map Title e.g. ACME inc.</source>
        <translation>Título do Mapa e.g. GEOGIS inc.</translation>
    </message>
    <message>
        <source>Quick Print</source>
        <translation>Imprimir rápido</translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <source>Quick Print</source>
        <translation>Imprimir Rápido</translation>
    </message>
    <message>
        <source>&amp;Quick Print</source>
        <translation>&amp;Imprimir Rápido</translation>
    </message>
    <message>
        <source>Provides a way to quickly produce a map with minimal user input.</source>
        <translation>Fornece um caminho para produzir rapidamente um mapa com mínimo de informações.</translation>
    </message>
</context>
<context>
    <name>RepositoryDetailsDialog</name>
    <message>
        <source>Repository details</source>
        <translation type="obsolete">Detalhes do repositório</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation type="obsolete">Nome:</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation type="obsolete">URL:</translation>
    </message>
    <message>
        <source>http://</source>
        <translation type="obsolete">http://</translation>
    </message>
</context>
<context>
    <name>[pluginname]GuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished">Modelo de Plugin para o QGIS</translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation type="unfinished">Modelo do Plugin</translation>
    </message>
</context>
<context>
    <name>dxf2shpConverter</name>
    <message>
        <source>Converts DXF files in Shapefile format</source>
        <translation>Convert Arquivos DXF em Shapefiles</translation>
    </message>
    <message>
        <source>&amp;Dxf2Shp</source>
        <translation>&amp;Dxf2SHP</translation>
    </message>
</context>
<context>
    <name>dxf2shpConverterGui</name>
    <message>
        <source>Polygon</source>
        <translation type="unfinished">Polígono</translation>
    </message>
    <message>
        <source>Point</source>
        <translation type="unfinished">Ponto</translation>
    </message>
    <message>
        <source>Dxf Importer</source>
        <translation>Importador DXF</translation>
    </message>
    <message>
        <source>Input Dxf file</source>
        <translation>Entre com o arquivo DXF</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Output file&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Output file&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Output file type</source>
        <translation>Tipo de arquivo de saída</translation>
    </message>
    <message>
        <source>Polyline</source>
        <translation>Polilinha</translation>
    </message>
    <message>
        <source>Export text labels</source>
        <translation>Exportar rótulos de texto</translation>
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
        <translation type="obsolete">Descrição dos campos: 
* Entrar com arquivo DXF: caminho para o arquivo s DXF a ser convertido 
* Arquivo SHP de saída: nome para o shapefile que será criado 
* Tipo de arquivo SHP: especifica o tipo de arquivo shapefile de saída 
* Exporta rótulos de texto selecionados: se selecionado, um arquivo SHP de pontos adicional será criado,   e a tabela DBF que conterá informações sobre os campos de &quot;TEXTO&quot; estabelecidos no arquivo DXF. 

--- 
Desenvolvido por Paolo L. Scala, Barbara Rita Barricelli, Marco Padula
CNR, Milan Unit (Information Technology), Construction Technologies Institute. 
For support send a mail to scala@itc.cnr.it

</translation>
    </message>
    <message>
        <source>Choose a DXF file to open</source>
        <translation type="obsolete">Escolha um arquivo DXF para abrir</translation>
    </message>
    <message>
        <source>Choose a file name to save to</source>
        <translation type="obsolete">Escolha um nome de arquivo para salvar como</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="obsolete">Troque isso por uma breve descrição do que o plugin faz</translation>
    </message>
    <message>
        <source>[menuitemname]</source>
        <translation type="unfinished">{nomedoitemdomenu]</translation>
    </message>
    <message>
        <source>&amp;[menuname]</source>
        <translation>&amp;[menuname]</translation>
    </message>
    <message>
        <source>Replace this with a short description of what the plugin does</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
