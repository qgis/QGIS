<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1">
<context>
    <name></name>
    <message>
        <location filename="" line="135533324"/>
        <source>Cannot read vector map region</source>
        <translation type="obsolete">ບໍ່ສາມາດອ່ານຂອບເຂດແຜ່ນທີ່ໆເປັນຕົວເລກບອກຄະຫນາດແລະທິດທາງ</translation>
    </message>
</context>
<context>
    <name>@default</name>
    <message>
        <location filename="" line="135533324"/>
        <source>OGR Driver Manager</source>
        <translation type="obsolete">OGR ໂຕຈັດການໂປຼແກມ 

</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>unable to get OGRDriverManager</source>
        <translation type="obsolete">ໂຕຈັດການໂປຼແກມ OGR ບໍສາມາດຮັບເອົາ</translation>
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
        <location filename="../python/plugins/plugin_installer/gui.ui" line="19"/>
        <source>Select repository, retrieve the list of available plugins, select one and install it</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="26"/>
        <source>Repository</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="34"/>
        <source>Active repository:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="61"/>
        <source>Get List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="81"/>
        <source>Add</source>
        <translation type="unfinished">ເພີ້ມ</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="88"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="95"/>
        <source>Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="117"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="122"/>
        <source>Version</source>
        <translation type="unfinished">ລຸ້ນ</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="127"/>
        <source>Description</source>
        <translation type="unfinished">ລາຍການ</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="132"/>
        <source>Author</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="142"/>
        <source>Name of plugin to install</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="152"/>
        <source>Install Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="163"/>
        <source>The plugin will be installed to ~/.qgis/python/plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="176"/>
        <source>Done</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished">ເຂົາຫາຈຸດພິກັດຂອງທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="62"/>
        <source>X:</source>
        <translation type="unfinished">X:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="69"/>
        <source>Y:</source>
        <translation>Y: </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="185"/>
        <source>&amp;OK</source>
        <translation>&amp;ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="172"/>
        <source>&amp;Cancel</source>
        <translation>&amp;ຍົກເລິກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="28"/>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>ໃຫ້ໃສ່ຕົວປະສານແກນ X ແລະ Y ຊື່ງໃຫ້ສອດຄ່ອງກັບຈຸດທີ່ເລືອກໄວ້ແລ້ວຢູ່ເທິງຮູບ ຫຼື ອີກທາງຫນື່ງໃຫ້ກົດເມົາສ໌ໃສ່ປຸ່ມເຄື່ອງຫມາຍສໍແລ້ວກົດໃສ່ຈຸດທີ່ຕອບສະຫນອງຢູ່ເທິງແຜ່ນທີ່ຂອງໂປຼກຼາມ QGIS ເພື່ອເຕີມເຕັມຕົວປະສານຂອງຈຸດນັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="137"/>
        <source> from map canvas</source>
        <translation type="unfinished">ຈາກພາບເເຜນທີ່</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="666"/>
        <source>Load layer properties from style file (.qml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="727"/>
        <source>Save layer properties as style file (.qml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="108"/>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="92"/>
        <source>No Data Providers</source>
        <translation type="unfinished">ບໍ່ມີຜູ້ໃຫ້ຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>No Data Provider Plugins</source>
        <comment>














No QGIS data provider plugins found in:
</comment>
        <translation type="obsolete">ບໍ່ມີຜູ້ໃຫ້ຂໍ້ມູນ ໂປຣກຣາມເສີມ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="89"/>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>ບໍມີລະດັບຊັ້ນທີ່ໂຫຼດໄດ້ທິດທາງ. ກວດການຕິດຕັ້ງໂປຼກຼາມ ຕັງໂປຼແກມ QGIS</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="251"/>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>ບໍ່ມີຕົວສະໜອງໂປຼກຼາມເສີມ (Plugin).ບໍ່ມີລະດັບຊັ້ນທີ່ບອກຄະໜາດແລະທິດທາງສາມາດໂຫຼດໄດ້ທາງສາມາດດືງເອົາ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2995"/>
        <source>QGis files (*.qgs)</source>
        <translation type="unfinished">ເເຟ້ມ QGis (*.qgs)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Open an OGR Supported Layer</source>
        <translation type="obsolete">ລະດັບຊັ້ນຮັບບຮອງໂປຼແກມ OGR ເປີດ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source> at line </source>
        <translation>ໃນເສັນ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="772"/>
        <source> column </source>
        <translation>ຖັນ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="778"/>
        <source> for file </source>
        <translation type="unfinished">ສໍາລັບເເຟ້ມ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="934"/>
        <source>Unable to save to file </source>
        <translation type="unfinished">ບໍ່ສາມາດບັນຖຶກໃສ່ເເຟ້ມ</translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="289"/>
        <source>Referenced column wasn&apos;t found: </source>
        <translation type="unfinished">ບໍພົບແຖວຕັງທີ່ອ້າງອີງ</translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="293"/>
        <source>Division by zero.</source>
        <translation type="unfinished">ການແບ່ງໂດຍເລກສູນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="75"/>
        <source>No active layer</source>
        <translation type="unfinished">ລະດັບຊັນບໍມີບົດບາດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>To identify features, you must choose an layer active by clicking on its name in the legend</source>
        <translation type="obsolete">ເພື່ອລະບຸຈຸດເດັ່ນຂອງແຕ່ລະດັບຊ້ັນ,ເລືອກເອົາລະດັບຊັ້ນທີ່ເຮັດວຽກຢູ່ໂດຍໃຊ້ມາວສ໌ກົດທີ່ຊື່ຂອງລະດັບຊັ້ນຢູ່ຄຳບັນຍາຍພາບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="154"/>
        <source>Band</source>
        <translation type="unfinished">ແທບເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="467"/>
        <source>action</source>
        <translation type="unfinished">ການປະຕິບັດງານ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="476"/>
        <source> features found</source>
        <translation type="unfinished">ມີຈຸດເດັ່ນພົບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="480"/>
        <source> 1 feature found</source>
        <translation>1 ພົບຈຸດເດັ່ນທີ່ນື່ງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="486"/>
        <source>No features found</source>
        <translation type="unfinished">ບໍ່ມີຈຸດເດັ່ນພົບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="486"/>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation type="unfinished">ບໍ່ມີຈຸດເດັ່ນພົບຢູ່ໃນລະດັບຊັ້ນທີ່ກຳລັງເຮັດວຽກຢູ່ຈຸດທີ່ກຳລັງກົດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="587"/>
        <source>Could not identify objects on</source>
        <translation type="unfinished">ບ່ໍສາມາດລະບຸວັດຖຸໄດ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="587"/>
        <source>because</source>
        <translation type="unfinished">ຍ້ອນວ່າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="72"/>
        <source>New centroid</source>
        <translation type="unfinished">ສູນກາງໄຫ່ມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>New point</source>
        <translation type="unfinished">ຈຸດໃຫມ່ </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="134"/>
        <source>New vertex</source>
        <translation type="unfinished">ຈຸດຈຸມເສັ້ນໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>Undo last point</source>
        <translation type="unfinished">ຍົກເລີກການສາ້ງຈຸດໃຫມ່ທີ່ສ້າງຄັ້ງສຸດທ້າຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>Close line</source>
        <translation type="unfinished">ປິດເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="543"/>
        <source>Select vertex</source>
        <translation type="unfinished">ເລືອກເສັ້ນຈອມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="296"/>
        <source>Select new position</source>
        <translation type="unfinished">ເລືອກຕຳແຫນ່ງໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="427"/>
        <source>Select line segment</source>
        <translation type="unfinished">ເສັ້ນແບ່ງເຄີ່ງກາງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="414"/>
        <source>New vertex position</source>
        <translation type="unfinished">ເປີດຕຳ່ແຫນ່ງເສັ້ນຈອມໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="414"/>
        <source>Release</source>
        <translation type="unfinished">ປ່ອຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="530"/>
        <source>Delete vertex</source>
        <translation>ລືບເສັ້ນຈອມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="530"/>
        <source>Release vertex</source>
        <translation type="unfinished">ປ່ອຍເສັ້ນຈອມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="784"/>
        <source>Select element</source>
        <translation type="unfinished">ເລື້ອກຖານ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="597"/>
        <source>New location</source>
        <translation type="unfinished">ຈຸດທີ່ຕັ້ງໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="673"/>
        <source>Release selected</source>
        <translation type="unfinished">ປ່ອຍຈຸດທີ່ເລືອກໄວ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="673"/>
        <source>Delete selected / select next</source>
        <translation>ລືບຈຸດທີ່ເລື້ອກໄວ້ / ເລື້ອກຕໍ່ໄປ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="736"/>
        <source>Select position on line</source>
        <translation type="unfinished">ເລື້ອກຕຳ່ແຫນ່ງທາງສາຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="754"/>
        <source>Split the line</source>
        <translation>ແຍກເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="754"/>
        <source>Release the line</source>
        <translation type="unfinished">ປ່ອຍເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="768"/>
        <source>Select point on line</source>
        <translation type="unfinished">ເລື້ອກຈຸດທາງສາຍ</translation>
    </message>
    <message>
        <location filename="../src/core/qgslabelattributes.cpp" line="58"/>
        <source>Label</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="454"/>
        <source>Length</source>
        <translation type="unfinished">ຄວາມຍາວ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="460"/>
        <source>Area</source>
        <translation type="unfinished">ພື້ນທີ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Could not snap segment. Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation type="obsolete">ບໍ່ສາມາດເອົາບາງສ່ວນໃດ້. ໃດ້ເລື່ອກຄວາມທົນທານຢູ່ໃນ ການກໍານົດ &gt; ຄົງການ ຄຸນສົມບັດ &gt; ທົວໄປ?</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Could not snap vertex. Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation type="obsolete">ບໍ່ສາມາດເອົາເສັ້ນຈອມໃດ້. ໃດ້ເລື່ອກຄວາມທົນທານຢູ່ໃນ ການກໍານົດ &gt; ຄົງການ ຄຸນສົມບັດ &gt; ທົວໄປ?</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source>Project file read error: </source>
        <translation>ການອ່ານແຟ້ມພິດພາດ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="32"/>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>ເພື່ືອໃຫ້ພໍ່ດີກັບເສັ້ນກົງຕ້ອງການຢ່າງຫນ້ອຍ 2 ຈຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="71"/>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>ເພ່ືອໃຫ້ແທດເຫມາະກັບການປ່ຽນແປງຂອງ Helmert ຕ້ອງການຢ່າງຫນ້ອຍ 2 ຈຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="123"/>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>ເພ່ືອໃຫ້ແທດເຫມາະກັບການປ່ຽນແປງ ຕ້ອງການຢ່າງຫນ້ອຍ 4 ຈຸດ</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="332"/>
        <source>Couldn&apos;t open the data source: </source>
        <translation type="unfinished">ບໍ່ສາດເປີດແຫລ່ງທີ່ມາຂອງຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="354"/>
        <source>Parse error at line </source>
        <translation>ກະຈາຍຂໍຜິດພາດໄປຕາມເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="55"/>
        <source>GPS eXchange format provider</source>
        <translation>ຜູ້ສະຫນອງການແລກປ່ຽນຂໍ້ມູນທາງ ຈີພີເອດສ໌ (GPS)</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="303"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>ຮັບຜິດພາດລະບົບຕົວປະສານຕອນກໍາລັງທໍາບການປ່ຽນແປງຈຸດ ບໍ່ສາມາດຄິດໄລຄວາມຍາວຂອງເສັ້ນ..</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="394"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation>ຮັບຜິດພາດລະບົບຕົວປະສານຕອນກໍາລັງທໍາການປ່ຽນແປງຈຸດ. ບໍ່ສາມາດຄິດໄລເນື້ອທີຂອງຮູບຫລາຍລຽມ.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="166"/>
        <source>GRASS plugin</source>
        <translation>ໂປຼກຼາມເສີມ GRASS</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="136"/>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>QGIS ບໍສາມາດພົບການຕິດຕັ້ງ GRASS ເຂົ້າໄປໃນລະບົບ,ທ່ານຍາກລະບຸເສັ້ນທາງຂອງ GISBASE ເພື່ອຕິດຕັ້ງເຂົ້າໄປໃນລະບົບບໍ່?</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="150"/>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>ເລື້ອກເສັ້ນທາງຂອງ GISBASE ເພື່ອຕິດຕັ້ງ GRASS ເຂົ້າໄປໃນລະບົບ</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="167"/>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>ຖານຂໍ້ມູນ GRASS ຈະບໍ່ມີ ຖ້າບໍ່ລະບຸ GISBASE</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="51"/>
        <source>CopyrightLabel</source>
        <translation type="unfinished">ລົງຊື່ລິຄະສິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="52"/>
        <source>Draws copyright information</source>
        <translation type="unfinished">ດືງເອົາຂໍ້ມູນທາງລິຄະສິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="31"/>
        <source>Version 0.1</source>
        <translation>ລູ້ນ 0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="44"/>
        <source>Version 0.2</source>
        <translation>ລູ້ນ 0.2</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="45"/>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation type="unfinished">ໂຫຼດ ແລະ ສະແດງ ແຟ້ມທີ່ມີຕົວປະສານ x,y</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="161"/>
        <source>Add Delimited Text Layer</source>
        <translation type="unfinished">ເພີ້ມລະດັບຊັ້ນຂອງເອກະສານ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="57"/>
        <source>Georeferencer</source>
        <translation>ທໍລະນີສາດ (Georeferencer)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="58"/>
        <source>Adding projection info to rasters</source>
        <translation>ເພີ້ມຂໍ້ມູນການກະໃວ້ ເຂົ້າໃສ່ ເເຣ້ເຕີ (rasters)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="55"/>
        <source>GPS Tools</source>
        <translation type="unfinished">ເຄື່ອງມື GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="57"/>
        <source>Tools for loading and importing GPS data</source>
        <translation>ເຄື່ອງມືສຳລັບໂຫຼດແລະນຳຂໍ້ມູນ GPS ເຂົ້າມາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="829"/>
        <source>GRASS</source>
        <translation>ເເກຣດໂປຣກຣາມ (GRASS)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="835"/>
        <source>GRASS layer</source>
        <translation>ລະດັບຊັ້ນ GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="43"/>
        <source>Graticule Creator</source>
        <translation type="unfinished">ເຄື່ອງສ້າງ Graticule</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="44"/>
        <source>Builds a graticule</source>
        <translation type="unfinished">ສ້າງ Graticule</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="60"/>
        <source>NorthArrow</source>
        <translation type="unfinished">ລູກສອນຊີ້ຂື້ນເທີງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="61"/>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation type="unfinished">ສະແດງລູກສອນຊີ້ຂື້ນເທີງທັບເທິງແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="38"/>
        <source>[menuitemname]</source>
        <translation>[ຊື່ຂອງເມນູ]</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="39"/>
        <source>[plugindescription]</source>
        <translation>[ຄຳອະທິບານຂອງໂປຼກຼາມເສີມ]</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="62"/>
        <source>ScaleBar</source>
        <translation type="unfinished">ແທບມາດວັດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="63"/>
        <source>Draws a scale bar</source>
        <translation type="unfinished">ລາກເສັ້ນມາດວັດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="37"/>
        <source>SPIT</source>
        <translation>SPIT</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="38"/>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>ຈັດແຟ້ມໃຫ້ສອດຄ່ອງກັບ PostgreSQL/PostGIS ເຄື່ອງມືການນຳເຂົ້າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="29"/>
        <source>WFS plugin</source>
        <translation>ໂປຼກຼາມເສີມ WFS</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="30"/>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>ເພີ້ມລະດັບຊັ້ນຂອງ WFSໃສ່ QGIS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Version 0.0001</source>
        <translation type="obsolete">ລູ້ນ ໐.໐໐໐໐1</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="42"/>
        <source>Not a vector layer</source>
        <translation type="unfinished">ບໍ່ມີລະດັບຊັ້ນຂອງຈຳນວນຕົວເລກບອກຄະຫນາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="43"/>
        <source>The current layer is not a vector layer</source>
        <translation type="unfinished">ລະດັບຊັ້ນປະຈຸບັນບໍ່ແມ່ນລະດັບຊັ້ນບອກຕົວເລກຄະຫນາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="72"/>
        <source>Layer cannot be added to</source>
        <translation type="unfinished">ບໍ່ສາມາດເພີ້ມລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="73"/>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation type="unfinished">ຕົວສະຫນອງຂໍ້ມູນໃຫ້ລະດັບຊັ້ນນີ້ບໍ່ອະນຸຍາດໃຫ້ເພີ້ມເຕີມລາຍລະອຽດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="49"/>
        <source>Layer not editable</source>
        <translation type="unfinished">ລະດັບຊັ້ນບໍ່ສາມາດແກ້ໄຂ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="51"/>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>ບໍ່ສາມາດແກ້ໄຂລະດັບຊັ້ນຕົວເລກຄະຫນາດແລະທິດທາງ,ເພື່ອເຮັດໃຫ້ສາມາດແກ້ໄຂໄດ້,ໃຫ້ໄປທີ່ຫົວຂໍ້ຂອງແຟ້ມຂອງລະດັບຊັ້ນ ແລ້ວກົດ Mouse ເບື້ອງຂວາເພື່ອກວດເບີ່ງວ່າມີຄຳສັ່ງໃຫ້ອະນຸຍາດແກ້ໄຂໄດ້ບໍ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="76"/>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>ເພື່ອເລື້ອກເອົາຈຸດລາຍລະອຽດ, ພວກເຮົາຕ້ອງເລື້ອກເອົາລະດັບຊັ້ນຂອງຕົວເລກບອກຄະຫນາດແລະທິດທາງໂດຍກົດໄປທີຊື່ຂອງຊັ້ນນັ້ນຢູ່ໃນຄຳບັນຍາຍຂອງແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="193"/>
        <source>Python error</source>
        <translation>ມີຂໍ້ຜິດພາດກັບ Python</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Couldn&apos;t load SIP module.
Python support will be disabled.</source>
        <translation type="obsolete">ບໍ່ສາມາດໂຫຼດ SIP ສ່ວນໂປຣກຣາມ.ໂປຼກຼາມສະຫນັບສະຫນຸນ Python ຖືກປິດ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Couldn&apos;t load PyQt bindings.
Python support will be disabled.</source>
        <translation type="obsolete">ໍບໍ່ສາມາດໂຫຼດຕົວປະສານ PyQt. ການເຂົ້າກັນຂອງ Python ຈະຖືກປິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Couldn&apos;t load QGIS bindings.
Python support will be disabled.</source>
        <translation type="obsolete">ໍບໍ່ສາມາດໂຫຼດຕົວປະສານ QGIS. ການເຂົ້າກັນຂອງ Python ຈະຖືກປິດ</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="477"/>
        <source>Couldn&apos;t load plugin </source>
        <translation>ບໍ່ສາມາດໂຫຼດໂປຼກຼາມເສີມ (plugin)</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="481"/>
        <source> due an error when calling its classFactory() method</source>
        <translation>ເນື່ອງຈາກມີການຜິດພາດເກີດຂື້ນໃນເມື່ອຮ້ອງ  classFactory() ໃຊ້ຮູບແບບການຈັດທີ່ມານຳໂປຼກຼາມ</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="485"/>
        <source> due an error when calling its initGui() method</source>
        <translation>ເນື່ອງຈາກມີການຜິດພາດເກີດຂື້ນໃນເມື່ອຮ້ອງໃຊ້ຮູບແບບການຈັດຂອງ initGui()</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="497"/>
        <source>Error while unloading plugin </source>
        <translation>ມີການຜິດພາດເກີດຂື້ນໃນຄະນະທີ່ປ້ອນໂປຼກຼາມເສີມ (Plugin)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="59"/>
        <source>2.5D shape type not supported</source>
        <translation type="unfinished">2.5 D shape Type ບໍ່ເຂົ້າກັບໂປຼກຼາມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="59"/>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation type="unfinished">ການເພີ້ມຈຸດເດັ່ນ ໃຫ້ແກ່ 2.5 D shape type ຍັງບໍ່ທັນເຂົ້າກັນກັບໂປຼກຼາມເທື່ອ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="207"/>
        <source>Wrong editing tool</source>
        <translation type="unfinished">ໃຊ້ເຄື່ອງມືໃນການດັດແກ້ຜິດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="92"/>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation type="unfinished">ບໍ່ສາມາດນຳໃຊ້ເຄື່ອງມືໃນການຊີ້ຈຸດບົນລະດັບຊັ້ນທີ່ບອກຄະຫນາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="65"/>
        <source>Coordinate transform error</source>
        <translation type="unfinished">ການປ່ຽນແປງຕົວປະສານມີການຜິດພາດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="66"/>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation type="unfinished">ບໍ່ສາມາດປ່ຽນແປງຈຸດໃດນື່ງໄປຫາລະດັບຊ້ັນຂອງລະບົບປະສານ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="200"/>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation type="unfinished">ບໍ່ສາມາດນຳໃຊ້ອຸປະກອນຈັບເສັ້ນຢູ່ເທີງລະດັບຊັ້ນຂອງຕົວເລກຄະຫນາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="208"/>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation type="unfinished">ບໍ່ສາມາດນຳໃຊ້ເຄື່ອງມືຮູບຫຼາຍຫຼຽມຢູ່ບົນລະດັບຊັ້ນຂອງຕົວເລກຄະຫນາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="427"/>
        <source>Error</source>
        <translation type="unfinished">ຂໍ້ຜິດພາດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="416"/>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>ບໍ່ສາມາດເພີ້ມຈຸດເດັ່ນ.ຊະນິດຂອງແຟ້ມ ‍‍‍WKB ບໍ່ຮູ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdisland.cpp" line="113"/>
        <source>Error, could not add island</source>
        <translation>ມີຂໍ້ຜິດພາດ,ບໍ່ສາມາດເພີ້ມເກາະ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="91"/>
        <source>A problem with geometry type occured</source>
        <translation>ມີບັນຫາເກີ້ດຂື້ນກັບຊະນິດຂອງຄະນິດສາດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="95"/>
        <source>The inserted Ring is not closed</source>
        <translation>ບໍ່ໄດ້ປີດວົງແຫວນ (Ring)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="99"/>
        <source>The inserted Ring is not a valid geometry</source>
        <translation>ວົງແຫວນ (Ring) ທີ່ສອດເຂົ້າໄປເປັນຄະນິດສາດໃຊ້ການບໍ່ໄດ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="103"/>
        <source>The inserted Ring crosses existing rings</source>
        <translation>ການສ້າງວົງແຫວນ (Ring) ເພີ້ມເຕີ່ມຈະທັບວົງແຫວນທີ່ມີຢູ່ແລ້ວ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="107"/>
        <source>The inserted Ring is not contained in a feature</source>
        <translation>ວົງແຫວນ (Ring) ທີ່ໃສ່ເພີ້ມເຕີ່ມບໍ່ມີຈຸດເດັ່ນອະທິບາຍຢູ່ໃນມັນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="111"/>
        <source>An unknown error occured</source>
        <translation type="unfinished">ມີຂໍ້ຜິດພາດເກີດຂື້ນໂດຍບໍ່ຮູ້ສາເຫດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="113"/>
        <source>Error, could not add ring</source>
        <translation>ມີຂໍ້ຜິດພາດ,ບໍ່ສາມາດເພີ້ມວົງແຫວນ (Ring)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Change geometry</source>
        <translation type="obsolete">ປ່ຽນສັດສ່ວນທາງເລຂາຄະນິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Data provider of the current layer doesn&apos;t allow changing geometries</source>
        <translation type="obsolete">ຕົວສະຫນອງຂໍ້ມູນຂອງລະດັບຊັ້ນປະຈຸບັນບໍ່ອະນຸຍາດໃຫ້ປ່ຽນແປງສັດສ່ວນທາງເລຂາຄະນິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cannot edit the vector layer. Use &apos;Start editing&apos; in the legend item menu</source>
        <translation type="obsolete">ບໍ່ສາມາດແກ້ໄຂລະດັບຊັ້ນຂອງຄະຫນາດແລະທິດທາງ.ນຳໃຊ້ &apos;ເລີ້ມ ເເກ້ໃຂ&apos; ຢູ່ໃນຄຳອະທິບາຍແຜນທີ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="640"/>
        <source> km2</source>
        <translation>ກລ2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="645"/>
        <source> ha</source>
        <translation type="unfinished">ha</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="650"/>
        <source> m2</source>
        <translation>ມ2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="678"/>
        <source> m</source>
        <translation>ມ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="663"/>
        <source> km</source>
        <translation>ກລ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="668"/>
        <source> mm</source>
        <translation>ມມ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="673"/>
        <source> cm</source>
        <translation>ຊມ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="687"/>
        <source> sq mile</source>
        <translation type="unfinished">ຕະລາງໄມ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="692"/>
        <source> sq ft</source>
        <translation type="unfinished">ຕະລາງຟຸດ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="699"/>
        <source> mile</source>
        <translation type="unfinished">ໄມ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="705"/>
        <source> foot</source>
        <translation type="unfinished">ຟຸດ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="707"/>
        <source> feet</source>
        <translation type="unfinished">ຫຼາຍຟຸດ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="714"/>
        <source> sq.deg.</source>
        <translation type="unfinished">ຕາອົງສາ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="719"/>
        <source> degree</source>
        <translation type="unfinished">ອົງສາ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="721"/>
        <source> degrees</source>
        <translation type="unfinished">ຫຼາຍອົງສາ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="725"/>
        <source> unknown</source>
        <translation type="unfinished">ບໍ່ຮູ້</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="273"/>
        <source>Received %1 of %2 bytes</source>
        <translation>ໄດ້ຮັບ %1 ຂອງ %2 ໄບ້ທ (bytes)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="279"/>
        <source>Received %1 bytes (total unknown)</source>
        <translation>ໄດ້ຮັບ %1 ຂອງ ໄບ້ທ (bytes) (ບໍ່ຮູ້ຈັກຈຳນວນທັງຫມົດ)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="390"/>
        <source>Not connected</source>
        <translation type="unfinished">ບໍ່ສາມາດເຊື່ອມຕໍ່ລະບົບ</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="396"/>
        <source>Looking up &apos;%1&apos;</source>
        <translation>ກຳລັງຊອກຫາ &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="403"/>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>ກຳລັງເຊື່ອມຕໍ່ &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="410"/>
        <source>Sending request &apos;%1&apos;</source>
        <translation>ຄຳຂໍທີ່ກຳລັງສົ່ງໄປໄດ້ %1</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="417"/>
        <source>Receiving reply</source>
        <translation type="unfinished">ໄດ້ຮັບຄຳຕອບ</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="423"/>
        <source>Response is complete</source>
        <translation type="unfinished">ໄດ້ຮັບຄຳຕອບສົມບູນ</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="429"/>
        <source>Closing down connection</source>
        <translation type="unfinished">ປິດການເຊື່ອມຕໍ່ໍລະບົບ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="753"/>
        <source>Unable to open </source>
        <translation type="unfinished">ບໍ່ສາມາດເປີດໄດ້</translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="253"/>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation type="unfinished">ປະໂຫຍກທຳມະດາກ່ຽວກັບຄ່າຂອງຕົວເລກບໍ່ມີຄວາມຫມາຍ.ຈົ່ງໃຊ້ຄຳປຽບທຽບແທນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>PostgresSQL Geoprocessing</source>
        <translation type="obsolete">PostgresSQL Geoprocessing</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="48"/>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Geoprocessing ກໍາລັລປະຕິບັດຫນ້າທີ່ຕໍ່ PostgreSQL/PostGIS ລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Location: </source>
        <comment>


















Metadata in GRASS Browser
</comment>
        <translation type="obsolete">ທີ່ຕັ້ງ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;br&gt;Mapset: </source>
        <comment>


















Metadata in GRASS Browser
</comment>
        <translation type="obsolete">&lt;br&gt;ຊຸດແຜນທີ່:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="141"/>
        <source>Location: </source>
        <translation>ທີ່ຕັ້ງ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="141"/>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;ຊຸດແຜນທີ່:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="146"/>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation>&lt;b&gt;ເເຣດຊເຕີ (Raster)&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="155"/>
        <source>Cannot open raster header</source>
        <translation>ໍບໍ່ສາມາດເປີດໃຊ້ ເເຣດຊເຕີ (raster) ຫົວຂໍ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="159"/>
        <source>Rows</source>
        <translation type="unfinished">ແຖວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="160"/>
        <source>Columns</source>
        <translation type="unfinished">ຖັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="161"/>
        <source>N-S resolution</source>
        <translation>ຄວາມລະອຽດຂອງ N-S</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="162"/>
        <source>E-W resolution</source>
        <translation>ຄວາມລະອຽດຂອງ E-W</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="300"/>
        <source>North</source>
        <translation type="unfinished">ທິດເຫນືອ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="302"/>
        <source>South</source>
        <translation type="unfinished">ທິດໄຕ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="304"/>
        <source>East</source>
        <translation type="unfinished">ທິດຕາເວັນອອກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="306"/>
        <source>West</source>
        <translation type="unfinished">ທິດຕາເວັນຕົກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="189"/>
        <source>Format</source>
        <translation>ຮູບແບບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="200"/>
        <source>Minimum value</source>
        <translation type="unfinished">ຄ່າຕຳສຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="201"/>
        <source>Maximum value</source>
        <translation type="unfinished">ຄ່າສູງສຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="212"/>
        <source>Data source</source>
        <translation type="unfinished">ແຫລ່ງຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="217"/>
        <source>Data description</source>
        <translation type="unfinished">ລາຍລະອຽດຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="226"/>
        <source>Comments</source>
        <translation type="unfinished">ຄຳເຫັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="241"/>
        <source>Categories</source>
        <translation type="unfinished">ປະເພດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="347"/>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;ເເວກເຕີ (Vector)&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="274"/>
        <source>Points</source>
        <translation type="unfinished">ຈຸດຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="275"/>
        <source>Lines</source>
        <translation type="unfinished">ເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="276"/>
        <source>Boundaries</source>
        <translation type="unfinished">ເຂດແດນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="277"/>
        <source>Centroids</source>
        <translation>ເຊັນທຣອຍ (Centroids)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="280"/>
        <source>Faces</source>
        <translation type="unfinished">ໃບຫນ້າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="281"/>
        <source>Kernels</source>
        <translation>ເຄີນໍ (Kernels)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="284"/>
        <source>Areas</source>
        <translation type="unfinished">ພື້ນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="285"/>
        <source>Islands</source>
        <translation type="unfinished">ເກາະຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="309"/>
        <source>Top</source>
        <translation type="unfinished">ເທີງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="310"/>
        <source>Bottom</source>
        <translation type="unfinished">ພື້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="313"/>
        <source>yes</source>
        <translation type="unfinished">ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="313"/>
        <source>no</source>
        <translation type="unfinished">ບໍ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="320"/>
        <source>History&lt;br&gt;</source>
        <translation>ປະຫວັດ&lt;br&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="348"/>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;ລະດັບຊັ້ນ&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="367"/>
        <source>Features</source>
        <translation type="unfinished">ຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="376"/>
        <source>Driver</source>
        <translation>ຊອບເເວ ໃຊ້ຄວບຄຸມອຸປະກອນຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="377"/>
        <source>Database</source>
        <translation type="unfinished">ຖານຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="378"/>
        <source>Table</source>
        <translation type="unfinished">ຕາຕະລາງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="379"/>
        <source>Key column</source>
        <translation>ຖັນຫລັກ</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="453"/>
        <source>GISBASE is not set.</source>
        <translation>ບໍ່ໄດ້ຕັ້ງຖານຂໍ້ມູນ GISBASE</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="458"/>
        <source> is not a GRASS mapset.</source>
        <translation>ບໍ່ແມ່ນຊຸດແຜ່ນທີ່ຂອງໂປຼກຼາມ GRASS</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="480"/>
        <source>Cannot start </source>
        <translation type="unfinished">ບໍ່ສາມາດເລີ້ມໄດ້</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="497"/>
        <source>Mapset is already in use.</source>
        <translation type="unfinished">ຊຸດແຜ່ນທີ່ພ້ອມເຮັດວຽກ</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="512"/>
        <source>Temporary directory </source>
        <translation type="unfinished">ຕູ້ເກັບແຟ້ມຊົ່ວຄາວ</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="512"/>
        <source> exist but is not writable</source>
        <translation type="unfinished">ແຟ້ມທີ່ມີຢູ່ບໍ່ສາມາດຂຽນເພີ້ມເຕີ່ມໄດ້</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="518"/>
        <source>Cannot create temporary directory </source>
        <translation type="unfinished">ບໍ່ສາມາດສ້າງຕູ້ແຟ້ມຊົ່ວຄາວ</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="534"/>
        <source>Cannot create </source>
        <translation type="unfinished">ໍບໍ່ສາມາດສ້າງ</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="611"/>
        <source>Cannot remove mapset lock: </source>
        <translation type="unfinished">ບໍ່ສາມາດເອົາຊຸດແຜ່ນທີ່ອອກ(ຖືກລ໋ອກ)</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1051"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="999"/>
        <source>Cannot read raster map region</source>
        <translation>ບໍ່ສາມາດອ່ານຂອບເຂດ raster ແຜ່ນທີ່</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1052"/>
        <source>Cannot read region</source>
        <translation type="unfinished">ບໍ່ສາມາດອ່ານຂອບເຂດໃດຫນື່ງຂອງແຜ່ນທີ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2422"/>
        <source>Where is &apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2422"/>
        <source>original location: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="427"/>
        <source>Could not remove polygon intersection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="123"/>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="521"/>
        <source>Loaded default style file from </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="552"/>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="565"/>
        <source>Created default style file as </source>
        <translation type="unfinished"></translation>
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
        <location filename="../src/core/qgsproviderregistry.cpp" line="87"/>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation type="unfinished">ບໍ່ມີຜູ້ໃຫ້ຂໍ້ມູນ ໂປຣກຣາມເສີມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="47"/>
        <source>PostgreSQL Geoprocessing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="137"/>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation type="unfinished">ທີ່ຕັ້ງ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="137"/>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation type="unfinished">&lt;br&gt;ຊຸດແຜນທີ່:</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsgridfilewriter.cpp" line="61"/>
        <source>Interpolating...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsgridfilewriter.cpp" line="61"/>
        <source>Abort</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished">ລູ້ນ ໐.໐໐໐໐1 {0.001?}</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="38"/>
        <source>Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="40"/>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation type="unfinished"></translation>
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
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1016"/>
        <source>Cannot read vector map region</source>
        <translation type="unfinished">ບໍ່ສາມາດອ່ານຂອບເຂດແຜ່ນທີ່ໆເປັນຕົວເລກບອກຄະຫນາດແລະທິດທາງ</translation>
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
        <location filename="../src/app/qgisapp.cpp" line="345"/>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS -</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Version </source>
        <translation type="obsolete">ລຸ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> with PostgreSQL support</source>
        <translation type="obsolete">ເຂົ້າກັນກັບໂປຼກຼາມ PostgreSQL </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> (no PostgreSQL support)</source>
        <translation type="obsolete">( ບໍ່ເຂົ້າກັນກັບ PostgreSQL)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1625"/>
        <source>Version</source>
        <translation>ລຸ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Available Data Provider Plugins</source>
        <translation type="obsolete">ໂປຼກຼາມເສີມ [Plugin] ເຊິ່ງເປັນຕົວປ້ອນຂໍ້ມູນມີໃຫ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2081"/>
        <source>is not a valid or recognized data source</source>
        <translation type="unfinished">ບໍ່ແມ່ນແຫຼ່ງຂໍ້ມູນທີ່ໄດ້ຮັບການຈົດຈຳຫຼືໃຊ້ການບໍ່ໄດ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5184"/>
        <source>Invalid Data Source</source>
        <translation type="unfinished">ແຫຼ່ງຂໍ້ມູນໃຊ້ການບໍ່ໄດ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3487"/>
        <source>No Layer Selected</source>
        <translation type="unfinished">ບໍ່ໄດ້ເລື້ອກລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4048"/>
        <source>No MapLayer Plugins</source>
        <translation>ບໍ່ມີໂປຼກຼາມເສີມຂອງລະດັບຊັ້ນແຜ່ນທີ່ (MapLayer)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4048"/>
        <source>No MapLayer plugins in ../plugins/maplayer</source>
        <translation>ບໍ່ມີໂປຼກຼາມເສີມຂອງລະດັບຊັ້ນແຜ່ນ (MapLayer) ທີ່ຢູ່ໃນ../plugins/maplayer </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4125"/>
        <source>No Plugins</source>
        <translation>ບໍ່ມີໂປຼກຼາມເສີມ (Plugins)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4126"/>
        <source>No plugins found in ../plugins. To test plugins, start qgis from the src directory</source>
        <translation>ບໍ່ພົບໂປຼກຼາມເສີມ (plugins) ຢູ່ໃນ../plugins ເພື່ອທົດສອບໂປຼກຼາມເສີມ (Plugins)、ໃຫ້ໃຊ້ໂປຼກຼາມ qgis ຈາກ ຕູ້ເກັບແຟ້ມ src</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4160"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4160"/>
        <source>Plugin %1 is named %2</source>
        <translation>ໂປຼກຼາມເສີມ (Plugins) %1 ຖືກໃສ່ຊື່ໄປ່ແລ້ວ %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4177"/>
        <source>Plugin Information</source>
        <translation>ຂໍ້ມູນຂອງໂປຼກຼາມເສີມ (Plugins)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>QGis loaded the following plugin:</source>
        <translation>QGIS ໄດ້ໂຫຼດໂປຼກຼາມເສີມ (Plugin) ດັ່ງຕໍ່ໄປນີ້:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>Name: %1</source>
        <translation>ຊື່: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>Version: %1</source>
        <translation>ລຸ້ນ: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4179"/>
        <source>Description: %1</source>
        <translation type="unfinished">ລາຍການ: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4197"/>
        <source>Unable to Load Plugin</source>
        <translation>ບໍ່ສາມາດໂຫຼດໂປຼກຼາມເສີມ (Plugin)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4198"/>
        <source>QGIS was unable to load the plugin from: %1</source>
        <translation>QGIS ບໍ່ສາມາດໂຫຼດໂປຼກຼາມເສີມ (Plugin) ຈາກ: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4254"/>
        <source>There is a new version of QGIS available</source>
        <translation>ມີ QGIS ລູ້ນໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4260"/>
        <source>You are running a development version of QGIS</source>
        <translation>ທ່ານກຳລັງໃຊ້ລຸ້ນທີ່ໄດ້ຮັບການພັດທະນາຂື້ນຂອງໂປຼກຼາມ QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4264"/>
        <source>You are running the current version of QGIS</source>
        <translation> ທ່ານກຳລັງໃຊ້ລຸ້ນປະຈຸບັນຂອງອງໂປຼາມ ຼກQGです</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4269"/>
        <source>Would you like more information?</source>
        <translation type="unfinished">ທ່ານຢາກໄດ້ຂໍ້ມູນເພີ້ມເຕີມບໍ່?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4316"/>
        <source>QGIS Version Information</source>
        <translation>ຂໍ້ມູນລຸ້ນ QGIS </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4288"/>
        <source>Unable to get current version information from server</source>
        <translation>ບໍ່ສາມາດເອົາຂໍ້ມູນເວີຊັນປະຈຸບັນຈາກຫນ່ວຍເຄືອຄ່າຍແມ່ (Server)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4302"/>
        <source>Connection refused - server may be down</source>
        <translation>ການເຊື່ອມຕໍ່ໄດ້ຮັບການປະຕິເສດ,ຫນ່ວຍເຄືອຄ່າຍແມ່ (Server) ອາດມອດເຄື່ອງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4305"/>
        <source>QGIS server was not found</source>
        <translation>ຫນ່ວຍເຄືອຄ່າຍແມ່  QGIS (Server) ບໍ່ພົບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2173"/>
        <source>Invalid Layer</source>
        <translation type="unfinished">ລະດັບຊັ້ນໃຊ້ການບໍ່ໄດ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2173"/>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>%1 ເປັນລະດັບຊັ້ນທີ່ໃຊ້ການບໍ່ໄດ້ແລະ ບໍ່ສາມາດໂຫຼດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4012"/>
        <source>Error Loading Plugin</source>
        <translation>ມີການຜິດພາດໃນການໂຫຼດໂປຼກຼາມເສີມ (Plugin)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4012"/>
        <source>There was an error loading %1.</source>
        <translation>ມີການຜິດພາດເກີດຂື້ນໃນການໂຫຼດ %1 </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3276"/>
        <source>Saved map image to</source>
        <translation>ຈັດເກັບຮູບແຜນທີ່ໄປຍັງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3234"/>
        <source>Choose a filename to save the map image as</source>
        <translation type="unfinished">ເລື້ອກຊື່ແຟ້ມເພື່ອຈັດເກັບຮູບແຜນທີ່ໃຫ້ເປັນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4684"/>
        <source>Extents: </source>
        <translation>ຂອບເຂດດ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3517"/>
        <source>Problem deleting features</source>
        <translation type="unfinished">ມີປັນຫາໃນການລືບຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3518"/>
        <source>A problem occured during deletion of features</source>
        <translation type="unfinished">ມີປັນຫາເກີດຂື້ນລະຫວ່າງລືບຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3495"/>
        <source>No Vector Layer Selected</source>
        <translation type="unfinished">ບໍ່ມີລະດັບຊັ້ນຕົວເລກທີບອກຄະຫນາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3496"/>
        <source>Deleting features only works on vector layers</source>
        <translation type="unfinished">ການລືບຈຸດເດັ່ນມີຜົນກັບລະດັບຊັ້ນທີ່ບອກຄະຫນາດແລະທິດທາງພຽງເທົ່ານັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3488"/>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation type="unfinished">ເພື່ອລືບຈຸດເດັ່ນ,ທ່ານຈະຕ້ອງເລື້ອກເອົາລະດັບຊັ້ນຂອງຕົວເລກບອກຄະຫນາດແລະທິດທາງຢູ່ໃນຄຳອະທິບາຍຂອງແຜນທີ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation type="obsolete">ໂປຼກຼາມ Quantum GIS ໄດ້ຮັບອະນຸຍາດພາຍໃຕ້ໃບອະນຸຍາດຂອງ (GNU General Public License)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>http://www.gnu.org/licenses</source>
        <translation type="obsolete">ທີ່ຢູ່ອີນເຕີເນດ http://www.gnu.org/licenses/licenses.ja.html</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1215"/>
        <source>Render</source>
        <translation>ປະສົມປະສານ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2921"/>
        <source>Choose a QGIS project file</source>
        <translation>ເລື້ອກແຟ້ມຫນ້າວຽກຂອງ QGIS </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3045"/>
        <source>Unable to save project</source>
        <translation type="unfinished">ບໍ່ສາມາດຈັດເກັບຫນ້າວຽກ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3046"/>
        <source>Unable to save project to </source>
        <translation>ບໍ່ສາມາດຈັດເກັບຫນ້າວຽກໄປຍັງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1464"/>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>ຄຳອະທິບາຍແຜນທີ່ເຊິ່ງປະຈຸບັນກຳລັງສະແດງລະດັບຊັ້ນທັງຫມົດຢູ່ເທິງພື້ນແຜນທີ່.ກົດໃສ່ກ່ອງກວດ (check box) ເພື່ອເປີດແລະປິດ.ກົດເມົາສ໌ສອງຄັ້ງໃສ່ລະດັບຊັ້ນໃນຄຳອະທິບາຍແຜນທີ່ເພື່ອຈັດການພາບຫນ້າຈໍທີ່ເຫັນໃຫ້ຕົງຕາມຄວາມຕ້ອງການຂອງຕົນແລະຕັ້ງຄຸນລັກສະນະອື່ນ ໆ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1425"/>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation type="unfinished">ພາບພື້ນແຜນທີ່ໂດຍລວມ.ພາບພື້ນນີ້ສາມາດນຳໃຊ້ເພື່ອສະແດງຕົວຄົ້ນຫາທີ່ຕັ້ງຂອງແຜນທີ່ເຊິ່ງເຮັດໃຫ້ເຫັນຂອບເຂດຂອງພື້ນແຜນທີ່ປະຈຸບັນ.ຂອບເຂດປະຈຸບັນຂອງແຜນທີ່ໄດ້ສະແດງເປັນສີ່ຫຼ່ຽມແດງ.ລະດັບຊັ້ນໃດໆກໍ່ຕາມຢູ່ແຜນທີ່ສາມາດເພີ້ມເຂົ້າໄປພື້ນແຜນທີ່ໄດ້.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation type="obsolete">ພື້ນແຜນທີ.ນີ້ແມ່ນບ່ອນທີ່ລະດັບຊັ້ນຂອງຕົວເລກທີ່ບອກຄະໜາດແລະທິດທາງແລະລະດັບຊັ້ນ Raster ຖືກສະແດງຢູ່ບ່ອນທີ່ເຮົາເພີ້ມເຂົ້າໄປໃສ່ແຜນທີ່.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1023"/>
        <source>&amp;Plugins</source>
        <translation>&amp;ໂປຼກຼາມເສີມ (Plugins)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1160"/>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>ຕົວບອກຄວາມກ້າວຫນ້າສະເເດງສະພາບຂອງການປະສົມປະສານລະດັບຊັ້ນ ເເລະ ຕົວທໍາຫນັາ ອື່ນ ຯ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1189"/>
        <source>Displays the current map scale</source>
        <translation type="unfinished">ສະແດງມາດຕາສ່ວນປະຈຸບັນຂອງແຜນທີ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Shows the map coordinates at the current cursor postion. The display is continuously updated as the mouse is moved.</source>
        <translation type="obsolete">ສະແດງຕົວປະສານແຜນທີ່ຢູ່ຕຳແຫນງເມັດເຄີເສີ້ປະຈຸບັນ,ພາບທີ່ສະແດງຢູ່ຫນ້າຈໍກຳລັງສະແດງຄວາມຄືບຫນ້າຢ່າງຕໍ່ເນື່ອງໃນຄະນະທີ່ເມົາສ໌ກຳລັງຄືບໄປໃ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1221"/>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>ຖ້າເລື້ອກ, ເເຜນທີ່ລະດັບຊັ້ນເເມ່ນປະສົມປະສານ ຕໍ່ກັບຄໍາຊັ່ງເເຜນທີ່ການເດີນອາກາດ ເເລະ ເຫດການອື່ນ ຯ. ຖ້າບໍ່ເລື້ອກ, ຈະບໍ່ມີການ ປະສົມປະສານ. ອັນນີ້ເເມ່ນທາງເລື້ອກເພື່ອເພີ້ມໂຕເລກຂອງລະດັບຊັ້ນ ເເລະ ເຮັດເປັນເຄືອງຫມາຍກ່ອນຈະ ປະສົມປະສານ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1222"/>
        <source>Toggle map rendering</source>
        <translation>ຕີກເເຜນທີ່ປະສົມປະສານ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1245"/>
        <source>This icon shows whether on the fly projection is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation>ຮູບນີ້ສະເເດງໃຫ້ເຫັນວ່າ ຄາດຫມາຍການບິນທໍາງານ ຫລື ບໍ່. ຕິກໃສ່ ຮູບເພື່ອເຂົ້າໄປຫາ ໂຄງການ ຄູນລັກສະນະ ເພື່ອຈະເເປງພຶດຕິກໍາຂອງມັນ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1247"/>
        <source>Projection status - Click to open projection dialog</source>
        <translation type="unfinished">ສະຖານະຂອງຫນ້າວຽກ-ກົດເມົາສ໌ເພື່ອເປີດກອງບັນຍາຍຫນ້າວຽກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Map Composer for creating map layouts</source>
        <translation type="obsolete">ຕົວສ້າງແຜນທີ່ສຳລັບສ້າງຜັງແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2027"/>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>ເປີດ OGR ທີ່ໃຊ້ເຂົ້າກັນກັບລະດັບຊັ້ນຕົວເລກບອກຄະຫນາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2864"/>
        <source>QGIS Project Read Error</source>
        <translation type="unfinished">ມີການຜິດພາດໃນການອ່ານ QGIS </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2866"/>
        <source>Try to find missing layers?</source>
        <translation>ພະຍາຍາມຊ້ອກຫາລະດັບຊັ້ນທີ່ຂາດໄປ?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5092"/>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>ເປີດ GDAL ທີ່ໃຊ້ເຂົ້າກັນກັບແຫຼ່ງຂໍ້ມູນ Raster </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2649"/>
        <source>Save As</source>
        <translation>ດເກັບເປັນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2749"/>
        <source>Choose a QGIS project file to open</source>
        <translation>ເລືອກແຟ້ມ QGIS ເພື່ອນຳໃຊ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>QGIS Browser Selection</source>
        <translation type="obsolete">ການເລືອກອ່ານແຟ້ມ QGIS </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Enter the name of a web browser to use (eg. konqueror).
</source>
        <translation type="obsolete">ໃສ່ຊື່ໂປຼກຼາມຫຼີ້ນອີນເຕີເນດເພື່ອຫຼີ້ນ（ເຊັ່ນວ່າ konqueror)。
</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Enter the full path if the browser is not in your PATH.
</source>
        <translation type="obsolete">ໃສ່ເຕັມຊື່ຖ້າຫາກວ່າໂປຼກຼາມຫລີ້ນອີນເຕີເນດບໍ່ຢູ່ໃນເສັ້ນທາງ。
</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3038"/>
        <source>Saved project to:</source>
        <translation>ຫນ້າວຽກທີ່ຖືກເກັບໄປຍັງ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="318"/>
        <source>Reading settings</source>
        <translation>ອ່ານການຕັ້ງຄ່າ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="322"/>
        <source>Setting up the GUI</source>
        <translation>ການຕັ້ງຄ່າ  ຫນ້າຕາ GUI</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="312"/>
        <source>Checking database</source>
        <translation type="unfinished">ການກວດຖານຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="405"/>
        <source>Restoring loaded plugins</source>
        <translation>ການຟື້ນຟູໂປຼກຼາມເສີມ (Plugin) ໃຫ້ກັບຄືນສູ່ສະພາບເກົ່າ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="409"/>
        <source>Initializing file filters</source>
        <translation>ກຳລັງກວດການ່ນກອງແຟ້ມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="434"/>
        <source>Restoring window state</source>
        <translation>ກຳລັງຟື້ນຟູສະຖານະຂອງວິນໂດ້ (Window)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="438"/>
        <source>QGIS Ready!</source>
        <translation>ໂປຼກຼາມ QGIS ພ້ອມໃຊ້ງານ!</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="548"/>
        <source>&amp;New Project</source>
        <translation>ເ&amp;ປີດຫນ້າວຽກໃຫມ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+N</source>
        <comment>















































New Project</comment>
        <translation type="obsolete">Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="550"/>
        <source>New Project</source>
        <translation type="unfinished">ຫນ້າວຽກໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="553"/>
        <source>&amp;Open Project...</source>
        <translation>ເ&amp;ປີດຫນ້າວຽກມາໃຊ້ງານ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+O</source>
        <comment>















































Open a Project</comment>
        <translation type="obsolete">Ctrl+O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="555"/>
        <source>Open a Project</source>
        <translation type="unfinished">ເປີດຫນ້າວຽກໃດ້ຫນື່ງມາໃຊ້ງານ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="558"/>
        <source>&amp;Save Project</source>
        <translation>ເ&amp;ກັບວຽກໂດຍທັບຊື່ເກົ່າ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+S</source>
        <comment>















































Save Project</comment>
        <translation type="obsolete">Ctrl＋S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="560"/>
        <source>Save Project</source>
        <translation>ເກັບວຽກໂດຍທັບຊື່ເກົ່າ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="563"/>
        <source>Save Project &amp;As...</source>
        <translation>&amp;ເກັບວຽກເປັນ...</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+A</source>
        <comment>








Save Project under a new name
</comment>
        <translation type="obsolete">Ctrl+A</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="565"/>
        <source>Save Project under a new name</source>
        <translation type="unfinished">ຈັດເກັບຫນ້າວຽກພາຍໃຕ້ຊື່ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="568"/>
        <source>&amp;Print...</source>
        <translation>&amp;ພີມ...</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+P</source>
        <comment>















































Print</comment>
        <translation type="obsolete">Ctrl+P</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="570"/>
        <source>Print</source>
        <translation type="unfinished">ພີມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="573"/>
        <source>Save as Image...</source>
        <translation>ເກັບໃຫ້ເປັນແຟ້ມຮູບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+I</source>
        <comment>





















Save map as image
</comment>
        <translation type="obsolete">Ctrl+l</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="575"/>
        <source>Save map as image</source>
        <translation>ເກັບແຜນທີ່ເປັນຮູບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Export to MapServer Map...</source>
        <translation type="obsolete">ນຳສົ່ງແຜນທີ່ໄປຫາແຜນທີ່ຢູ່ຫນ່ວຍເຄືອຄ່າຍແມ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>M</source>
        <comment>





















Export as MapServer .map file
</comment>
        <translation type="obsolete">M</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Export as MapServer .map file</source>
        <translation type="obsolete">ນຳສົ່ງຮູບເປັນແຟ້ມ (.map) ແຜນທີ່ອອກໄປຫາແຜນທີ່ຢູ່ຫນ່ວຍເຄືອຄ່າຍແມ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="586"/>
        <source>Exit</source>
        <translation>ອອກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+Q</source>
        <comment>





















Exit QGIS
</comment>
        <translation type="obsolete">Ctrl+Q</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="588"/>
        <source>Exit QGIS</source>
        <translation>ອອກຈາກໂປຼກຼາມ QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="593"/>
        <source>Add a Vector Layer...</source>
        <translation type="unfinished">ເພີ້ມລະດັບຊັ້ນທີ່ມີຕົວເລກບອກຄະຫນາດແລະທິດທາງໃດຫນື່ງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>V</source>
        <comment>















































Add a Vector Layer</comment>
        <translation type="obsolete">V</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="595"/>
        <source>Add a Vector Layer</source>
        <translation type="unfinished">ເພີ້ມລະດັບຊັ້ນທີ່ມີຕົວເລກບອກຄະຫນາດແລະທິດທາງໃດຫນື່ງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="598"/>
        <source>Add a Raster Layer...</source>
        <translation>ເພີ້ມລະດັບຊັ້ນຂອງ Raster</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>R</source>
        <comment>















































Add a Raster Layer</comment>
        <translation type="obsolete">R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="600"/>
        <source>Add a Raster Layer</source>
        <translation>ເພີ້ມລະດັບຊັ້ນຂອງ Raster</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="603"/>
        <source>Add a PostGIS Layer...</source>
        <translation>ເພີ້ມລະດັບຊັ້ນຂອງໂປຼກຼາມ PostGIS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>D</source>
        <comment>















































Add a PostGIS Layer</comment>
        <translation type="obsolete">D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="605"/>
        <source>Add a PostGIS Layer</source>
        <translation>ເພີ້ມລະດັບຊັ້ນຂອງໂປຼກຼາມ PostGIS </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="615"/>
        <source>New Vector Layer...</source>
        <translation type="unfinished">ສາ້ງລະດັບຊັ້ນຕົວເລກທີບອກຄະຫນາດແລະທິດທາງໃຫມ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>N</source>
        <comment>















































Create a New Vector Layer</comment>
        <translation type="obsolete">N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="617"/>
        <source>Create a New Vector Layer</source>
        <translation type="unfinished">ສາ້ງລະດັບຊັ້ນຕົວເລກທີບອກຄະຫນາດແລະທິດທາງໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="620"/>
        <source>Remove Layer</source>
        <translation type="unfinished">ເອົາລະດັບຊັ້ນອອກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+D</source>
        <comment>





















Remove a Layer
</comment>
        <translation type="obsolete">Ctrl+D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="622"/>
        <source>Remove a Layer</source>
        <translation type="unfinished">ເອົາລະດັບຊັ້ນໃດຫນື່ງອອກ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="626"/>
        <source>Add All To Overview</source>
        <translation type="unfinished">ເພີ້ມທັງຫມົດໄປໃສ່ພາບໂດຍສະຫລູບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>+</source>
        <comment>















































Show all layers in the overview map</comment>
        <translation type="obsolete">+</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="628"/>
        <source>Show all layers in the overview map</source>
        <translation type="unfinished">ສະແດງລະດັບຊັ້ນທັງຫມົດຢູ່ໃນພາບແຜນທີ່ໂດຍສະຫລູບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="631"/>
        <source>Remove All From Overview</source>
        <translation type="unfinished">ເອົາທັງຫມົດອອກຈາກພາບແຜນທີ່ໂດຍສະຫລູບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>-</source>
        <comment>















































Remove all layers from overview map</comment>
        <translation type="obsolete">-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="633"/>
        <source>Remove all layers from overview map</source>
        <translation type="unfinished">ເອົາທັງຫມົດອອກຈາກພາບແຜນທີ່ໂດຍສະຫລູບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="642"/>
        <source>Show All Layers</source>
        <translation type="unfinished">ສະແດງລະດັບຊັ້ນທັງຫມົດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>S</source>
        <comment>















































Show all layers</comment>
        <translation type="obsolete">S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="644"/>
        <source>Show all layers</source>
        <translation type="unfinished">ສະແດງລະດັບຊັ້ນທັງຫມົດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="647"/>
        <source>Hide All Layers</source>
        <translation type="unfinished">ເຊື່ອງລະດັບຊັ້ນທັງຫມົດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>H</source>
        <comment>















































Hide all layers</comment>
        <translation type="obsolete">H</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="649"/>
        <source>Hide all layers</source>
        <translation type="unfinished">ເຊື່ອງລະດັບຊັ້ນທັງຫມົດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="654"/>
        <source>Project Properties...</source>
        <translation>ຄຸນລັກສະນະຂອງໜ້າວຽກອງພາບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="655"/>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="656"/>
        <source>Set project properties</source>
        <translation type="unfinished">ຕັ້ງຄຸນລັກສະນະຕ່າງໆຂອງຫນ້າວຽກ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="659"/>
        <source>Options...</source>
        <translation>ທາງເລື້ອກ...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="661"/>
        <source>Change various QGIS options</source>
        <translation>ປ່ຽນທາງເລືອກຂອງໂປຼກຼາມ QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="664"/>
        <source>Custom Projection...</source>
        <translation type="unfinished">ແກ້ໄຂຫນ້າວຽກຕາມຄວາມຕ້ອງການຂອງຕົນເອງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="666"/>
        <source>Manage custom projections</source>
        <translation type="unfinished">ຈັດການຫນ້າວຽກຕາມຄວາມຕ້ອງການຂອງຕົນເອງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="671"/>
        <source>Help Contents</source>
        <translation type="unfinished">ສາລະບານເພື່ອໃນການຫຼີ້ນໂປຼກຼາມ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>F1</source>
        <comment>















































Help Documentation</comment>
        <translation type="obsolete">F1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="677"/>
        <source>Help Documentation</source>
        <translation type="unfinished">ເອກະສານສະຫນັບສະຫນູນເພື່ອຊ່ວຍຫີ້ຼນໂປຼກຼາມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="680"/>
        <source>Qgis Home Page</source>
        <translation>ໂຮມເພດ (Home Page) ຂອງ Qgis</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+H</source>
        <comment>





















QGIS Home Page
</comment>
        <translation type="obsolete">Ctrl+H</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="684"/>
        <source>QGIS Home Page</source>
        <translation>ໂຮມເພດ (Home Page) ຂອງ Qgis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="687"/>
        <source>About</source>
        <translation type="unfinished">ກ່ຽວກັບໂປຼກຼາມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="688"/>
        <source>About QGIS</source>
        <translation>ກ່ຽວກັບໂປຼກຼາມ QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="691"/>
        <source>Check Qgis Version</source>
        <translation>ກວດສອບລຸ້ນ (Version) ຂອງ QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="692"/>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>ກວດເບີ່ງວ່າລຸ້ນ (Version) ຂອງ QGIS ຕົກລຸ້ນຫຼືບໍ່ (ຕ້ອງການໃຊ້ອີນເຕີເນດ)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="697"/>
        <source>Refresh</source>
        <translation type="unfinished">ກະຕຸ້ນໃຫມ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+R</source>
        <comment>








Refresh Map
</comment>
        <translation type="obsolete">Ctrl+R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="699"/>
        <source>Refresh Map</source>
        <translation type="unfinished">ກະຕຸ້ນແຜນທີ່ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="704"/>
        <source>Zoom In</source>
        <translation>ດື່ງພາບເຂົ້າໃກ້ (Zoom In)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl++</source>
        <comment>








Zoom In
</comment>
        <translation type="obsolete">Ctrl++</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="709"/>
        <source>Zoom Out</source>
        <translation>ຍັບພາບອອກໃກ້ (Zoom out)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+-</source>
        <comment>





















Zoom Out
</comment>
        <translation type="obsolete">Ctrl+-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="712"/>
        <source>Zoom Full</source>
        <translation>ຊຸມ (Zoom Full) ເຕັມຫນ້າຈໍ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>F</source>
        <comment>





















Zoom to Full Extents
</comment>
        <translation type="obsolete">F</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="714"/>
        <source>Zoom to Full Extents</source>
        <translation>ຊຸມ (Zoom) ໄປຮູບເຕັມຂອງແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="717"/>
        <source>Zoom To Selection</source>
        <translation>ຊຸມໃສ່ຈຸດທີ່ເລື້ອກໄວ້ແລ້ວ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+F</source>
        <comment>





















Zoom to selection
</comment>
        <translation type="obsolete">ctrl+F</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="719"/>
        <source>Zoom to selection</source>
        <translation>ຊຸມ (Zoom) ໃສ່ຈຸດທີ່ເລື້ອກໄວ້ແລ້ວ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="722"/>
        <source>Pan Map</source>
        <translation>ຕັດເອົາພາບແຜນທີ່ໆຕ້ອງການເບີ່ງສະເພາະ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="723"/>
        <source>Pan the map</source>
        <translation type="unfinished">ຕັດເອົາພາບແຜ່ນທີ່ໆຕ້ອງການເບີ່ງສະເພາະ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="726"/>
        <source>Zoom Last</source>
        <translation>ຊຸມ (Zoom) ໄປຫາຈຸດຊຸມຄັ້ງລ້າສຸດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="728"/>
        <source>Zoom to Last Extent</source>
        <translation>ຊຸມ (Zoom) ໄປຫາຂອບເຂດຄັ້ງສຸດທ້າຍ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="731"/>
        <source>Zoom To Layer</source>
        <translation>ຊຸມ (Zoom) ໄປຫາລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="733"/>
        <source>Zoom to Layer</source>
        <translation>ຊຸມ (Zoom) ໄປຫາລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="736"/>
        <source>Identify Features</source>
        <translation>ລະບຸລາຍລະອຽດຈຸດເດັ່ນ (Features)</translation>
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
        <translation>ກົດໃສ່ຈຸດເດັ່ນ (Features) ເພື່ອລະບຸລາຍລະອຽດຂອງແຕ່ລະອັນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="742"/>
        <source>Select Features</source>
        <translation>ເລື້ອກຈຸດເດັ່ນ (Features)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="748"/>
        <source>Open Table</source>
        <translation type="unfinished">ເປີດການນຳໃຊ້ຕາຕະລາງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="752"/>
        <source>Measure Line </source>
        <translation type="unfinished">ວັດແທກເສັ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+M</source>
        <comment>















































Measure a Line</comment>
        <translation type="obsolete">Ctrl+M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="754"/>
        <source>Measure a Line</source>
        <translation type="unfinished">ວັດແທກແນວສາຍ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="757"/>
        <source>Measure Area</source>
        <translation type="unfinished">ວັດແທກພື້ນທີ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+J</source>
        <comment>








Measure an Area
</comment>
        <translation type="obsolete">Ctrl+J</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="759"/>
        <source>Measure an Area</source>
        <translation type="unfinished">ວັດແທກພື້ນທີ່ໃດຫນື່ງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="764"/>
        <source>Show Bookmarks</source>
        <translation>ສະແດງບຸກມາກ (Bookmark)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="763"/>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation>B</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="779"/>
        <source>New Bookmark...</source>
        <translation>ບຸກມາກ (Bookmark)ໃຫມ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+B</source>
        <comment>















































New Bookmark</comment>
        <translation type="obsolete">Ctrl+B</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5440"/>
        <source>New Bookmark</source>
        <translation>ບຸກມາກ (Bookmark) ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="784"/>
        <source>Add WMS Layer...</source>
        <translation>ເພີ້ມລະດັບຊັ້ນຂອງ WMS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>W</source>
        <comment>















































Add Web Mapping Server Layer</comment>
        <translation type="obsolete">W</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="786"/>
        <source>Add Web Mapping Server Layer</source>
        <translation type="unfinished">ເພີ້ມລະດັບຊັ້ນເວບໄຊ້ແຜ່ນທີ່ໃສ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="789"/>
        <source>In Overview</source>
        <translation type="unfinished">ພາບໂດຍສະຫຼຸບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="790"/>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="791"/>
        <source>Add current layer to overview map</source>
        <translation type="unfinished">ເພີ້ມລະດັບຊັ້ນປະຈຸບັນໃສ່ແຜນທີ່ໂດຍສະຫລຸບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="797"/>
        <source>Plugin Manager...</source>
        <translation>ຕົຍຈັດການໂປຼກຼາມເສີມ (plugin)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="799"/>
        <source>Open the plugin manager</source>
        <translation>ເປີດນຳໃຊ້ຕົວຈັດການໂປຼກຼາມເສີມ (Plugin)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="817"/>
        <source>Capture Point</source>
        <translation type="unfinished">ຈັບຈຸດ</translation>
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
        <translation type="unfinished">ຈັບຈຸດຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="823"/>
        <source>Capture Line</source>
        <translation type="unfinished">ຈັບເສັ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>/</source>
        <comment>


















Capture Lines
</comment>
        <translation type="obsolete">ດ/</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="825"/>
        <source>Capture Lines</source>
        <translation type="unfinished">ຈັບເສັ້ນຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="829"/>
        <source>Capture Polygon</source>
        <translation type="unfinished">ຈັບຮູບຫຼາຍຫຼ່ຽມ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+/</source>
        <comment>


















Capture Polygons
</comment>
        <translation type="obsolete">Ctrl+/</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="831"/>
        <source>Capture Polygons</source>
        <translation type="unfinished">ຈັບຮູບຫຼາຍຫຼ່ຽມຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="836"/>
        <source>Delete Selected</source>
        <translation type="unfinished">ລືບໂຕຖືກເລືອກ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="851"/>
        <source>Add Vertex</source>
        <translation type="unfinished">ເພີ້ມຈຸດລວມຂອງເສັ້ນຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="856"/>
        <source>Delete Vertex</source>
        <translation type="unfinished">ລືບຈຸດລວມຂອງເສັ້ນຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="861"/>
        <source>Move Vertex</source>
        <translation type="unfinished">ຍ້າຍຈຸດລວມຂອງເສັ້ນຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="954"/>
        <source>&amp;File</source>
        <translation>ແ&amp;ຟ້ມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="957"/>
        <source>&amp;Open Recent Projects</source>
        <translation>&amp;ເປີດຫນ້າວຽກທີ່ກຳລັງໃຊ້ຢູ່ປະຈຸບັນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="977"/>
        <source>&amp;View</source>
        <translation>ເ&amp;ບີ່ງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="996"/>
        <source>&amp;Layer</source>
        <translation>&amp;ລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1016"/>
        <source>&amp;Settings</source>
        <translation>&amp;ການຕັ້ງຄ່າ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1036"/>
        <source>&amp;Help</source>
        <translation>&amp;ຊ່ວຍເຫລືອ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1055"/>
        <source>File</source>
        <translation type="unfinished">ແຟ້ມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1072"/>
        <source>Manage Layers</source>
        <translation type="unfinished">ຈັດການລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1133"/>
        <source>Help</source>
        <translation type="unfinished">ຊ່ວຍເຫຼືອ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1082"/>
        <source>Digitizing</source>
        <translation type="unfinished">ການປ້ອນຂໍ້ມູນເປັນຕົວເລກ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1102"/>
        <source>Map Navigation</source>
        <translation type="unfinished">ການນຳລ້ອງແຜ່ນທີ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1115"/>
        <source>Attributes</source>
        <translation>ຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1128"/>
        <source>Plugins</source>
        <translation>ໂປຼກຼາມເສີມ (Plugins)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1251"/>
        <source>Ready</source>
        <translation type="unfinished">ພ້ອມໃຊ້ງານ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1627"/>
        <source>New features</source>
        <translation type="unfinished">ເປີດຈຸດເດັ່ນຂອງຫນ້າວຽກໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2882"/>
        <source>Unable to open project</source>
        <translation type="unfinished">ບໍ່ສາມາດເປີດຫນ້າວຽກ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3052"/>
        <source>Unable to save project </source>
        <translation type="unfinished">ບໍ່ສາມາດຈັດເກັບຫນ້າວຽກທີ່ສ້າງໄວ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2994"/>
        <source>Choose a filename to save the QGIS project file as</source>
        <translation>ຈົ່ງເລືຶອກຊື່ເພື່ອຈັດເກັບແຟ້ມ QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3110"/>
        <source>QGIS: Unable to load project</source>
        <translation type="unfinished">QGIS ບໍ່ສາມາດໂຫຼດຫນ້າວຽກທີ່ເຮັດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3111"/>
        <source>Unable to load project </source>
        <translation type="unfinished">ບໍ່ສາມາດໂຫຼດຫນ້າວຽກທີ່ສ້າງໄວ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4276"/>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS ປ່ຽນຢູ່ໃນ SVN ຕັ້ງແຕ່ອອກໂປູກຼາມນີ້ຄັ້ງລ້າສຸດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>You can change this option later by selecting Options from the Settings menu (Help Browser tab).</source>
        <translation type="obsolete">ສາມາດປ່ຽນອອບຊັ້ນ (Option) ນີ້ໄດ້ພາຍຫຼັງໂດຍກົດໄປທີ່Options ຈາກເມນູຕັ້ງຄ່າ Settings (ແທບເພື່ອເລື້ອກອ່ານຄວາມຊ່ວຍເຫຼືອ)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5264"/>
        <source>Layer is not valid</source>
        <translation type="unfinished">ລະດັບຊັ້ນໃຊ້ການບໍ່ໄດ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5265"/>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>ລະບໍ່ແມ່ນແຫຼ່ງຂໍ້ມູນທີ່ໃຊ້ເຂົ້າກັບ Rasterດັບຊັ້ນຕົວນີ້ໃຊ້ການບໍ່ໄດ້ແລະບໍ່ສາມາດເພີ້ມຕື່ມໃສ່ແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4533"/>
        <source>Save?</source>
        <translation>ດເກັບບໍ່?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5047"/>
        <source>Clipboard contents set to: </source>
        <translation>ຊິ່ງທີ່ຢູ່ໃນໜ່ວຍຄວາມຈຳຖືກຕັ້ງໄປຫາ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5183"/>
        <source> is not a valid or recognized raster data source</source>
        <translation>ແຫຼ່ງຂໍ້ມູນທີ່ໃຊ້ເຂົ້າກັບ Raster ບໍ່ໄດ້ຮັບການຈື່ ຫູື ໃຊ້ການບໍ່ໄດ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5344"/>
        <source> is not a supported raster data source</source>
        <translation>ບໍ່ແມ່ນແຫຼ່ງຂໍ້ມູນທີ່ໃຊ້ເຂົ້າກັບ Raster</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5345"/>
        <source>Unsupported Data Source</source>
        <translation type="unfinished">ແຫຼ່ງຂໍ້ມູນທີ່ບໍ່ເຂົ້າກັບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5441"/>
        <source>Enter a name for the new bookmark:</source>
        <translation>ໃສ່ຊື່ໃຫ້ບຸກມາກ (Bookmark) ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5458"/>
        <source>Error</source>
        <translation>ໍ້ຜິດພາດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5458"/>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>ບໍ່ສາມາດສ້າງບຸກມາກ (Bookmark),ຖານຂໍ້ມູນຂອງຜູ້ໃຊ້ອາດບໍ່ມີຫຼືເພ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+?</source>
        <comment>








Help Documentation (Mac)
</comment>
        <translation type="obsolete">Ctrl+?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="769"/>
        <source>Show most toolbars</source>
        <translation type="unfinished">ສະແດງເຄື່ອງມືໃຊ້ງານທັງຫມົດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="775"/>
        <source>Hide most toolbars</source>
        <translation type="unfinished">ເຊື່ອງເຄື່ອງມືໃຊ້ງານທັງຫມົດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="875"/>
        <source>Cut Features</source>
        <translation type="unfinished">ຕັດຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="876"/>
        <source>Cut selected features</source>
        <translation type="unfinished">ຕັດຈຸດເດັ່ນທີ່ເລື້ອກໄວ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="880"/>
        <source>Copy Features</source>
        <translation type="unfinished">ສຳເນົາຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="881"/>
        <source>Copy selected features</source>
        <translation type="unfinished">ສຳເນົາຈຸດເດັ່ນທີ່ເລື້ອກໄວ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="885"/>
        <source>Paste Features</source>
        <translation>ນຳເອົາຈຸດເດັ່ນອອກມານຳໃຊ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="886"/>
        <source>Paste selected features</source>
        <translation type="unfinished">ນຳເອົາສຳເນົາຂອງຈຸດເດັ່ນທີ່ຖືກເລື້ອກໄວ້ອອກມາໃຊ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>
Compiled against Qt </source>
        <translation type="obsolete">ຮຽບຮຽງກັບ Qt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>, running against Qt </source>
        <translation type="obsolete">, ບໍ່ທໍາງານກັບ Qt </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Many user interface improvements</source>
        <translation type="obsolete">ປັບປຸງຕົວປະສານລະຫວ່າງຜູ້ໃຊ້ແລະຄອມພີວເຕີ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Improved vector and attribute editing</source>
        <translation type="obsolete">ແກ້ໄຂຄຸນລັກສະນະແລະຕົວເລກທີ່ບອກຈຳນວນຄະຫນາດແລະທິດທາງທີ່ໄດ້ຮັບການປັບປຸງແລ້ວ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>WMS support</source>
        <translation type="obsolete">ເຂົ້າກັນກັບ WMS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Improved measure tools with area measuring</source>
        <translation type="obsolete">ເຄື່ອງມືວັດແທກໄດ້ຮັບການປັບປຸງດ້ວຍການວັດແທກພື້ນທີ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Attribute searching</source>
        <translation type="obsolete">ການຊອກຫາຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>New legend structure</source>
        <translation type="obsolete">ໂຄງສ້າງຄຳອະທິບາຍໃຫມ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Refactoring of API to allow the use of QGIS libraries in mapping applications</source>
        <translation type="obsolete">ໃຊ້ API ເພື່ອໃຫ້ ນໍາໃຊ້ QGIS libraries ສະເພາະກັບໂປຣກຣາມເເຜນທີ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Improved MapServer export tool</source>
        <translation type="obsolete">ເຄື່ອງມືສົ່ງອອກຂອງຫນ່ວຍເຄືອຄາຍແມ່ໄດ້ຮັບການປັບປຸງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Map canvas background color is customizable</source>
        <translation type="obsolete">ສີພື້ນແຜນທີ່ສາມາດດັດແກ້ໃຫ້ຕົງຄວາມຕ້ອງການຂອງເຈົ້າຂອງໄດ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Vector layer transparency</source>
        <translation type="obsolete">ຄວາມໂປ່ງໃສຂອງລະດັບຊັ້ນທີ່ເປັນຕົວເລກບອກຄະໜາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>GRASS support in all platforms</source>
        <translation type="obsolete">GRASS ທີ່ເຂົ້າກັນກັບລະບົບຄອມພີວເຕີ້ທັງໝົດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Enhanced GRASS support and toolbox commands</source>
        <translation type="obsolete">ຍົກລະດັບຄຳສັ່ງຂອງ Toolbox ແລະການເຮັດໃຫ້ເຂົ້າກັນກັບ GRASS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Enhanced vector editing, including copy/paste, snapping and vertex editing</source>
        <translation type="obsolete">ຍົກລະດັບການແກ້ໄຂຕົວເລກທີ່ບອກຄະຫນາດແລະທິດທາງ,ລວມທັງການເຮັດສຳເນົາ/ການນຳເອົາສຳເນົາອອກມາໃຊ້ (Paste),ການຖ່າຍຮູບແລະການແກ້ໄຂຈຸດລວມຂອງບັນດາເສັ້ນຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Shapefile/OGR layer editing</source>
        <translation type="obsolete">ການແກ້ໄຂລະດັບຊັ້ນຂອງ Shapefile/OGR</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Option to only look in the geometry_columns table when searching for PostGIS layers</source>
        <translation type="obsolete">ຂໍ້ເລື້ອກເພື່ອເບິ່ງພາຍໃນຕາຕະລາງອງຖັນທາງຄະນິດສາດໃນເວລາຄົ້ນຫາລະດັບຊັ້ນຂອງ PostGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4308"/>
        <source>Network error while communicating with server</source>
        <translation>ມີການຜິດພາດເກີດຂື້ນກັບເຄືອຄ່າຍໃນຄະນະທີສື່ສານກັບຫນ່ວຍເຄືອຄ່າຍແມ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4311"/>
        <source>Unknown network socket error</source>
        <translation>ບໍ່ຮູ້ຂໍ້ຜິດພາດເກີດຂື້ນກັບຮູ້ສຽບເຄ່ືອຄ່າຍ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4316"/>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>ບໍ່ສາມາດຕິດຕໍ່ກັບ QGIS ລຸ້ນໜ່ວຍແມ່ເຄື່ອຄ່າຍ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="768"/>
        <source>T</source>
        <comment>Show most toolbars</comment>
        <translation>T</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ctrl+T</source>
        <comment>















































Hide most toolbars</comment>
        <translation type="obsolete">Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="358"/>
        <source>Checking provider plugins</source>
        <translation>ກວດສອບຜູ້ໃຫ້ສະໜອງໂປຼກຼາມເສີມ (Plugins)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="362"/>
        <source>Starting Python</source>
        <translation>ເລີ້ມ Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="396"/>
        <source>Python console</source>
        <translation>ແຜງຄວບຄຸມ (ຈໍ) Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1762"/>
        <source>Python error</source>
        <translation>ມີຂໍ້ຜິດພາດກັບ Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1762"/>
        <source>Error when reading metadata of plugin </source>
        <translation>ມີຂໍ້ຜິດພາດເກີດຂື້ນເມື່ອອ່ານ Metadata ຂອງໂປຼກຼາມເສີມ (Plugin) </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3502"/>
        <source>Provider does not support deletion</source>
        <translation type="unfinished">ຜູ້ສະຫນອງບໍ່ເຂົ້າກັນກັບການລືບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3503"/>
        <source>Data provider does not support deleting features</source>
        <translation type="unfinished">ຜູ້ສະຫນອງຂໍ້ມູນບໍ່ເຂົ້າກັນກັບການລືບຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3509"/>
        <source>Layer not editable</source>
        <translation type="unfinished">ລະດັບຊັ້ນບໍ່ສາມາດແກ້ໄຂ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3510"/>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation>ລະດັບຊັ້ນປະຈຸບັນບໍ່ສາມາດແກ້ໄຂໄດ້,ເລື້ອກ &apos;ເລີ້ມ ເເກ້ໄຂ&apos; ຢູ່ໃນທ່ອນຄຳສັ່ງ (Toolbar) ທີ່ເປັນດິຈີຕອນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="810"/>
        <source>Toggle editing</source>
        <translation>ຕີກ ເເກ້ໄຂ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="811"/>
        <source>Toggles the editing state of the current layer</source>
        <translation>ຕີກ ເເກ້ໄຂລະດັບຊັ້ນປັດຈຸບັນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="866"/>
        <source>Add Ring</source>
        <translation>ເພີ້ມວົງແວນ (Ring)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="870"/>
        <source>Add Island</source>
        <translation>ເພີ້ມເກາະ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="871"/>
        <source>Add Island to multipolygon</source>
        <translation type="unfinished">ເພີ້ມເກາະໃສ່ຮູບຫຼາຍຫຼ່ຽມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1145"/>
        <source>Toolbar Visibility...</source>
        <translation>ທ່ອນຄຳສັ່ງເບີ່ງເຫັນ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1175"/>
        <source>Scale </source>
        <translation type="unfinished">ມາຕາສ່ວນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1190"/>
        <source>Current map scale (formatted as x:y)</source>
        <translation>ມາດຕາສ່ວນແຜນທີ່ປະຈຸບັນ (ຖືກຈັດໃຫ້ເປັນ x:y)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1204"/>
        <source>Map coordinates at mouse cursor position</source>
        <translation>ແຜນທີ່ໆປະສານກັບຕຳແໜ່ງຂອງເມັດເຄີເຊີຂອງເມົ້າສ໌ (Mouse Cursor)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3780"/>
        <source>Invalid scale</source>
        <translation type="unfinished">ມາດຕາສ່ວນໃຊ້ການບໍ່ໄດ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4534"/>
        <source>Do you want to save the current project?</source>
        <translation type="unfinished">ທ່ານຕ້ອງການຈັດເກັບໜ້າວຽກປະຈຸບັນນີ້ບໍ່?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="549"/>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation type="unfinished">Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="554"/>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation type="unfinished">Ctrl+O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="559"/>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation type="unfinished">Ctrl＋S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="564"/>
        <source>Ctrl+A</source>
        <comment>Save Project under a new name</comment>
        <translation type="unfinished">Ctrl+A</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="569"/>
        <source>Ctrl+P</source>
        <comment>Print</comment>
        <translation type="unfinished">Ctrl+P</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="574"/>
        <source>Ctrl+I</source>
        <comment>Save map as image</comment>
        <translation type="unfinished">Ctrl+l</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="587"/>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation type="unfinished">Ctrl+Q</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="594"/>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation type="unfinished">V</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="599"/>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation type="unfinished">R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="604"/>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation type="unfinished">D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="616"/>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation type="unfinished">N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="621"/>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation type="unfinished">Ctrl+D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="627"/>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation type="unfinished">+</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="632"/>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation type="unfinished">-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="636"/>
        <source>Toggle full screen mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="637"/>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="638"/>
        <source>Toggle fullscreen mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="643"/>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation type="unfinished">S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="648"/>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation type="unfinished">H</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="673"/>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation type="unfinished">Ctrl+?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="675"/>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation type="unfinished">F1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="682"/>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation type="unfinished">Ctrl+H</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="698"/>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation type="unfinished">Ctrl+R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="703"/>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation type="unfinished">Ctrl++</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="708"/>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation type="unfinished">Ctrl+-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="713"/>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation type="unfinished">F</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="718"/>
        <source>Ctrl+F</source>
        <comment>Zoom to selection</comment>
        <translation type="unfinished">ctrl+F</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="753"/>
        <source>Ctrl+M</source>
        <comment>Measure a Line</comment>
        <translation type="unfinished">Ctrl+M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="758"/>
        <source>Ctrl+J</source>
        <comment>Measure an Area</comment>
        <translation type="unfinished">Ctrl+J</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="774"/>
        <source>Ctrl+T</source>
        <comment>Hide most toolbars</comment>
        <translation type="unfinished">Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="780"/>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation type="unfinished">Ctrl+B</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="785"/>
        <source>W</source>
        <comment>Add Web Mapping Server Layer</comment>
        <translation type="unfinished">W</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="824"/>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation type="unfinished">ດ/</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="830"/>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation type="unfinished">Ctrl+/</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="841"/>
        <source>Move Feature</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="846"/>
        <source>Split Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="891"/>
        <source>Map Tips</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="892"/>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1176"/>
        <source>Current map scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1203"/>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1633"/>
        <source>Imrovements to digitising capabilities.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1639"/>
        <source>Supporting default and defined styles (.qml) files for file based vector layers. With styles you can save the symbolisation and other settings associated with a vector layer and they will be loaded whenever you load that layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1645"/>
        <source>Improved support for transparency and contrast stretching in raster layers. Support for color ramps in raster layers. Support for non-north up rasters. Many other raster improvements &apos;under the hood&apos;.</source>
        <translation type="unfinished"></translation>
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
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5493"/>
        <source>Project file is older</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5495"/>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5497"/>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5500"/>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5502"/>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5504"/>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5506"/>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5506"/>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5510"/>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5511"/>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation>ວີນໂດ້ຫຼັກ (MainWindow)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="102"/>
        <source>Legend</source>
        <translation type="unfinished">ຄຳອະທິບາຍຫຼືເຄື່ອງໝາຍຂອງແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="135"/>
        <source>Map View</source>
        <translation type="unfinished">ຮູບແຜນທີ່</translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="13"/>
        <source>About Quantum GIS</source>
        <translation>ກ່ຽວກັບ Quantum GIS </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="230"/>
        <source>Ok</source>
        <translation type="unfinished">ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="26"/>
        <source>About</source>
        <translation type="unfinished">ກ່ຽວກັບໂປຼກຼາມ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="65"/>
        <source>Version</source>
        <translation>ລຸ້ນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="152"/>
        <source>What&apos;s New</source>
        <translation>ມີຫຍັງໃໝ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</source>
        <translation type="obsolete">&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="129"/>
        <source>QGIS Home Page</source>
        <translation>ໂຮມເພດ (Home Page) ຂອງ Qgis</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Subscribe to the QGIS-User mailing list</source>
        <translation type="obsolete">ລົງນາມເປັນສະມາຊິກຂອງ QGIS ໄດ້ທີ່ລາຍຊື່ທາງໄປສະນີ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="189"/>
        <source>Providers</source>
        <translation type="unfinished">ຜູ້ສະຫນອງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="175"/>
        <source>Developers</source>
        <translation type="unfinished">ຜູ້ພັດທະນາ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;h2&gt;QGIS Developers&lt;/h2&gt;</source>
        <translation type="obsolete">&lt;h2&gt; ຜູ້ພັດທະນາ QGIS&lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>QGIS Browser Selection</source>
        <translation type="obsolete">ການເລື້ອກໃຊ້ໂປຼກຼາມ QGIS Browser</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Enter the name of a web browser to use (eg. konqueror).
Enter the full path if the browser is not in your PATH.
You can change this option later by selection Options from the Settings menu (Help Browser tab).</source>
        <translation type="obsolete">ໃສ່ຊື່ໂປຼກຼາມ Web browser ເພື່ອນຳໃຊ້ (ເຊັ່ນວ່າ: konqueror).ໃສ່ເຕັມຊື່ຂອງເສັ້ນທາງໄປຫາໂປຼກຼາມ Web browser ຖ້າຫາກວ່າມັນບໍ່ຢູ່ໃນເສັ້ນທາງຂອງມັນ.ທ່ານສາມາດປ່ຽນຄຳສັ່ງນີ້ພາຍຫຼັງໂດຍເລື້ອກ Option ຈາກເມນູ Settings(Help Br0wser Tab).ເລື້ອກເອົາ Selection.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>QGIS Sponsors</source>
        <translation type="obsolete">ຜູ້ໃຫ້ການສະຫນັບສະຫນຸນ QGIS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>The following have sponsored QGIS by contributing money to fund development and other project costs</source>
        <translation type="obsolete">ຊິ່ງທີ່ຕາມມານີ້ໄດ້ອຸປະຖຳ QGIS ໂດຍໃຫ້ເງິນເຂົ້າກອງທືນພັດທະນາແລະ ມູນຄ່າໃຊ້ຈ່າຍໃນໂຄງການອື່ນໆ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="114"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="115"/>
        <source>Website</source>
        <translation>ເວບໄຊ້ (Website)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Arial&apos;; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p align=&quot;center&quot; style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Quantum GIS is licensed under the GNU General Public License&lt;/p&gt;
&lt;p align=&quot;center&quot; style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;http://www.gnu.org/licenses&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Arial&apos;; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p align=&quot;center&quot; style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt; ທົວໄປ&lt;/p&gt;&lt;p align=&quot;center&quot; style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;http://www.gnu.org/licenses&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="199"/>
        <source>Sponsors</source>
        <translation type="unfinished">ຜູ້ໃຫ້ການສະຫນັບສະຫນຸນ</translation>
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
        <location filename="../src/ui/qgsabout.ui" line="91"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation type="unfinished">ໂປຼກຼາມ Quantum GIS ໄດ້ຮັບອະນຸຍາດພາຍໃຕ້ໃບອະນຸຍາດຂອງ (GNU General Public License)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="104"/>
        <source>http://www.gnu.org/licenses</source>
        <translation type="unfinished">ທີ່ຢູ່ອີນເຕີເນດ http://www.gnu.org/licenses/licenses.ja.html</translation>
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
        <translation type="unfinished">ເພີ້ມຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="22"/>
        <source>Name:</source>
        <translation type="unfinished">ຊື່:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="35"/>
        <source>Type:</source>
        <translation type="unfinished">ຊະນິດ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>OK</source>
        <translation type="obsolete">ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">ຍົກເລີກ</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Name</source>
        <translation type="obsolete">ຊື່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Action</source>
        <translation type="obsolete">ການປະຕິບັດງານ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Capture</source>
        <translation type="obsolete">ຈັບພາບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Select an action</source>
        <comment>












File dialog window title
</comment>
        <translation type="obsolete">ເລື້ອກການປະຕິບັດງານ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="150"/>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation type="unfinished">ເລື້ອກການປະຕິບັດງານ</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Form1</source>
        <translation type="obsolete">ແບບນື່ງ1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="214"/>
        <source>Move up</source>
        <translation type="unfinished">ເຄື່ອນຍ້າຍຂື້ນເທິງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="211"/>
        <source>Move the selected action up</source>
        <translation type="unfinished">ເຄື່ອນຍ້າຍການປະຕິບັດງານທີ່ໄດ້ຮັບການເລືອກໄວ້ຂື້ນເທິງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="224"/>
        <source>Move down</source>
        <translation type="unfinished">ເຄື່ອນຍ້າຍລົງລຸ່ມ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="221"/>
        <source>Move the selected action down</source>
        <translation type="unfinished">ເຄື່ອນຍ້າຍການປະຕິບັດງານທີ່ໄດ້ຮັບການເລືອກໄວ້ລົງລຸ່ມ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="250"/>
        <source>Remove</source>
        <translation type="unfinished">ເອົາອອກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="247"/>
        <source>Remove the selected action</source>
        <translation type="unfinished">ເອົາການປະຕິບັດງານທີ່ເລື້ອກໄວ້ແລ້ວອອກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name:</source>
        <translation type="obsolete">ຊື່:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="59"/>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>ກະລຸນາໃສ່ຊື່ຂອງການປະຕິບັດງານໃດນື່ງຢູ່ບ່ອນນີ້.ການໃສ່ຊື່ຄວນເປັນຊື່ນື່ງດຽວເທົ່ານັ້ນ (ໂປຼກຼາມ qgis ຈະໃສ່ຊື່ພຽງນື່ງດຽວເທົ່ານັ້ນຫາກຈຳເປັນ)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="56"/>
        <source>Enter the action name here</source>
        <translation type="unfinished">ຈົ່ງໃສ່ຊື່ການປະຕິບັດງານຢູ່ຫນີ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Action:</source>
        <translation type="obsolete">ການປະຕິບັດງານ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="79"/>
        <source>Enter the action command here</source>
        <translation type="unfinished">ໃສ່ຄຳສັ່ງຂອງການປະຕິບັດງານຢູ່ຫນີ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Browse</source>
        <translation type="obsolete">ການເບີ່ງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Browse for action commands</source>
        <translation type="obsolete">ການເລືອກຫາຄຳສັ່ງປະຕິບັດງານ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="178"/>
        <source>Insert action</source>
        <translation type="unfinished">ສອດໃສ່ການປະຕິບັດງານ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="175"/>
        <source>Inserts the action into the list above</source>
        <translation type="unfinished">ສອດໃສ່ການປະຕິບັດງານໃນລາຍການຂ້າງເທິງນີ້</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="188"/>
        <source>Update action</source>
        <translation type="unfinished">ຍົກລະດັບການປະຕິບັດງານ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="185"/>
        <source>Update the selected action</source>
        <translation type="unfinished">ຍົກລະດັບການປະຕິບັດງານທີເລືອກໄວ້ແລ້ວ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="115"/>
        <source>Insert field</source>
        <translation type="unfinished">ສອດໃສ່ສະຫນາມຫນ້າວຽກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="112"/>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation type="unfinished">ສອດໃສ່ສະຫນາມຫນ້າວຽກເຂົ້າໄປໃນພາກປະຕິບັດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="105"/>
        <source>The valid attribute names for this layer</source>
        <translation type="unfinished">ຊື່ທີ່ໃຊ້ການໄດ້ສຳລັບລະດັບຊັ້ນນີ້</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="144"/>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>ລາຍການນີ້ໄດ້ລວມເອົາການປະຕິບັດງານທັງຫມົດເຊິ່ງໄດ້ຮັບການກຳຫນົດຄວາມຫມາຍສຳລັບລະດັບຊັ້ນປະຈຸບັນ,ເພີ້ມລະດັບຊັ້ນໂດຍໃສ່ລາຍລະອຽດໃນແຜງຄວບຄຸມຂ້າງລຸ່ມແລ້ວກົດປຸ່ມ Insert action, ການປະຕິບັດງານສາມາດແກ້ໄຂໂດຍກົດສອງບາດໃສ່ພວກເຂົາ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation type="obsolete">ລົງມືເຮັດວຽກບ່ອນນີ້.ນຳໃຊ້ໂປຼກຼາມໃດກໍ່ໄດ້,ຕົວໜັງສືຫຼືຄຳສັ່ງທີ່ມີຢູ່ບົນລະບົບ.ເມື່ອລົງມືເຮັດວຽກໄປກະຕຸ້ນຊຸດຕົວຫນັງສືໃດນຶ່ງທີ່ເລີ້ມດ້ວຍ % ແລະ ມີຊື່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="128"/>
        <source>Capture output</source>
        <translation>ຈັບພາບຜົນອອກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="122"/>
        <source>Captures any output from the action</source>
        <translation type="unfinished">ຈັບພາບຂາອອກໃດນື່ງຈາກການປະຕິບັດງານ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="125"/>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation type="unfinished">ຈັບພາບຂາອອກຫຼືຂໍ້ຜິດພາດທີ່ກໍ່ຂື້ນໂດຍການປະຕິບັດງານແລະສະແດງໃຫ້ຢູ່ໃນກ່ອງຂໍ້ຄວາມ</translation>
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
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="157"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="82"/>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="162"/>
        <source>Action</source>
        <translation type="unfinished">ການປະຕິບັດງານ</translation>
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
        <translation type="unfinished">ຈັບພາບ</translation>
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
        <translation type="unfinished">ໃສ່ຄ່າຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>1</source>
        <translation type="obsolete">1</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Attribute</source>
        <translation type="obsolete">ຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Value</source>
        <translation type="obsolete">ຄ່າ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;OK</source>
        <translation type="obsolete">&amp;ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Cancel</source>
        <translation type="obsolete">&amp;ຍົກເລິກ</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="340"/>
        <source>Run action</source>
        <translation type="unfinished">ແລ່ນການປະຕິບັດງານ</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="13"/>
        <source>Attribute Table</source>
        <translation>ຕາຕະລາງຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="167"/>
        <source>Start editing</source>
        <translation type="unfinished">ເລີ້ມການແກ້ໄຂ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Close</source>
        <translation type="obsolete">&amp;ປິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Alt+C</source>
        <translation type="obsolete">Alt+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="144"/>
        <source>Ctrl+X</source>
        <translation type="unfinished">Ctrl+X</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="128"/>
        <source>Ctrl+N</source>
        <translation type="unfinished">Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="71"/>
        <source>Ctrl+S</source>
        <translation type="unfinished">Ctrl＋S</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="62"/>
        <source>Invert selection</source>
        <translation type="unfinished">ກັບການເລືອກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="55"/>
        <source>Ctrl+T</source>
        <translation type="unfinished">Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="46"/>
        <source>Move selected to top</source>
        <translation type="unfinished">ເຄື່ອນຍ້າຍຕົວທີ່ເລື້ອກໄວ້ໄປຍັງຈຸດຈອມ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="30"/>
        <source>Remove selection</source>
        <translation type="unfinished">ເອົາຈຸດທີ່ເລື້ອກໄວ້ອອກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="78"/>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>ສຳເນົາເອົາແຖວທີ່ເລຶ້ອກໄວ້ໄປຍັງໜ່ວຍຄວາມຈຳຊົວຄາວ (clipboard)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="81"/>
        <source>Copies the selected rows to the clipboard</source>
        <translation>ສຳເນົາເອົາແຖວທີ່ເລຶ້ອກໄວ້ໄປຍັງໜ່ວຍຄວາມຈຳຊົ່ວຄາວ (clipboard)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="90"/>
        <source>Ctrl+C</source>
        <translation type="unfinished">Ctrl+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="174"/>
        <source>Stop editin&amp;g</source>
        <translation>&amp;ຢຸດແກ້ໄຂ້</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="177"/>
        <source>Alt+G</source>
        <translation type="unfinished">Alt+G</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Search for:</source>
        <translation type="obsolete">ຄົ້ນຫາ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="211"/>
        <source>in</source>
        <translation>ໃນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="231"/>
        <source>Search</source>
        <translation type="unfinished">ຄົ້ນຫາ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="241"/>
        <source>Adva&amp;nced...</source>
        <translation>&amp;ກ້າວໜ້າ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="244"/>
        <source>Alt+N</source>
        <translation type="unfinished">Alt+N</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;ຊ່ວຍເຫລືອ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="119"/>
        <source>New column</source>
        <translation>ຖັນໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="135"/>
        <source>Delete column</source>
        <translation type="unfinished">ລືບຖັນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="97"/>
        <source>Zoom map to the selected rows (Ctrl-F)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="100"/>
        <source>Zoom map to the selected rows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="106"/>
        <source>Ctrl+F</source>
        <translation type="unfinished">ctrl+F</translation>
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
        <translation>ເລື້ອກ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="99"/>
        <source>select and bring to top</source>
        <translation type="unfinished">ເລື້ອກແລະນຳມາຈຸດຈອມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="100"/>
        <source>show only matching</source>
        <translation type="unfinished">ສະແດງພຽງແຕ່ຕົວທີ່ແທດເຫມາະກັນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="330"/>
        <source>Search string parsing error</source>
        <translation type="unfinished">ຄົ້ນຫາ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="382"/>
        <source>Search results</source>
        <translation type="unfinished">ຊອກຫາຜົນຮັບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="336"/>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>ບໍ່ມີໂຕຊອກຫາ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="370"/>
        <source>Error during search</source>
        <translation type="unfinished">ມີຂໍ້ຜິດພາດລະຫວ່າງຄົ້ນຫາ</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="379"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform>ພົບຈຸດເດັ່ນທີ່ແທດເຫມາະກັນພຽງ%d</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="381"/>
        <source>No matching features found.</source>
        <translation type="unfinished">ບໍ່ພົບຈຸດເດັ່ນທີ່ແທດເຫມາະກັນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="148"/>
        <source>Name conflict</source>
        <translation type="unfinished">ຊື່ຄັດກັນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>The attribute could not be inserted. The name already exists in the table</source>
        <translation type="obsolete">ບໍ່ສາມາດສອດຄຸນລັກສະນະໄດ້,ຊື່ມີຢູ່ໃນຕາຕະລາງແລ້ວ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="197"/>
        <source>Stop editing</source>
        <translation>ຢຸດການແກ້ໄຂ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="198"/>
        <source>Do you want to save the changes?</source>
        <translation type="unfinished">ທ່ານຕ້ອງການຈັດເກັບສີ່ງທີ່ໄດ້ແກ້ໄຂໄປບໍ່?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="204"/>
        <source>Error</source>
        <translation>ຜິດພາດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Could not commit changes</source>
        <translation type="obsolete">ບໍ່ສາມາດຈົດຈຳສີ່ງທີ່ໄດ້ແກ້ໄຂໄປ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="148"/>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="204"/>
        <source>Could not commit changes - changes are still pending</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="160"/>
        <source>Really Delete?</source>
        <translation type="unfinished">ລຶບບໍ່?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="161"/>
        <source>Are you sure you want to delete the </source>
        <translation>ທ່ານຫມັ້ນໃຈບໍ່ທີ່ຈະລືບ ບຸກມາກ (Bookmark)?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="161"/>
        <source> bookmark?</source>
        <translation>ຈື່ໄວ້ (Bookmark)?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="177"/>
        <source>Error deleting bookmark</source>
        <translation>ມີຂໍ້ຜິດພາດເກີດຂື້ນໃນການລືບ Bookmark</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="179"/>
        <source>Failed to delete the </source>
        <translation type="unfinished">ບໍ່ສາມາດລືບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="181"/>
        <source> bookmark from the database. The database said:
</source>
        <translation>ບຸກມາກ (Bookmark) ຈາກຖານຂໍ້ມູນ.ຖານຂໍ້ມູນເວົ້າ:</translation>
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
        <translation>ບຸກມາກ (geospatial Bookmark)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="29"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="34"/>
        <source>Project</source>
        <translation type="unfinished">ຫນ້າວຽກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="39"/>
        <source>Extent</source>
        <translation type="unfinished">ຂອບເຂດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="44"/>
        <source>Id</source>
        <translation type="unfinished">Id</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Zoom To</source>
        <translation type="obsolete">ຊຸມເຂົ້າໄປຫາ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Zoom to the currently selected bookmark</source>
        <translation type="obsolete">ຊຸມເຂົ້າໄປຫາບຸກມາກ (Bookmark) ປະຈຸບັນທີ່ຖືກເລື້ອກໄວ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Delete</source>
        <translation type="obsolete">ລຶບບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Delete the currently selected bookmark</source>
        <translation type="obsolete">ລຶບ Bookmark ທີ່ຖືກເລື້ອກປະຈຸບັນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Close</source>
        <translation type="obsolete">ປິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Close the dialog</source>
        <translation type="obsolete">ປິດຂໍ້ຄວາມ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Help</source>
        <translation type="obsolete">ຊ່ວຍເຫຼືອ</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="688"/>
        <source>Choose a filename to save the map image as</source>
        <translation type="unfinished">ເລື້ອກຊື່ແຟ້ມເພື່ອຈັດເກັບຮູບແຜນທີ່ໃຫ້ເປັນ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="783"/>
        <source>Choose a filename to save the map as</source>
        <translation type="unfinished">ເລື້ອກຊື່ແຟ້ມເພື່ອຈັດເກັບຮູບແຜນທີ່ນື່ງໃຫ້ເປັນ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="472"/>
        <source> for read/write</source>
        <translation type="unfinished">ສຳລັບອ່ານແລະຂານ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="551"/>
        <source>Error in Print</source>
        <translation type="unfinished">ມີຂໍ້ຜິດພາດໃນການພີມ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="529"/>
        <source>Cannot seek</source>
        <translation type="unfinished">ບໍ່ສາມາດຊອກຫາ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="454"/>
        <source>Cannot overwrite BoundingBox</source>
        <translation>ບໍ່ສາມາດຂຽນທັບ BoundingBox</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="462"/>
        <source>Cannot find BoundingBox</source>
        <translation>ບໍ່ສາມາດຊອກຫາ BoundingBox</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="545"/>
        <source>Cannot overwrite translate</source>
        <translation type="unfinished">ບໍ່ສາມາດຂຽນທັບຕົວແປ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="551"/>
        <source>Cannot find translate</source>
        <translation type="unfinished">ບໍ່ສາມາດຫາບຕົວແປ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="559"/>
        <source>File IO Error</source>
        <translation type="unfinished">ແຟ້ມ IO ມີຂໍ້ຜິດພາດ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="569"/>
        <source>Paper does not match</source>
        <translation type="unfinished">ເຈ້ຍບໍ່ແທດເຫມາະ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="570"/>
        <source>The selected paper size does not match the composition size</source>
        <translation type="unfinished">ຄະຫນາດເຈ້ຍທີ່ເລື້ອກໄວ້ບໍ່ແທດເຫມາະກັບຄະໜາດລຽມພີມ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="634"/>
        <source>Big image</source>
        <translation type="unfinished">ຮູບພາບໃຫຍ່</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="635"/>
        <source>To create image </source>
        <translation>ຈະສ້າງພາບ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="638"/>
        <source> requires circa </source>
        <translation>ຕ້ອງການ circa</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="638"/>
        <source> MB of memory</source>
        <translation>ຈຳນວນເມກກະໄບ (Megabyte) ຂອງຄວາມຈຳ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="56"/>
        <source>QGIS - print composer</source>
        <translation>QGIS-ຜູ້ລຽງພີມ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="81"/>
        <source>Map 1</source>
        <translation>ແຜນທີ່ 1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="472"/>
        <source>Couldn&apos;t open </source>
        <translation type="unfinished">ບໍ່ສາມາດເປີດ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="665"/>
        <source>format</source>
        <translation>ຮູບແບບ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="757"/>
        <source>SVG warning</source>
        <translation>ເຕືອນ SVG</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="758"/>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished">ບໍ່ສະແດງຂໍ້ຄວາມອີກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the Qt4 svg code. Of note, text does not appear in the SVG file and there are problems with the map bounding box clipping other items such as the legend or scale bar.&lt;/p&gt;If you require a vector-based output file from Qgis it is suggested that you try printing to PostScript if the SVG output is not satisfactory.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the Qt4 svg code. Of note, text does not appear in the SVG file and there are problems with the map bounding box clipping other items such as the legend or scale bar.&lt;/p&gt;If you require a vector-based output file from Qgis it is suggested that you try printing to PostScript if the SVG output is not satisfactory.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="784"/>
        <source>SVG Format</source>
        <translation>ຮູບແບບຂອງ SVG</translation>
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
        <location filename="../src/ui/qgscomposerbase.ui" line="62"/>
        <source>General</source>
        <translation type="unfinished">ທົວໄປ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="107"/>
        <source>Composition</source>
        <translation type="unfinished">ສ່ວນປະກອບຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="115"/>
        <source>Item</source>
        <translation>ລາຍການ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="208"/>
        <source>&amp;Open Template ...</source>
        <translation>ເ&amp;ປີດເເບບຟອມ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="216"/>
        <source>Save Template &amp;As...</source>
        <translation>ເ&amp;ກັບເເບບຟອມເປັນ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="224"/>
        <source>&amp;Print...</source>
        <translation>&amp;ພີມ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="256"/>
        <source>Add new map</source>
        <translation type="unfinished">ເພີ້ມແຜນທີ່ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="264"/>
        <source>Add new label</source>
        <translation type="unfinished">ເພີ້ມຊື່ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="272"/>
        <source>Add new vect legend</source>
        <translation type="unfinished">ເພີ່ມຄຳບັນຍາຍຂອງຕົວເລກທີ່ບອກຄະຫນາດແລະທິດທ່າງໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="280"/>
        <source>Select/Move item</source>
        <translation>ເລື້ອກຫຼືຍາຍລາຍການ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="288"/>
        <source>Export as image</source>
        <translation type="unfinished">ສົ່ງອອກເປັນແຟ້ມຮູບ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="296"/>
        <source>Export as SVG</source>
        <translation>ສົ່ງອອກເປັນຮູບ SVG</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="304"/>
        <source>Add new scalebar</source>
        <translation type="unfinished">ເພີ້ມທ່ອນມາດຕາສ່ວນໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="312"/>
        <source>Refresh view</source>
        <translation type="unfinished">ກະຕຸ້ນພາບໃນການເບີ່ງໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation type="unfinished">ວີນໂດ້ຫຼັກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="232"/>
        <source>Zoom All</source>
        <translation>ຊຸມ (Zoom) ເບີ່ງທັ້ງຫມົດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="240"/>
        <source>Zoom In</source>
        <translation>ດື່ງພາບເຂົ້າໃກ້ (Zoom In)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="248"/>
        <source>Zoom Out</source>
        <translation>ດືງພາບອອກໄກ (Zoom out)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="320"/>
        <source>Add Image</source>
        <translation type="unfinished">ເພີ້ມຮູບ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="170"/>
        <source>Close</source>
        <translation type="unfinished">ປິດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="150"/>
        <source>Help</source>
        <translation type="unfinished">ຊ່ວຍເຫຼືອ</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="21"/>
        <source>Label Options</source>
        <translation type="unfinished">ທາງເລື້ອກໃສ່ຊື່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="48"/>
        <source>Font</source>
        <translation type="unfinished">ແບບຕົວຫນັງສື</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="55"/>
        <source>Box</source>
        <translation type="unfinished">ກອງ</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="99"/>
        <source>Extent (calculate scale)</source>
        <translation type="unfinished">ຂອບເຂດ (ຄິດໄລ່ມາດຕາສ່ວນ)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="100"/>
        <source>Scale (calculate extent)</source>
        <translation>ມາດຕາສ່ວນ (ຄິດໄລຂອບເຂດ)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="77"/>
        <source>Map %1</source>
        <translation type="unfinished">ແຜນທີ່ %1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="107"/>
        <source>Cache</source>
        <translation type="unfinished">ໜ່ວຍຄວາມຈຳຊົວຄາວ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="108"/>
        <source>Render</source>
        <translation>ປະສົມປະສານ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="109"/>
        <source>Rectangle</source>
        <translation type="unfinished">ຮູບສີ່ຫຼ່ຽມມຸມສາກ</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapBase</name>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="21"/>
        <source>Map options</source>
        <translation type="unfinished">ທາງເລື້ອກໃນການສ້າງແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="173"/>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation>&lt;b&gt;ແຜນທີ່&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="147"/>
        <source>Set</source>
        <translation>ວາງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="196"/>
        <source>Width</source>
        <translation type="unfinished">ຄວາມກ້ວາງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="180"/>
        <source>Height</source>
        <translation type="unfinished">ຄວາມສູງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Scale</source>
        <translation type="obsolete">ມາດຕາສ່ວນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>1 :</source>
        <translation type="obsolete">1 :</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="72"/>
        <source>Set Extent</source>
        <translation type="unfinished">ຕັ້ງຂອບເຂດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="69"/>
        <source>Set map extent to current extent in QGIS map canvas</source>
        <translation>ຕັ້ງຂອບເຂດຂອງແຜນທີ່ໃສ່ຂອບເຂດປະຈຸບັນໃນພື້ນແຜນທີ່ QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="212"/>
        <source>Line width scale</source>
        <translation type="unfinished">ມາດຕາສ່ວນຄວາມກ້ວາງຂອງເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="116"/>
        <source>Width of one unit in millimeters</source>
        <translation type="unfinished">ຄວາມກ້ວາງຂອງນື່ງຫົວໜ່ວຍເປັນມິລີແມັດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="225"/>
        <source>Symbol scale</source>
        <translation type="unfinished">ເຄື່ອງໝາຍຂອງມາດຕາສ່ວນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="238"/>
        <source>Font size scale</source>
        <translation type="unfinished">ມາດຕາສ່ວນຂອງຄະໜາດແບບຕົວໜັງສື</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="251"/>
        <source>Frame</source>
        <translation type="unfinished">ກອບຮູບ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="258"/>
        <source>Preview</source>
        <translation type="unfinished">ພາບກ່ອນຈັດພີມ</translation>
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
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="399"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="400"/>
        <source>Cannot load picture.</source>
        <translation type="unfinished">ບໍ່ສາມາດໂຫຼດຮູບ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="483"/>
        <source>Choose a file</source>
        <translation type="unfinished">ເລື້ອກແຟ້ມ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="466"/>
        <source>Pictures (</source>
        <translation>ຮູບຕ່າງໆ (</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureBase</name>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="21"/>
        <source>Picture Options</source>
        <translation>ທາງເລື້ອກຂອງຮູບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Picture</source>
        <translation type="obsolete">ຮູບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="197"/>
        <source>Frame</source>
        <translation type="unfinished">ກອບຮູບ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="161"/>
        <source>Angle</source>
        <translation type="unfinished">ມູມ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="119"/>
        <source>Width</source>
        <translation type="unfinished">ຄວາມກ້ວາງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="140"/>
        <source>Height</source>
        <translation type="unfinished">ຄວາມສູງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="58"/>
        <source>Browse</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerScalebarBase</name>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="21"/>
        <source>Barscale Options</source>
        <translation>ທາງເລື້ອກຂອງທ່ອນມາດຕາສ່ວນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="140"/>
        <source>Segment size</source>
        <translation>ຄະໜາດຂອງທ່ອນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="172"/>
        <source>Number of segments</source>
        <translation>ຈຳນວນຂອງຄະໜາດຫລາຍທ່ອນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="159"/>
        <source>Map units per scalebar unit</source>
        <translation>ຫົວໜ່ວຍແຜນທີ່ຕໍ່ຫົວໜ່ວຍທ່ອນມາດຕາສ່ວນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="88"/>
        <source>Unit label</source>
        <translation type="unfinished">ຊື່ຫົວໜ່ວຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="127"/>
        <source>Map</source>
        <translation type="unfinished">ແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="195"/>
        <source>Font</source>
        <translation type="unfinished">ແບບຕົວຫນັງສື</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="41"/>
        <source>Line width</source>
        <translation type="unfinished">ຄວາມກ້ວາງຂອງເສັ້ນ</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegend</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Layers</source>
        <translation type="obsolete">ລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Group</source>
        <translation type="obsolete">ການຮວມກຸຸ່່ມ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="718"/>
        <source>Combine selected layers</source>
        <translation type="unfinished">ລວມລະດັບຊັ້ນທີ່ຖືກເລື້ອກໄວ້ແລ້ວ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="134"/>
        <source>Cache</source>
        <translation type="unfinished">ໜ່ວຍຄວາມຈຳຊົວຄາວ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="135"/>
        <source>Render</source>
        <translation>ປະສົມປະສານ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="136"/>
        <source>Rectangle</source>
        <translation type="unfinished">ຮູບສີ່ຫຼ່ຽມມຸມສາກ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="110"/>
        <source>Legend</source>
        <translation type="unfinished">ຄຳອະທິບາຍຫຼືເຄື່ອງໝາຍຂອງແຜນທີ່</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="21"/>
        <source>Vector Legend Options</source>
        <translation type="unfinished">ທາງເລື້ອກຂອງຄຳອະທິບາຍຫຼືເຄື່ອງໝາຍແຜນທີ່ເປັນຕົວເລກບອກຄະໜາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="113"/>
        <source>Title</source>
        <translation type="unfinished">ຫົວຂໍ້</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="92"/>
        <source>Map</source>
        <translation type="unfinished">ແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="182"/>
        <source>Font</source>
        <translation type="unfinished">ແບບຕົວຫນັງສື</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="167"/>
        <source>Box</source>
        <translation type="unfinished">ກອງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Column 1</source>
        <translation type="obsolete">ຖັນ 1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="53"/>
        <source>Preview</source>
        <translation>ເບີ່ງພາບກ່ອນຈັດພີມ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="149"/>
        <source>Layers</source>
        <translation type="unfinished">ລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="154"/>
        <source>Group</source>
        <translation type="unfinished">ການຮວມກຸຸ່່ມ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="159"/>
        <source>ID</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposition</name>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="88"/>
        <source>Custom</source>
        <translation type="unfinished">ການຈັດການຫຼືຕັ້ງຄ່າຕາມຄວາມມັກ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="89"/>
        <source>A5 (148x210 mm)</source>
        <translation type="unfinished">A5 (148x210 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="90"/>
        <source>A4 (210x297 mm)</source>
        <translation type="unfinished">A4 (210x297 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="91"/>
        <source>A3 (297x420 mm)</source>
        <translation type="unfinished">A3 (297x420 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="92"/>
        <source>A2 (420x594 mm)</source>
        <translation type="unfinished">A2 (420x594 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="93"/>
        <source>A1 (594x841 mm)</source>
        <translation type="unfinished">A1 (594x841 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="94"/>
        <source>A0 (841x1189 mm)</source>
        <translation type="unfinished">A0 (841x1189 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="95"/>
        <source>B5 (176 x 250 mm)</source>
        <translation type="unfinished">B5 (176 x 250 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="96"/>
        <source>B4 (250 x 353 mm)</source>
        <translation type="unfinished">B4 (250 x 353 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="97"/>
        <source>B3 (353 x 500 mm)</source>
        <translation type="unfinished">B3 (353 x 500 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="98"/>
        <source>B2 (500 x 707 mm)</source>
        <translation type="unfinished">B2 (500 x 707 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="99"/>
        <source>B1 (707 x 1000 mm)</source>
        <translation type="unfinished">B1 (707 x 1000 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="100"/>
        <source>B0 (1000 x 1414 mm)</source>
        <translation type="unfinished">B0 (1000 x 1414 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="101"/>
        <source>Letter (8.5x11 inches)</source>
        <translation>ຫນັງສື (8.5x11 ນິ້ວ)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="102"/>
        <source>Legal (8.5x14 inches)</source>
        <translation>Legal (8.5x14 ນິ້ວ)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="115"/>
        <source>Portrait</source>
        <translation type="unfinished">ການຈັດພີມທາງຕັ້ງ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="116"/>
        <source>Landscape</source>
        <translation type="unfinished">ການຈັດພີມທາງຂວາງ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="632"/>
        <source>Out of memory</source>
        <translation type="unfinished">ນຳໃຊ້ຫນ່ວຍຄວາມຈຳຫມົດ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="635"/>
        <source>Qgis is unable to resize the paper size due to insufficient memory.
 It is best that you avoid using the map composer until you restart qgis.
</source>
        <translation>Qgis ບໍ່ສາມາດປັບຄະຫນາດຂອງຫນາດເຈ້ຍໄດ້ເນື່ອງຈາກຫນ່ວຍຄວາມຈຳບໍ່ພໍ.ເປັນການດີທີ່ສຸດຖ້າທ່ານຫຼີກລ້ຽງການນຳໃຊ້ຜູ້ແຕ່ງແຜນທີ່ (Map Composer)ຈັນກະທັ້ງເລີ້ມ Qgisໃຫມ່.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="779"/>
        <source>Label</source>
        <translation>ກາຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="829"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="830"/>
        <source>Cannot load picture.</source>
        <translation type="unfinished">ບໍ່ສາມາດໂຫຼດຮູບ</translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="21"/>
        <source>Composition</source>
        <translation type="unfinished">ສ່ວນປະກອບຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="33"/>
        <source>Paper</source>
        <translation type="unfinished">ເຈ້ຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="176"/>
        <source>Size</source>
        <translation type="unfinished">ຄະຫນາດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="158"/>
        <source>Units</source>
        <translation type="unfinished">ຫົວຫນ່ວຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="140"/>
        <source>Width</source>
        <translation type="unfinished">ຄວາມກ້ວາງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="122"/>
        <source>Height</source>
        <translation type="unfinished">ຄວາມສູງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="104"/>
        <source>Orientation</source>
        <translation>ການກຳໜົດທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="213"/>
        <source>Resolution (dpi)</source>
        <translation>ຄວາມລະອຽດຂອງພາບ (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialog</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Test connection</source>
        <translation type="obsolete">ທົດສອບການເຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Connection to </source>
        <translation type="obsolete">ການເຊື່ອມຕໍ່ກັບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> was successfull</source>
        <translation type="obsolete">ໄດ້ສຳເລັດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Connection failed - Check settings and try again </source>
        <translation type="obsolete">ການເຊື່ອມຕໍ່ບໍໄດ້-ກວດເບິ່ງການຕັ້ງຄ່າແລ້ວລ້ອງໄຫມ່ອີກຄັ້ງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>General Interface Help:

</source>
        <translation type="obsolete">ການຊ່ວຍເຫຼືອຂອງຕົວປະສານງານລະຫວ່າງຄອມພີວເຕີແລະຄົນໂດຍທົ່ວໄປ:</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialogBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Connection Information</source>
        <translation type="obsolete">ຂໍ້ມູນການເຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Host</source>
        <translation type="obsolete">ຫນ່ວຍແມ່ (Host)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Database</source>
        <translation type="obsolete">ຖານຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Username</source>
        <translation type="obsolete">ຊື່ຜູ້ໃຊ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name</source>
        <translation type="obsolete">ຊື່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name of the new connection</source>
        <translation type="obsolete">ຊື່ຂອງການເຊື່ອມຕໍ່ຄັ້ງໃຫມ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Password</source>
        <translation type="obsolete">ລະຫັດຜ່ານ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Test Connect</source>
        <translation type="obsolete">ທົດສອບການເຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Save Password</source>
        <translation type="obsolete">ດເກັບລະຫັດຜ່ານ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Create a New PostGIS connection</source>
        <translation type="obsolete">ສ້າງການເຊື່ອມຕໍ່ PostGIS ໃຫມ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Port</source>
        <translation type="obsolete">ພອດ (port)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>5432</source>
        <translation type="obsolete">5432</translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="13"/>
        <source>Continuous color</source>
        <translation type="unfinished">ສີຕໍ່ເນື່ອງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="54"/>
        <source>Maximum Value:</source>
        <translation>ຄ່າສູງສຸດ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="80"/>
        <source>Outline Width:</source>
        <translation type="unfinished">ຄວາມໜາຂອງເສັ້ນລອບນອກ: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="93"/>
        <source>Minimum Value:</source>
        <translation>ຄ່າຕຳ່ສຸດ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="109"/>
        <source>Classification Field:</source>
        <translation>ສະໜາມການຈັດໝວດໝູ່:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="119"/>
        <source>Draw polygon outline</source>
        <translation>ຂີດເສັ້ນລອບນອກຫຼາຍຫຼ່ຽມ່ຽມ</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>Failed</source>
        <translation type="unfinished">ລົ້ມເຫລວ</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>transform of</source>
        <translation type="unfinished">ການປ່ຽນແປງຂອງ</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="496"/>
        <source>with error: </source>
        <translation type="unfinished">ມີຂໍ້ຜິດພາດ:</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="418"/>
        <source>The source spatial reference system (SRS) is not valid. </source>
        <translation>ລະບົບອ້າງອີງແຫຼ່ງ (SRS) ໃຊ້ການບໍ່ໄດ້່ໄດ້່ໄດ້</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="426"/>
        <source>The coordinates can not be reprojected. The SRS is: </source>
        <translation>ຕົວປະສານບໍ່ສາມາດຄາດຄະເນໄດ້.SRS ແມ່ນ:</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="425"/>
        <source>The destination spatial reference system (SRS) is not valid. </source>
        <translation>ຈຸດຫມາຍລະບົບອ້າງອີງແຫຼ່ງ (SRS) ບໍ່ໄດ້</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="66"/>
        <source>Bottom Left</source>
        <translation>ຂ້າງລຸ່ມເບື້ອງຊ້າຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Left</source>
        <translation>ຂ້າງເທິງເບື້ອງຊ້າຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Right</source>
        <translation type="unfinished">ຂ້າງເທິງເບື້ອງຂວາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Bottom Right</source>
        <translation>ພື້ນເບື້ອງຂວາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="79"/>
        <source>&amp;Copyright Label</source>
        <translation>&amp;ກາຫມາຍລິຂະສິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="80"/>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>ສ້າງກາຫມາຍລິຂະສິດເຊິ່ງສະແດງຢູ່ເທິງພື້ນແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="204"/>
        <source>&amp;Decorations</source>
        <translation>&amp;ການຕົກແຕ່ງ</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="13"/>
        <source>Copyright Label Plugin</source>
        <translation>ໂປຼກຼາມເສີ້ມເພື່ອໃສ່ກາຫມາຍລິຂະສິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="145"/>
        <source>Placement</source>
        <translation type="unfinished">ການວາງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation type="unfinished">ຂ້າງລຸ່ມເບື້ອງຊ້າຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="158"/>
        <source>Top Left</source>
        <translation type="unfinished">ຂ້າງເທິງເບື້ອງຊ້າຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="163"/>
        <source>Bottom Right</source>
        <translation type="unfinished">ຂ້າງລຸ່ມເບື້ອງຂວາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="168"/>
        <source>Top Right</source>
        <translation type="unfinished">ຂ້າງເທິງເບື້ອງຂວາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="176"/>
        <source>Orientation</source>
        <translation type="unfinished">ການກຳຫນົດທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="184"/>
        <source>Horizontal</source>
        <translation type="unfinished">ລວງນອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="189"/>
        <source>Vertical</source>
        <translation type="unfinished">ລວງຕັ້ງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="118"/>
        <source>Enable Copyright Label</source>
        <translation>ເຮັດໃຫ້ສາມາດໃສ່ກາຫມາຍລິຂະສິດໄດ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Enter your copyright label below. This plugin supports basic html markup tags for formatting the label. For example:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Bold text &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Italics &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(note: &amp;amp;copy; gives a copyright symbol)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt; ບັນຍາຍ/span&gt;&lt;/p&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;&amp;gt;&amp;lt;&amp;gt;&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;&amp;gt;&amp;lt;&amp;gt;&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;&amp;amp;&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-size:14pt;&quot;&gt;&#xa9; QGIS 2006&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-size:14pt;&quot;&gt;© QGIS 2006&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="36"/>
        <source>Color</source>
        <translation type="unfinished">ສີ</translation>
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
        <source>© QGIS 2008</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialog</name>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="175"/>
        <source>Delete Projection Definition?</source>
        <translation type="unfinished">ລືບຂໍ້ກຳຫນົດຂອງການວາງແຜນການບໍ່?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="176"/>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>ລືບຂໍ້ກຳຫນົດຂອງການວາງແຜນຈະເຮັດໃຫ້ບໍ່ສາມາດແກ້ໄຂ້ໄດ້ພາຍຫຼັງ. ທ່ານຕ້ອງການຢາກລືບບໍ?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="882"/>
        <source>Abort</source>
        <translation type="unfinished">ຍົກເລີກໂດຍກາງຄັນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="884"/>
        <source>New</source>
        <translation type="unfinished">ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="939"/>
        <source>QGIS Custom Projection</source>
        <translation>ການວ່າງແຜນການຂອງ QGIS ຕາມຄວາມຕ້ອງການຂອງຕົນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="800"/>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation> ລືບຂໍ້ກຳຫນົດຂອງການວາງແຜນການ (proj4) ໃຊ້ການບໍ່ໄດ້,ກະລຸນາແກ້ໄຂ້ໃຫ້ຖືກຕ້ອງກ່ອນກົດປຸ່ມການຈັດເກັບ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="913"/>
        <source>This proj4 projection definition is not valid.</source>
        <translation>ລືບຂໍ້ກຳຫນົດຂອງການວາງແຜນການ (proj4) ໃຊ້ການບໍ່ໄດ້。</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="928"/>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation type="unfinished">ທາງທິດເຫນືອແລະໃຕ້ຕ້ອງຢູ່ໃນຮູບແບບຫົວສີບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="756"/>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>ຂໍ້ກຳຫນົດຂອງການວາງແຜນການ (proj4) ໃຊ້ການບໍ່ໄດ້.ກະລຸນາໃສ່ຊື່ແຜນການກ່ອນຈັດເກັບ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="762"/>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>ຂໍ້ກຳຫນົດຂອງການວາງແຜນການ (proj4) ໃຊ້ການບໍ່ໄດ້,ກະລຸນາເພີ້ມຕົວແປຄຳສັ່ງ (Parameters) ກ່ອນຈັດເກັບ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="777"/>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>ຂໍ້ກຳຫນົດຂອງການວາງແຜນການ (proj4) ໃຊ້ການບໍ່ໄດ້,ກະລຸນາເພີ້ມ Proj=clause ກ່ອນຈັດເກັບ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="784"/>
        <source>This proj4 ellipsoid definition is not valid. Please add a ellips= clause before pressing save.</source>
        <translation>ຂໍ້ກຳຫນົດແຜນການທີ່ມີຊົງຮູບໄຂ່ໃຊ້ບໍ່ໄດ້,ກະລຸນາເພີ້ມ ellips=clause ກ່ອນຈັດເກັບ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="940"/>
        <source>Internal Error (source projection invalid?)</source>
        <translation type="unfinished">ມີຂໍ້ຜິດພາດເກີດຂື້ນພາຍໃນ(ແຫລ່ງແຜນການໃຊ້ບໍ່ໄດ້ບໍ່?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Name:</source>
        <translation type="obsolete">ຊື່:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="13"/>
        <source>Custom Projection Definition</source>
        <translation>ຈັດຂໍ້ກຳຫນົດແຜນການຕາມຄວາມຕ້ອງການຂອງຕົນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Parameters:</source>
        <translation type="obsolete">ຕົວແປຄຳສັ່ງ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="66"/>
        <source>|&lt;</source>
        <translation type="unfinished">|&lt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="76"/>
        <source>&lt;</source>
        <translation type="unfinished">&lt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="86"/>
        <source>1 of 1</source>
        <translation>1 ຂອງ 1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="96"/>
        <source>&gt;</source>
        <translation type="unfinished">&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="106"/>
        <source>&gt;|</source>
        <translation type="unfinished">&gt;|</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>New</source>
        <translation type="obsolete">ໃຫມ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Save</source>
        <translation type="obsolete">ເກັບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Delete</source>
        <translation type="obsolete">ລຶບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Close</source>
        <translation type="obsolete">ປິດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="22"/>
        <source>Define</source>
        <translation>ກຳຫນົດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="151"/>
        <source>Test</source>
        <translation type="unfinished">ທົດລອງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Transform from WGS84 to the chosen projection</source>
        <translation type="obsolete">ປ່ຽນແປງຈາກ WGS84 ໄປຫາແຜ່ນການທີ່ເລື້ອກໄວ້ແລ້ວ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="253"/>
        <source>Calculate</source>
        <translation type="unfinished">ຄິດໄລ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Projected Corrdinate System</source>
        <translation type="obsolete">ລະບົບຕົວປະສານທີ່ໄດ້ຮັບການສ້າງແຜນໄວ້ແລ້ວ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="180"/>
        <source>Geographic / WGS84</source>
        <translation>ພູມມິສາດ / WGS84</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>North:</source>
        <translation type="obsolete">ທິດເຫນືອ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>East:</source>
        <translation type="obsolete">ທິດຕາເວັນອອກ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;You can define your own custom projection here. The definition must conform to the proj4 format for specifying a Spatial Reference System.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt; ຮູບແບບການຈັດແຟ້ມໃນລະບົບຄອມພີວເຕີ&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Use the text boxes below to test the projection definition you are creating. Enter a coordinate where both the lat/long and the projected result are known (for example by reading off a map). Then press the calculate button to see if the projection definition you are creating is accurate.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Use the text boxes below to test the projection definition you are creating. Enter a coordinate where both the lat/long and the projected result are known (for example by reading off a map). Then press the calculate button to see if the projection definition you are creating is accurate.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="28"/>
        <source>You can define your own custom projection here. The definition must conform to the proj4 format for specifying a Spatial Reference System.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="38"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="167"/>
        <source>Parameters</source>
        <translation type="unfinished">ຕົວຂອບເຂດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="116"/>
        <source>*</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="126"/>
        <source>S</source>
        <translation type="unfinished">S</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="136"/>
        <source>X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="157"/>
        <source>Use the text boxes below to test the projection definition you are creating. Enter a coordinate where both the lat/long and the projected result are known (for example by reading off a map). Then press the calculate button to see if the projection definition you are creating is accurate.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="187"/>
        <source>Projected Coordinate System</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="194"/>
        <source>North</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="224"/>
        <source>East</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="230"/>
        <source>Are you sure you want to remove the </source>
        <translation type="unfinished">ທ່ານໝັ້ນໃຈບໍ່ວ່າຈະເອົາອອກ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="230"/>
        <source> connection and all associated settings?</source>
        <translation type="unfinished">ການເຊື່ອມຕໍ່ແລະການຕັ້ງຄ່າທີ່ພົວພັນກັບມັນທັງໝົດ?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="231"/>
        <source>Confirm Delete</source>
        <translation type="unfinished">ຢືນຢັນລືບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="338"/>
        <source>Select Table</source>
        <translation type="unfinished">ເລື້ອກຕາຕະລາງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="338"/>
        <source>You must select a table in order to add a Layer.</source>
        <translation type="unfinished">ທ່ານຕ້ອງເລື້ອກຕາຕະລາງເພື່ອທີ່ຈະເພີ້ມລະດັບຊັ້ນໃດນື່ງ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="371"/>
        <source>Password for </source>
        <translation type="unfinished">ລະຫັດລັບສຳລັບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="372"/>
        <source>Please enter your password:</source>
        <translation type="unfinished">ກະລຸນາໃສ່ລະຫັດລັບຂອງທ່ານ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="437"/>
        <source>Connection failed</source>
        <translation type="unfinished">ການເຊື່ອມຕໍ່ລົ້ມເຫລວ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Access to relation </source>
        <translation type="obsolete">ເຂົ້າໄປຫາສາຍພົວພັນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> using sql;
</source>
        <translation type="obsolete">ກຳລັງນຳໃຊ້ sql;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>
has failed. The database said:
</source>
        <translation type="obsolete">ໄດ້ລົ້ມເຫລວ.ຖານຂໍ້ມູນບອກ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="147"/>
        <source>Type</source>
        <translation type="unfinished">ຊະນິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name</source>
        <translation type="obsolete">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="155"/>
        <source>Sql</source>
        <translation type="unfinished">Sql</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Point layer</source>
        <translation type="obsolete">ລະດັບຊັ້ນນຶ່ງຈຸດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Multi-point layer</source>
        <translation type="obsolete">ລະດັບຊັ້ນຫຼາຍຈຸດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Linestring layer</source>
        <translation type="obsolete">ເເຖວລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Multi-linestring layer</source>
        <translation type="obsolete">ຫລາຍເເຖວລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Polygon layer</source>
        <translation type="obsolete">ລະດັບຊັ້ນຂອງຮູບຫຼາຍລ່ຽມ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Multi-polygon layer</source>
        <translation type="obsolete">ລະດັບຊັ້ນຂອງຮູບຫຼາຍລ່ຽມຫຼາຍຕົວ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Mixed geometry layer</source>
        <translation type="obsolete">ລະດັບຊັ້ນທີ່ປະສົມປະສານຮູບເລຂາຄະນິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Geometry collection layer</source>
        <translation type="obsolete">ການລວບລວມຮູບເລຂາຄະນິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Waiting for layer type</source>
        <translation type="obsolete">ລໍຖ້າຊະນິດຂອງລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Unknown layer type</source>
        <translation type="obsolete">ບໍ່ຮູ້ຊະນິດຂອງລະດັບຊັ້ນັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="440"/>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>ການເຊື່ອມຕໍ່ໄປຫາ %1 ເທິງ 2%ລົ້ມເຫລວ.ທັງຖານຂອງມູນເພແລະການຕັ້ງຄ່າບໍ່ຖືກຕ້ອງທັງສອງອັນ.%3 ກວດເບິ່ງຊື່ຜູ້້ແລະລະຫັດຜ່ານແລ້ວລອງອີກເທ່ອນຶ່ງ.ຖານຂໍ້ມູນບອກວ່າ: %5%6ື ຄ່າ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="123"/>
        <source>Wildcard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="127"/>
        <source>RegExp</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="135"/>
        <source>All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="139"/>
        <source>Schema</source>
        <translation type="unfinished">ແບບແຜນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="143"/>
        <source>Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="151"/>
        <source>Geometry column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="547"/>
        <source>Accessible tables could not be determined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="549"/>
        <source>Database connection was successful, but the accessible tables could not be determined.

The error message from the database was:
%1
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="554"/>
        <source>No accessible tables found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="558"/>
        <source>Database connection was successful, but no accessible tables were found.

Please verify that you have SELECT privilege on a table carrying PostGIS
geometry.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelectBase</name>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="13"/>
        <source>Add PostGIS Table(s)</source>
        <translation>ເພີ້ມຕາຕະລາງຂອງ PostGIS </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="140"/>
        <source>Add</source>
        <translation type="unfinished">ເພີ້ມ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="111"/>
        <source>Help</source>
        <translation type="unfinished">ຊ່ວຍເຫຼືອ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="114"/>
        <source>F1</source>
        <translation type="unfinished">F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="87"/>
        <source>Connect</source>
        <translation type="unfinished">ເຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="80"/>
        <source>New</source>
        <translation type="unfinished">ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="73"/>
        <source>Edit</source>
        <translation type="unfinished">ແກ້ໄຂ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="66"/>
        <source>Delete</source>
        <translation>ລຶບ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="156"/>
        <source>Close</source>
        <translation type="unfinished">ປິດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation>ການເຊື່ອມຕໍ່ຂອງ PostgreSQL</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Tables:</source>
        <translation type="obsolete">ຕາຕະລາງ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Type</source>
        <translation type="obsolete">ຊະນິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name</source>
        <translation type="obsolete">ຊື່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Sql</source>
        <translation type="obsolete">Sql</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Encoding:</source>
        <translation type="obsolete">ການປຽນລະຫັດລັບ(encoding):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="183"/>
        <source>Search:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="190"/>
        <source>Search mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="200"/>
        <source>Search in columns:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="216"/>
        <source>Search options...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="24"/>
        <source>Schema</source>
        <translation type="unfinished">ແບບແຜນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="25"/>
        <source>Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="26"/>
        <source>Type</source>
        <translation type="unfinished">ຊະນິດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="27"/>
        <source>Geometry column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="28"/>
        <source>Sql</source>
        <translation type="unfinished">Sql</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="229"/>
        <source>Point</source>
        <translation type="unfinished">ຈຸດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="233"/>
        <source>Multipoint</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="237"/>
        <source>Line</source>
        <translation type="unfinished">ເເຖວ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="241"/>
        <source>Multiline</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="245"/>
        <source>Polygon</source>
        <translation type="unfinished">ຮູບຫຼາຍຫຼ່ຽມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="249"/>
        <source>Multipolygon</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="13"/>
        <source>Delete Attributes</source>
        <translation type="unfinished">ລືບຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>OK</source>
        <translation type="obsolete">ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">ຍົກເລີກ</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="101"/>
        <source>&amp;Add Delimited Text Layer</source>
        <translation>ເ&amp;ພີ້ມລະດັບຊັ້ນຂອງເອກະສານທີ່ຖືກກຳໜົດເຂດແດນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="104"/>
        <source>Add a delimited text file as a map layer. </source>
        <translation type="unfinished">ເພີ້ມແຟ້ມເອກະສານທີ່ຖືກກຳໜົດເຂດແດນເປັນລະດັບຊັ້ນແຜນທີ່ </translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>The file must have a header row containing the field names. </source>
        <translation type="unfinished">ແຟ້ມຂອງທ່ານຕ້ອງມີແຖວຫົວຂໍ້ເອກະສານທີ່ມີບ່ອນໃສ່ຊື່</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>X ເເລະ Y ຈຸດເເມ່ນຈໍເປັນມີ ເເລະ ຕ້ອງມີຈຸດພິກັດຂອງທີ່ເປັນໂຕເລກ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="142"/>
        <source>&amp;Delimited text</source>
        <translation>ເ&amp;ອກະສານທີ່ຖືກກຳໜົດເຂດແດນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="56"/>
        <source>DelimitedTextLayer</source>
        <translation type="unfinished">ລະດັບຊັ້ນເອກະສານທີ່ຖືກກຳໜົດເຂດແດນ</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>No layer name</source>
        <translation type="unfinished">ບໍ່ມີຊື່ລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation type="unfinished">ກະລຸນາໃສ່ຊື່ລະດັບຊັ້ນກ່ອນເພີ້ມລະດັບຊັ້ນເຂົ້າໄປໃສ່ແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="211"/>
        <source>No delimiter</source>
        <translation type="unfinished">ບໍ່ມີຕົວກຳໜົດເຂດແດນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="211"/>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>ກະລູນາກໍານົດຕົວກຳໜົດເຂດແດນກ່ອນຈະຮ້ອງເອົາເເຟ້ມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="245"/>
        <source>Choose a delimited text file to open</source>
        <translation type="unfinished">ເລື້ອກແຟ້ມເອກະສານທີ່ກຳໜົດຂອບເຂດເພື່ອເປີດນຳໃຊ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="35"/>
        <source>Parse</source>
        <translation>ຮ້ອງເອົາເຂົ້າ (Parse)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;h1&gt;Description&lt;/h1&gt;&lt;p&gt;Select a delimited text file containing x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!&lt;/p&gt;&lt;p&gt;Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma or tab). After choosing a delimiter, press the parse button an select the columns containing the x and y values for the layer.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;h1&gt;Description&lt;/h1&gt;&lt;p&gt;Select a delimited text file containing x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!&lt;/p&gt;&lt;p&gt;Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma or tab). After choosing a delimiter, press the parse button an select the columns containing the x and y values for the layer.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="60"/>
        <source>Description</source>
        <translation type="unfinished">ລາຍການ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="63"/>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="67"/>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="13"/>
        <source>Create a Layer from a Delimited Text File</source>
        <translation type="unfinished">ສ້າງລະດັບຊັ້ນຈາກອກແຟ້ມເອກະສານທີ່ກຳໜົດຂອບເດນຳໃຊ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="67"/>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;ຂວາ&quot;&gt;X ເຂດ&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="88"/>
        <source>Name of the field containing x values</source>
        <translation>ຊື່ຂອງສະໜາມບັນຈຸຄ່າ x</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="91"/>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>ຊື່ຂອງສະໜາມບັນຈຸຄ່າ x. Choose a field from the list. The list is generated by parsing the header row of the delimited text file້</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="101"/>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;ຂວາ&quot;&gt;Y ເຂດ&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="122"/>
        <source>Name of the field containing y values</source>
        <translation type="unfinished">ຊື່ຂອງສະໜາມບັນຈຸຄ່າ y</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="125"/>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>ຊື່ຂອງສະໜາມບັນຈຸຄ່າ  y. ເລື້ອກເຂດຈາກລາຍການ. ລາຍການທີ່ໄດ້ສ້າງຈາກຕ້ອງມີແຖວຫົວຂໍ້ເອກະສານທີ່ມີບ່ອນໃສ່ຊື່</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="191"/>
        <source>Layer name</source>
        <translation type="unfinished">ຊື່ລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="198"/>
        <source>Name to display in the map legend</source>
        <translation type="unfinished">ຊື່ເພື່ອສະແດງໃນຄຳອະທິບາຍແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="201"/>
        <source>Name displayed in the map legend</source>
        <translation type="unfinished">ຊື່ທີ່ໄດ້ສະແດງຢູ່ໃນແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="297"/>
        <source>Delimiter</source>
        <translation type="unfinished">ຕົວກຳໜົດຂອບເຂດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="318"/>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation type="unfinished">ຕົວກຳໜົດເຂດແດນນຳໃຊ້ເມື່ອແຍກສະໜາມອອກຈາກກັນຢູ່ໃນແຟ້ມເອກະສານ.ຕົວກຳໜົດຂອບເຂດສາມາດເປັນໄດ້ຫູາຍກວ່ານຶ່ງຕົວຫນັງສື</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="321"/>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation type="unfinished">ຕົວກຳໜົດຂອບເຂດນຳໃຊ້ເມື່ອແຍກສະໜາມອອກຈາກກັນຢູ່ໃນແຟ້ມເອກະສານ.ຕົວກຳໜົດຂອບເຂດສາມາດເປັນໄດ້່ານຶຕົວຫນັງສືຫຼືຫຼາຍກວ່າທາງຍາວສື</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="39"/>
        <source>Delimited Text Layer</source>
        <translation type="unfinished">ລະດັບຊັ້ນເອກະສານທີ່ຖືກກຳໜົດຂອບເຂດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="145"/>
        <source>Delimited text file</source>
        <translation type="unfinished">ແຟ້ມັ້ນເອກະສານທີ່ຖືກກຳໜົດຂອບເຂດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="152"/>
        <source>Full path to the delimited text file</source>
        <translation type="unfinished">ເສັ້ນທາງໄປຫາແຟ້ມັ້ນເອກະສານທີ່ຖືກກຳໜົດຂອບເຂດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="155"/>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="168"/>
        <source>Browse to find the delimited text file to be processed</source>
        <translation type="unfinished">ເລື້ອກອ່ານເພື່ອຊອກຫາແຟ້ມເອກະສານທີ່ກຳໜົດຂອບເຂດເພື່ອດຳເນີນການ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="171"/>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="246"/>
        <source>Sample text</source>
        <translation type="unfinished">ຕົວຢ່າງເນື້ອຫາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="174"/>
        <source>Browse...</source>
        <translation>ການເບີ່ງ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="331"/>
        <source>The delimiter is taken as is</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="334"/>
        <source>Plain characters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="344"/>
        <source>The delimiter is a regular expression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="347"/>
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
        <translation>ສັ່ງເກດ:ເສັ້ນທີ່ຕາມມານີ້ບໍ່ສາມາດໂຫຼດໄດ້ເພາະ QGIS ບໍ່ສາມາດລະບຸຄ່າສຳລັບຕົວປະສານ X ແລະ Y</translation>
    </message>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="400"/>
        <source>Error</source>
        <translation>ໍ້ຜິດພາດ</translation>
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
        <translation type="unfinished">ຈຸດເດັ່ນຂອງໜ່ວຍຄວາມຈຳຊົວຄາວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="224"/>
        <source>Buffer distance in map units:</source>
        <translation type="unfinished">ໄລຍະຫ່າງຂອງໜ່ວຍຄວາມຈຳຊົ່ວຄາວຢູ່ໃນຫົວໜ່ວຍແຜນທີ່:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="132"/>
        <source>Table name for the buffered layer:</source>
        <translation type="unfinished">ຊື່ຕາຕະລາງສຳລັບລະດັບຊັ້ນທີ່ຖືກກຳໜົດໜ່ວຍຄວາມຈຳໄວ້ຊົ່ວຄາວ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="172"/>
        <source>Create unique object id</source>
        <translation>ສ້າງ ID ພ່ຽງນຶ່ງດຽວຂອງວັດຖຸໃດນຶ່ງ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="216"/>
        <source>public</source>
        <translation type="unfinished">ສາທາລະນະ</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="59"/>
        <source>Geometry column:</source>
        <translation type="unfinished">ຖັນທີ່ເປັນເລຂາຄະນິດ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="92"/>
        <source>Spatial reference ID:</source>
        <translation>ID ອ້າງອີງກ່ຽວກັບໄລຍະ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="125"/>
        <source>Unique field to use as feature id:</source>
        <translation>ສະໜາມພຽງນຶ່ງດຽວເພື່ອໃຊ້ເປັນ ID ຈຸດເດັ່ນ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="118"/>
        <source>Schema:</source>
        <translation>ແບບແຜນ (Schema):</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="66"/>
        <source>Add the buffered layer to the map?</source>
        <translation type="unfinished">ເພີ້ມລະດັບຊັ້ນທີ່ກຳໜົດໜ່ວຍຄວາມຈຳໄປໃສ່ແຜນທີ່ບໍ່?</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="234"/>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation type="unfinished">&lt;h2&gt;ກຳໜົດໜ່ວຍຄວາມຈຳໃຫ້ຈຸດເດັ່ນໃນລະດັບຊັ້ນ: &lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="28"/>
        <source>Parameters</source>
        <translation>ຕົວຂອບເຂດ</translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Edit Reserved Words</source>
        <translation type="obsolete">ແກ້ໄຂຄຳເວົ້າທີ່ໄດ້ຮັບການສະຫງວນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Status</source>
        <translation type="obsolete">ສະຖານະ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Index</source>
        <translation type="obsolete">ດັດສະນີ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Reserved Words</source>
        <translation type="obsolete">ຄຳເວົ້າທີ່ສະຫງວນໄວ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Double click the Column Name column to change the name of the column.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Double click the Column Name column to change the name of the column.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Column Name</source>
        <translation type="obsolete">ຊື່ຖັນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This shapefile contains reserved words. These may affect the import into PostgreSQL. Edit the column names so none of the reserved words listed at the right are used (click on a Column Name entry to edit). You may also change any other column name if desired.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This shapefile contains reserved words. These may affect the import into PostgreSQL. Edit the column names so none of the reserved words listed at the right are used (click on a Column Name entry to edit). You may also change any other column name if desired.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsDialog</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Status</source>
        <translation type="obsolete">ສະຖານະ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Column Name</source>
        <translation type="obsolete">ຊື່ຖັນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Index</source>
        <translation type="obsolete">ດັດສະນີ</translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <location filename="../src/gui/qgsencodingfiledialog.cpp" line="29"/>
        <source>Encoding:</source>
        <translation>ໃສ່ລະຫັດ:</translation>
    </message>
</context>
<context>
    <name>QgsFillStyleWidgetBase</name>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="83"/>
        <source>Form1</source>
        <translation>ແບບຟອມ 1</translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="84"/>
        <source>Fill Style</source>
        <translation type="unfinished">ເຕີມຮູບເເບບ</translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="90"/>
        <source>PolyStyleWidget</source>
        <translation type="unfinished">PolyStyleWidget</translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="86"/>
        <source>Colour:</source>
        <translation type="unfinished">ສີ:</translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="85"/>
        <source>col</source>
        <translation type="unfinished">ສີ</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="44"/>
        <source>New device %1</source>
        <translation>ອຸປະກອນໃຫມ່ %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="57"/>
        <source>Are you sure?</source>
        <translation>ທ່ານໝັ້ນໃຈບໍ່?</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="58"/>
        <source>Are you sure that you want to delete this device?</source>
        <translation>ທ່ານໝັ້ນໃຈທີ່ຈະລືບອຸປະກອນນີ້ບໍ່?</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="21"/>
        <source>GPS Device Editor</source>
        <translation>ຜູ້ແກ້ໄຂອຸປະກອນ GPS </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="132"/>
        <source>Device name:</source>
        <translation type="unfinished">ຊື່ອຸປະກອນ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="147"/>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation type="unfinished">ນີ້ແມ່ນຊື່ຂອງອຸປະກອນເຊິ່ງມັນປະກົດໃຫ້ເຫັນໃນລາຍການ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="99"/>
        <source>Update device</source>
        <translation type="unfinished">ຍົກລະດັບອຸປະກອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="84"/>
        <source>Delete device</source>
        <translation type="unfinished">ລືບອຸປະກອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="69"/>
        <source>New device</source>
        <translation type="unfinished">ອຸປະກອນໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="298"/>
        <source>Close</source>
        <translation type="unfinished">ປິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="156"/>
        <source>Commands</source>
        <translation type="unfinished">ຄຳສັ່ງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="185"/>
        <source>Waypoint download:</source>
        <translation>ດາວໂຫຼດເອົາ Waypoint:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="248"/>
        <source>Waypoint upload:</source>
        <translation>ອັບໂຫຼດ Waypoint:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="199"/>
        <source>Route download:</source>
        <translation>ເສັ້ນທາງການດາວໂຫລດຂໍ້ມູນປະຈຳ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="178"/>
        <source>Route upload:</source>
        <translation>ເສັ້ນທາງການສົ່ງຂໍ້ມູນ (Upload):</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="171"/>
        <source>Track download:</source>
        <translation>ເສັ້ນທາງການອັບໂຫຼດຂໍ້ມູນ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="241"/>
        <source>The command that is used to upload tracks to the device</source>
        <translation type="unfinished">ຄຳສັ່ງທີ່ໃຊ້ສົ່ງຮ່ອງສຽງໄປຫາອຸປະກອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="213"/>
        <source>Track upload:</source>
        <translation type="unfinished">ການສົ່ງຮ່ອງສຽງ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="220"/>
        <source>The command that is used to download tracks from the device</source>
        <translation type="unfinished">ຄຳສັ່ງທີ່ໃຊ້ດາວໂຫຼດ່ງຮ່ອງສຽງໄປຫາອຸປະກອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="227"/>
        <source>The command that is used to upload routes to the device</source>
        <translation type="unfinished">ຄຳສັ່ງທີ່ໃຊ້ສົ່ງເສັ້ນທາງງໄປຫາອຸປະກອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="192"/>
        <source>The command that is used to download routes from the device</source>
        <translation type="unfinished">ຄຳສັ່ງທີ່ໃຊ້ດາວໂຫຼດເສັ້ນທາງງໄປຫາອຸປະກອນກອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="206"/>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>ຄຳສັ່ງທີ່ໃຊ້ອັບໂຫຼດ (upload waypoints)ໄປຫາອຸປະກອນກອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="234"/>
        <source>The command that is used to download waypoints from the device</source>
        <translation>ຄຳສັ່ງທີ່ໃຊ້ດາວໂຫຼດ (download waypoints)ໄປຫາອຸປະກອນກອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="266"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="93"/>
        <source>&amp;Gps Tools</source>
        <translation>ເ&amp;ຄື່ອງມີ GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="94"/>
        <source>&amp;Create new GPX layer</source>
        <translation>&amp;ສ້າງລະດັບຊັ້ນຂອງ GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="97"/>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>ສ້າງລະດັບຊັ້ນ GPX ໃຫມ່ແລະສະແດງມັນຢູ່ພື້ນແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="196"/>
        <source>&amp;Gps</source>
        <translation>&amp;GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="160"/>
        <source>Save new GPX file as...</source>
        <translation>ເກັບແຟ້ມ GPX ໃໝ່ໃຫ້ເປັນ...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="160"/>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>ແຟ້ມແລກປ່ຽນ GPS (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="165"/>
        <source>Could not create file</source>
        <translation>ບໍ່ສາມາດສ້າງແຟ້ມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="167"/>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>ບໍ່ສາມາດສ້າງແຟ້ມ GPX ກັບຊື່ທີ່ໃຫ້ໄວ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="168"/>
        <source>Try again with another name or in another </source>
        <translation>ພະຍາຍາມອີກເທື່ອນື່ງກັບອີກຊື່ນື່ງຫຼືໃນອື່ນໆ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="168"/>
        <source>directory.</source>
        <translation>ໄດເຣກທໍຣີ (Directory).</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="207"/>
        <source>GPX Loader</source>
        <translation>ຕົວໂຫຼດ GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="209"/>
        <source>Unable to read the selected file.
</source>
        <translation>ບໍ່ສາມາດອ່ານແຟ້ມທີ່ຖືກເລື້ອກໄວ້.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="209"/>
        <source>Please reselect a valid file.</source>
        <translation>ກະລຸນາເລື້ອກແຟ້ມທີ່ໃຊ້ການໄດ້ອີກໃໝ່.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="487"/>
        <source>Could not start process</source>
        <translation type="unfinished">ບໍສາມາດເລີ້ມລະບົບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="488"/>
        <source>Could not start GPSBabel!</source>
        <translation>ບໍສາມາດເລີ້ມ GPSBabel!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="330"/>
        <source>Importing data...</source>
        <translation>ນຳເຂົ້າຂໍ້ມູນ...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="493"/>
        <source>Cancel</source>
        <translation type="unfinished">ຍົກເລີກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="274"/>
        <source>Could not import data from %1!

</source>
        <translation>ບໍ່ສາມາດນຳເຂົ້າຂໍ້ມູນຈາກ %1!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="276"/>
        <source>Error importing data</source>
        <translation>ມີຂໍ້ຜິດພາດເກີດຂື້ນກັບການນຳຂໍ້ມູນເຂົ້າມາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="476"/>
        <source>Not supported</source>
        <translation type="unfinished">ບໍ່ເຂົ້າກັນກັບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="394"/>
        <source>This device does not support downloading </source>
        <translation type="unfinished">ອຸປະກອນນີ້ບໍເຂົ້າກັນກັບການດາວໂຫຼດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="394"/>
        <source>of </source>
        <translation type="unfinished">ຂອງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="410"/>
        <source>Downloading data...</source>
        <translation>ກຳລັງດາວໂຫຼດຂໍ້ມູນ...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="421"/>
        <source>Could not download data from GPS!

</source>
        <translation>ບໍ່ສາມາດດາວໂຫຼດເອົາຂໍ້ມູນຈາກ GPS!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="423"/>
        <source>Error downloading data</source>
        <translation type="unfinished">ມີຂໍ້ຜິດພາດເກີດຂື້ນໃນການດາວໂຫຼດຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="477"/>
        <source>This device does not support uploading of </source>
        <translation>ອຸປະກອນນີ້ບໍ່ເຂົ້າກັນກັບການນຳສົ່ງຂໍ້ມູນອອກອງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="493"/>
        <source>Uploading data...</source>
        <translation>ກຳລັງນຳສົ່ງຂໍ້ມູນອອກ...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="504"/>
        <source>Error while uploading data to GPS!

</source>
        <translation>ມີຂໍ້ຜິດພາດໃນຄະນະທີ່ນຳສົ່ງຂໍມູນໄປຫາ GPS!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="506"/>
        <source>Error uploading data</source>
        <translation>ມີຂໍ້ຜິດພາດໃນການນຳສົ່ງຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="342"/>
        <source>Could not convert data from %1!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="344"/>
        <source>Error converting data</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="522"/>
        <source>Choose a filename to save under</source>
        <translation type="unfinished">ເລື້ອກຊື່ແຟ້ມເພື່ອຈັດເກັບພາຍໃຕ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="524"/>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>ແຟ້ມແລກປ່ຽນ GPS (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="511"/>
        <source>Select GPX file</source>
        <translation>ເລື້ອກແຟ້ມ GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="250"/>
        <source>Select file and format to import</source>
        <translation>ເລື້ອກຮູບເເຟ້ມເເລະແບບການຈັດແຟ້ມທີ່ຈະເອົາເຂົ້າມາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="486"/>
        <source>Waypoints</source>
        <translation type="unfinished">ເວພ້ອຍທ໌ (Waypoints)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="486"/>
        <source>Routes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="272"/>
        <source>Tracks</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="422"/>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="422"/>
        <source>GPS eXchange file format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="423"/>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="435"/>
        <source>This tool will help you download data from a GPS device.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="436"/>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="455"/>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="457"/>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="492"/>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="453"/>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="454"/>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="472"/>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="474"/>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="475"/>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="491"/>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="493"/>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX filename that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="13"/>
        <source>GPS Tools</source>
        <translation type="unfinished">ເຄື່ອງມື GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="58"/>
        <source>Load GPX file</source>
        <translation>ໂຫຼດແຟ້ມ GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="94"/>
        <source>File:</source>
        <translation type="unfinished">ແຟ້ມ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="111"/>
        <source>Feature types:</source>
        <translation type="unfinished">ຊະນິດຈຸດເດັ່ນ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="314"/>
        <source>Waypoints</source>
        <translation>ເວພ້ອຍທ໌ (Waypoints)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="319"/>
        <source>Routes</source>
        <translation type="unfinished">ເສັ້ນທາງປະຈຳ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="324"/>
        <source>Tracks</source>
        <translation type="unfinished">ເສັ້ນທາງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="158"/>
        <source>Import other file</source>
        <translation type="unfinished">ນຳເຂົ້າແຟ້ມອື່ນໆ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="264"/>
        <source>File to import:</source>
        <translation type="unfinished">ແຟ້ມເພື່ອນຳເຂົ້າ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="339"/>
        <source>Feature type:</source>
        <translation type="unfinished">ຊະນິດຈຸດເດັ່ນ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="582"/>
        <source>GPX output file:</source>
        <translation>ແຟ້ມຂາອອກຂອງ GPX :</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="558"/>
        <source>Layer name:</source>
        <translation type="unfinished">ຊື່ລະດັບຊັ້ນ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="272"/>
        <source>Download from GPS</source>
        <translation>ດາວໂຫຼດຈາກ GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="508"/>
        <source>Edit devices</source>
        <translation type="unfinished">ແກ້ໄຂອຸປະກອນຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="515"/>
        <source>GPS device:</source>
        <translation>ອຸປະກອນຂອງ GPS:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="356"/>
        <source>Output file:</source>
        <translation type="unfinished">ແຟ້ມຂາອອກ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="481"/>
        <source>Port:</source>
        <translation>ພອດ (Port):</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="430"/>
        <source>Upload to GPS</source>
        <translation>ນຳສົ່ງຂໍ້ມູນຼດໄປຫາ GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="522"/>
        <source>Data layer:</source>
        <translation type="unfinished">ລະດັບຊັ້ນຂໍ້ມູນ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;GPX is the &lt;/span&gt;&lt;a href=&quot;http://www.topografix.com/gpx.asp&quot;&gt;&lt;span style=&quot; font-size:10pt; text-decoration: underline; color:#0000ff;&quot;&gt;GPS eXchange file format&lt;/span&gt;&lt;/a&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;, which is used to store information about waypoints, routes, and tracks.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Select a GPX file and then select the feature types that you want to load.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;GPX is the &lt;/span&gt;&lt;a href=&quot;http://www.topografix.com/gpx.asp&quot;&gt;&lt;span style=&quot; font-size:10pt; text-decoration: underline; color:#0000ff;&quot;&gt;GPS eXchange file format&lt;/span&gt;&lt;/a&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;, which is used to store information about waypoints, routes, and tracks.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Select a GPX file and then select the feature types that you want to load.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="572"/>
        <source>Browse...</source>
        <translation>ການເບີ່ງ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (&lt;/span&gt;&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; font-size:10pt; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;). This requires that you have GPSBabel installed where QGIS can find it.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer. All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (&lt;/span&gt;&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; font-size:10pt; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;). This requires that you have GPSBabel installed where QGIS can find it.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer. All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="565"/>
        <source>Save As...</source>
        <translation>ດເກັບເປັນ...</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This tool will help you download data from a GPS device. Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data. If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;This tool uses the program GPSBabel (&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;) to transfer the data. If you don&apos;t have GPSBabel installed where QGIS can find it, this tool will not work.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This tool will help you download data from a GPS device. Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data. If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;This tool uses the program GPSBabel (&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;) to transfer the data. If you don&apos;t have GPSBabel installed where QGIS can find it, this tool will not work.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This tool will help you upload data from a GPX layer to a GPS device. Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to. If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;This tool uses the program GPSBabel (&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;) to transfer the data. If you don&apos;t have GPSBabel installed where QGIS can find it, this tool will not work.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:12pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This tool will help you upload data from a GPX layer to a GPS device. Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to. If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;This tool uses the program GPSBabel (&lt;a href=&quot;http://gpsbabel.sf.net&quot;&gt;&lt;span style=&quot; text-decoration: underline; color:#0000ff;&quot;&gt;http://gpsbabel.sf.net&lt;/span&gt;&lt;/a&gt;) to transfer the data. If you don&apos;t have GPSBabel installed where QGIS can find it, this tool will not work.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="545"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="250"/>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="530"/>
        <source>GPX Conversions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="615"/>
        <source>Conversion:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="629"/>
        <source>GPX input file:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="70"/>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>URI ບໍ່ດີ-ທ່ານຕ້ອງລະບຸຊະນິດຂອງຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="114"/>
        <source>GPS eXchange file</source>
        <translation>ແຟ້ມແລກປ່ຽນ GPS</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="731"/>
        <source>Digitized in QGIS</source>
        <translation>ຈັດເກັບຂໍ້ມູນໃຫ້ເປັນດີຈີຕອນໃນ QGIS້</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Name</source>
        <translation type="obsolete">ຊື່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Type</source>
        <translation type="obsolete">ຊະນິດ</translation>
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
        <translation type="unfinished">ຊະນິດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="41"/>
        <source>Point</source>
        <translation type="unfinished">ຈຸດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="48"/>
        <source>Line</source>
        <translation type="unfinished">ເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="55"/>
        <source>Polygon</source>
        <translation type="unfinished">ຮູບຫຼາຍຫຼ່ຽມ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="13"/>
        <source>New Vector Layer</source>
        <translation type="unfinished">ລະດັບຊັ້ນທີ່ມີຕົວເລກບອກຂະໜາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Attributes:</source>
        <translation type="obsolete">ຄຸນລັກສະນະ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Add</source>
        <translation type="obsolete">ເພີ້ມ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Column 1</source>
        <translation type="obsolete">ຖັນ 1</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Remove</source>
        <translation type="obsolete">ເອົາອອກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>File Format:</source>
        <translation type="obsolete">ແຟ້ມ ຮູບແບບຂອງການຈັດແຟ້ມໃນລະບົບຄອມພີວເຕີ້:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="22"/>
        <source>File format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="65"/>
        <source>Attributes</source>
        <translation type="unfinished">ຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="150"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
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
        <translation>&amp;Georeferencer</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="85"/>
        <source>Choose a raster file</source>
        <translation>ເລື້ອກແຟ້ມ Raster</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="87"/>
        <source>Raster files (*.*)</source>
        <translation>ແຟ້ມ Raster(*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="97"/>
        <source>Error</source>
        <translation>ຜິ້ຜິດພາດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="98"/>
        <source>The selected file is not a valid raster file.</source>
        <translation>ແຟ້ມທີ່ຖືກເລື້ອກເປັນແຟ້ມ Raster ທີ່ໃຊ້ການບໍ່ໄດ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>You need to specify a file to georeference first.</source>
        <translation type="obsolete">ຕ້ອງລະບຸເເຟ້ມຕໍ່ georeference ກ່ອນ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="122"/>
        <source>World file exists</source>
        <translation>ໂລກ ແຟ້ມຍັງຄົງມີຢູ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="124"/>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation>&lt;p&gt;ແຟ້ມທີ່ເລື້ອກໄວແລ້ວເບິ່ງຄືມີ...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>world file! Do you want to replace it with the </source>
        <translation>ໂລກ ແຟ້ມ! ທ່ານຢາກປ່ຽນແທນມັນດ້ວຍ...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>new world file?&lt;/p&gt;</source>
        <translation>ໂລກ ແຟ້ມໃໝ່?&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="13"/>
        <source>Georeferencer</source>
        <translation>Georeferencer</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="100"/>
        <source>Close</source>
        <translation type="unfinished">ປິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="62"/>
        <source>Raster file:</source>
        <translation>ແຟ້ມ Raster:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Enter world coordinates</source>
        <translation type="obsolete">ໃສ່ຕົວປະສານໂລກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:16px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin can generate world files for rasters. You select points on the raster and give their world coordinates, and the plugin will compute the world file parameters. The more coordinates you can provide the better the result will be.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:16px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin can generate world files for rasters. You select points on the raster and give their world coordinates, and the plugin will compute the world file parameters. The more coordinates you can provide the better the result will be.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Browse...</source>
        <translation type="obsolete">ການເບີ່ງ...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="28"/>
        <source>Arrange plugin windows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="43"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
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
        <translation>ທາງເລື້ອກ Warp</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="35"/>
        <source>Resampling method:</source>
        <translation type="unfinished">ວິທີໃຫ້ຕົວຢ່າງຄືນໃໝ່:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="46"/>
        <source>Nearest neighbour</source>
        <translation type="unfinished">ເພື່ອນບ້ານໃກ້ຄຽງທີ່ສຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="51"/>
        <source>Linear</source>
        <translation type="unfinished">ເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="56"/>
        <source>Cubic</source>
        <translation type="unfinished">ແມັດກ້ອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="74"/>
        <source>OK</source>
        <translation type="unfinished">ຕົກລົງ </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="64"/>
        <source>Use 0 for transparency when needed</source>
        <translation>ໃຊ້ ໐ ສຳລັບ ຄວາມໂປ່ງໃສ (Transparency) ເມື່ອຕ້ອງການ</translation>
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
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="324"/>
        <source>Equal Interval</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="301"/>
        <source>Quantiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="350"/>
        <source>Empty</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="25"/>
        <source>graduated Symbol</source>
        <translation type="unfinished">ເຄື່ອງທີ່ຖືກຈັດລຳດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Classification Field:</source>
        <translation type="obsolete">ສະໜາມການຈັດໝວດໝູ່%</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Mode:</source>
        <translation type="obsolete">ແບບ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Number of Classes:</source>
        <translation type="obsolete">ຈຳນວນຊັ້ນຕ່າງໆ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="188"/>
        <source>Delete class</source>
        <translation>ລຶບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="181"/>
        <source>Classify</source>
        <translation type="unfinished">ຈັດໝວດໝູ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="55"/>
        <source>Classification field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="93"/>
        <source>Mode</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="152"/>
        <source>Column</source>
        <translation type="unfinished">ຖັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="153"/>
        <source>Value</source>
        <translation type="unfinished">ຄ່າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="154"/>
        <source>Type</source>
        <translation type="unfinished">ຊະນິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Field</source>
        <translation type="obsolete">ສະໜາມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="301"/>
        <source>ERROR</source>
        <translation>ຜິຜິດພາດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="303"/>
        <source>OK</source>
        <translation type="unfinished">ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="158"/>
        <source>Layer</source>
        <translation type="unfinished">ລະດັບຊັ້ນ</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="48"/>
        <source>GRASS Attributes</source>
        <translation>ຄຸນລັກສະນະຂອງ GRASS </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="78"/>
        <source>Tab 1</source>
        <translation type="unfinished">Tab 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="112"/>
        <source>result</source>
        <translation type="unfinished">ຜົນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="180"/>
        <source>Update</source>
        <translation type="unfinished">ຍົກລະດັບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="177"/>
        <source>Update database record</source>
        <translation type="unfinished">ຍົກລະດັບການບັນທຶກຖານຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="210"/>
        <source>New</source>
        <translation type="unfinished">ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="207"/>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>ເພີ້ມການຈັດໝວດໝູໃໝ່ຢູ່ໃນການຕັ້ງຄ້າ (Settings)ໃນ ດັດເເປງ ຕູ້ເຄື່ອງມື ຂອງໂປຼກຼາມ GRASS </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="240"/>
        <source>Delete</source>
        <translation>ລຶບບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="237"/>
        <source>Delete selected category</source>
        <translation type="unfinished">ລືບໝວດໝູ່ທີ່ເລື້ອກໄວ້ແລ້ວ</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="66"/>
        <source>Tools</source>
        <translation>ເຄື່ອງມື</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="71"/>
        <source>Add selected map to canvas</source>
        <translation type="unfinished">ເພີ້ມແຜນທີ່ໆຖືກເລື້ອກໄວ້ແລ້ວໄປໃສ່ພື້ນແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="79"/>
        <source>Copy selected map</source>
        <translation type="unfinished">ສຳເນົາແຜນທີ່ໆຖືກເລື້ອກໄວ້ແລ້ວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="87"/>
        <source>Rename selected map</source>
        <translation type="unfinished">ປ່ຽນຊື່ແຜນທີ່ໆຖືກເລື້ອກໄວ້ແລ້ວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="95"/>
        <source>Delete selected map</source>
        <translation type="unfinished">ໍລືບແຜນທີ່ໆຖືກໄວ້ແລ້ວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="103"/>
        <source>Set current region to selected map</source>
        <translation type="unfinished">ຕັ້ງຄ່າຂອບເຂດປັດຈຸບັນໄປໃສ່ແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="111"/>
        <source>Refresh</source>
        <translation type="unfinished">ກະຕຸ້ນໃຫ້ເຮັດວຽກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="453"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="289"/>
        <source>Cannot copy map </source>
        <translation type="unfinished">ບໍ່ສາມາດສຳເນົາແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="411"/>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt; ຄຳສັ່ງ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="355"/>
        <source>Cannot rename map </source>
        <translation type="unfinished">ບໍ່ສາມາດປ່ຽນຊື່ແຜນທີ່ໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="393"/>
        <source>Delete map &lt;b&gt;</source>
        <translation>ລຶບແຜນທີ່່ &lt;b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="410"/>
        <source>Cannot delete map </source>
        <translation type="unfinished">ບໍ່ສາມາດລືບແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="454"/>
        <source>Cannot write new region</source>
        <translation type="unfinished">ບໍ່ສາມາດຂຽນຂອບເຂດໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="339"/>
        <source>New name</source>
        <translation type="unfinished">ຊື່ໃຫມ່</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="217"/>
        <source>New point</source>
        <translation type="unfinished">ຈຸດໃຫມ່ 
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="235"/>
        <source>New centroid</source>
        <translation type="unfinished">ສູນກາງໄຫ່ມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="253"/>
        <source>Delete vertex</source>
        <translation>ລືບຈຸດລວມເສັ້ນ໌</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1821"/>
        <source>Left: </source>
        <translation type="unfinished">ຊ້າຍມື:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1822"/>
        <source>Middle: </source>
        <translation type="unfinished">ກາງ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="214"/>
        <source>Edit tools</source>
        <translation>ເຄ່ືອງມືແກ້ໄຂ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="223"/>
        <source>New line</source>
        <translation type="unfinished">ເສັ້ນໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="229"/>
        <source>New boundary</source>
        <translation type="unfinished">ເຂດແດນໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="241"/>
        <source>Move vertex</source>
        <translation type="unfinished">ຍ້າຍຈຸດລວມເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="247"/>
        <source>Add vertex</source>
        <translation type="unfinished">ເພີ້ມຈຸດລວມເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="259"/>
        <source>Move element</source>
        <translation type="unfinished">ຍ້າຍຖານ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="265"/>
        <source>Split line</source>
        <translation type="unfinished">ແຍ້ກເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="271"/>
        <source>Delete element</source>
        <translation>ລຶບຖານ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="277"/>
        <source>Edit attributes</source>
        <translation type="unfinished">ແກ້ໄຂຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="282"/>
        <source>Close</source>
        <translation type="unfinished">ປິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1458"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="201"/>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>ທ່ານບໍແມ່ນເຈົ້າຂອງຊຸດແຜນທີ່,ບໍ່ສາມາດເປີດຕົວເລກທີ່ບອກຄະໜາດແລະທິດທາງເພື່ອແກ້ໄຂ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="206"/>
        <source>Cannot open vector for update.</source>
        <translation>ໍບໍ່ສາມາດເປີດຕົວເລກທີ່ບອກຂະໜາດແລະທິດທາງເພື່ອຍົກລະດັບ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="665"/>
        <source>Info</source>
        <translation>ຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="665"/>
        <source>The table was created</source>
        <translation type="unfinished">ຕາຕະລາງຖືກສ້າງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1333"/>
        <source>Tool not yet implemented.</source>
        <translation>ເຄື່ອງມືຍັງໃຊ້ການບໍ່ໄດ້.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1357"/>
        <source>Cannot check orphan record: </source>
        <translation>ບໍ່ສາມາດກວດກາຂໍ້ມູນກໍາພາ້:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1364"/>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation>ຂໍ້ມູນກໍາພາ້ຢູ່ກັບຄຸນສົມບັດຕາຕະລາງ: &lt;br&gt;ລືບບໍ່?</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1373"/>
        <source>Cannot delete orphan record: </source>
        <translation>ບໍ່ສາມາດລຶບກາຂໍ້ມູນກໍາພາ້:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1401"/>
        <source>Cannot describe table for field </source>
        <translation type="unfinished">ບໍ່ສາມາດບັນຍາຍຕາຕະລາງສຳລັບສະໜາມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="363"/>
        <source>Background</source>
        <translation type="unfinished">ພື້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="364"/>
        <source>Highlight</source>
        <translation type="unfinished">ຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="365"/>
        <source>Dynamic</source>
        <translation type="unfinished">ການເຄື່ອນເໜັງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="366"/>
        <source>Point</source>
        <translation type="unfinished">ຈຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="367"/>
        <source>Line</source>
        <translation type="unfinished">ເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="368"/>
        <source>Boundary (no area)</source>
        <translation>ເຂດແດນ (ບໍ່ແມ່ນພື້ນທີ່)ໍ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="369"/>
        <source>Boundary (1 area)</source>
        <translation>ເຂດແດນ (1 ພື້ນທີ່)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="370"/>
        <source>Boundary (2 areas)</source>
        <translation>ເຂດແດນ(2 ພື້ນທີ່)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="371"/>
        <source>Centroid (in area)</source>
        <translation>ສູນກາງ (ໃນ ພື້ນທີ່)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="372"/>
        <source>Centroid (outside area)</source>
        <translation>ສູນກາງ (ນອກ ພື້ນທີ່)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="373"/>
        <source>Centroid (duplicate in area)</source>
        <translation>ສູນກາງ (ຊໍ້າ ໃນ ພື້ນທີ່)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="374"/>
        <source>Node (1 line)</source>
        <translation>ໂນດ (Node) (1 ເສັ້ນ)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="375"/>
        <source>Node (2 lines)</source>
        <translation>ໂນດ (Node) (2 ເສັ້ນ)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="408"/>
        <source>Disp</source>
        <comment>Column title</comment>
        <translation>ສະແດງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Color</source>
        <comment>









































Column title
</comment>
        <translation type="obsolete">ສີ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Type</source>
        <comment>









































Column title
</comment>
        <translation type="obsolete">ຊະນິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Index</source>
        <comment>









































Column title
</comment>
        <translation type="obsolete">ດັດສະນີ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="444"/>
        <source>Column</source>
        <translation type="unfinished">ຖັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="445"/>
        <source>Type</source>
        <translation type="unfinished">ຊະນິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="446"/>
        <source>Length</source>
        <translation type="unfinished">ຄວາມຍາວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="501"/>
        <source>Next not used</source>
        <translation type="unfinished">ຖັດໄປບໍ່ໄດ້ໃຊ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="502"/>
        <source>Manual entry</source>
        <translation type="unfinished">ເຂົ້າໂດຍເຮັດເອງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="503"/>
        <source>No category</source>
        <translation type="unfinished">ບໍ່ແມ່ນໝວດໝູ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1823"/>
        <source>Right: </source>
        <translation>ຂວາມື:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="410"/>
        <source>Color</source>
        <comment>Column title</comment>
        <translation type="unfinished">ສີ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="412"/>
        <source>Type</source>
        <comment>Column title</comment>
        <translation type="unfinished">ຊະນິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="414"/>
        <source>Index</source>
        <comment>Column title</comment>
        <translation type="unfinished">ດັດສະນີ</translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="16"/>
        <source>GRASS Edit</source>
        <translation type="unfinished">ແກ້ໄຂໂປຼກຼາມ GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="106"/>
        <source>Category</source>
        <translation type="unfinished">ໝວດໝູ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="66"/>
        <source>Mode</source>
        <translation type="unfinished">ແບບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Field (layer)</source>
        <translation type="obsolete">ສະໜາມ(ລະດັບຊັ້ນ)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="170"/>
        <source>Settings</source>
        <translation type="unfinished">ການຕັ້ງຄ່າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="190"/>
        <source>Snapping in screen pixels</source>
        <translation>ການຖ່າຍຮູບເປັນ Pixels</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="241"/>
        <source>Symbology</source>
        <translation>ເຄື່ອງຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="274"/>
        <source>Column 1</source>
        <translation type="unfinished">ຖັນ 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="360"/>
        <source>Table</source>
        <translation type="unfinished">ຕາຕະລາງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="496"/>
        <source>Add Column</source>
        <translation type="unfinished">ເພີ້ມຖັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="511"/>
        <source>Create / Alter Table</source>
        <translation>ສ້າງ/ແກ້ ໄຂແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="298"/>
        <source>Line width</source>
        <translation type="unfinished">ຄວາມກ້ວາງຂອງເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="325"/>
        <source>Marker size</source>
        <translation type="unfinished">ຄະໜາດຫຼັກແດນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="410"/>
        <source>Layer</source>
        <translation type="unfinished">ລະດັບຊັ້ນ</translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="132"/>
        <source>Cancel</source>
        <translation type="unfinished">ຍົກເລີກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="163"/>
        <source>Ok</source>
        <translation type="unfinished">ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="168"/>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;ສີແດງ&apos;&gt;ສີແດ!&lt;/font&gt; </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="179"/>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;ສີແດງ&apos;&gt;ນີ້ແມ່ນແຫຼ່ງທີ່ມາຊື່!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="185"/>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;ສີແດງ&apos;&gt;ບໍ່ມີ!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="186"/>
        <source>Overwrite</source>
        <translation type="unfinished">ຂຽນຖັບ</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="111"/>
        <source>Mapcalc tools</source>
        <translation type="unfinished">ເຄື່ອງມືຄິດໄລແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="114"/>
        <source>Add map</source>
        <translation type="unfinished">ເພີ້ມແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="121"/>
        <source>Add constant value</source>
        <translation type="unfinished">ເພີ້ມຄ່າຄົງທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="128"/>
        <source>Add operator or function</source>
        <translation type="unfinished">ເພີ້ມຕົວປະຕິບັດງານ ຫຼື ຄຳສັ່ງໃຊ້ງານ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="135"/>
        <source>Add connection</source>
        <translation type="unfinished">ເພີ້ມການເຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="142"/>
        <source>Select item</source>
        <translation>ເລື້ອກລາຍການ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="149"/>
        <source>Delete selected item</source>
        <translation>ລືບລາຍການທີ່ເລື້ອກໄວ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="159"/>
        <source>Open</source>
        <translation type="unfinished">ເປີດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="164"/>
        <source>Save</source>
        <translation>ດເກັບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="170"/>
        <source>Save as</source>
        <translation>ດເກັບເປັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="178"/>
        <source>Addition</source>
        <translation type="unfinished">ການເພີ້ມຕື່ມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="179"/>
        <source>Subtraction</source>
        <translation type="unfinished">ການຫັກລົບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="180"/>
        <source>Multiplication</source>
        <translation type="unfinished">ການຄູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="181"/>
        <source>Division</source>
        <translation type="unfinished">ການຫານ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="182"/>
        <source>Modulus</source>
        <translation>ຫຼັກເກນຂອງການຄູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="183"/>
        <source>Exponentiation</source>
        <translation>ການທະວີຄູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="186"/>
        <source>Equal</source>
        <translation type="unfinished">ເທົ່າກັບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="187"/>
        <source>Not equal</source>
        <translation type="unfinished">ບໍ່ເທົ່າກັບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="188"/>
        <source>Greater than</source>
        <translation type="unfinished">ໃຫຍ່ກວ່າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="189"/>
        <source>Greater than or equal</source>
        <translation type="unfinished">ໃຫຍ່ກວ່າຫຼືເທົ່າກັບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="190"/>
        <source>Less than</source>
        <translation type="unfinished">ນ້ອຍກວ່າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="191"/>
        <source>Less than or equal</source>
        <translation type="unfinished">ນ້ອຍກວ່າຫຼືເທົ່າກັບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="192"/>
        <source>And</source>
        <translation type="unfinished">ແລະ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="193"/>
        <source>Or</source>
        <translation type="unfinished">ຫຼື</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="196"/>
        <source>Absolute value of x</source>
        <translation>ຄ່າສົມບູນຂອງຄາ່ X</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="197"/>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>ເສັ້ນຊື່ກັບກັນຂອງແກ່ນ x (ຜົນອອກມາເປັນອົງສາ)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="198"/>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>ເສັ້ນຊື່ກັບກັນຂອງແກ່ນ y/x (ຜົນໄດ້ຮັບອອກມາເປັນອົງສາ)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="199"/>
        <source>Current column of moving window (starts with 1)</source>
        <translation>ຖັນປະຈຸບັນກຳລັງຍ້າຍ ວິນໂດ (ເລີ້ມຈາກຖັນ 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="200"/>
        <source>Cosine of x (x is in degrees)</source>
        <translation>C0sine ຂອງ x (x ເປັນອົງສາ)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="201"/>
        <source>Convert x to double-precision floating point</source>
        <translation>ປ່ຽນແກນ x ໄປຫາຈຸດລອຍຕົວທີ່ມີຄວາມທ່ຽງຕົງເພີ້ມຂ້ືນສອງເທົ່າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="202"/>
        <source>Current east-west resolution</source>
        <translation type="unfinished">ລາຍລະອຽດຂອງຮູບທາງທິດຕາເວັນອອກແລະຕາເວັນຕົກປະຈຸບັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="203"/>
        <source>Exponential function of x</source>
        <translation>ຄຳສັ່ງທະວີຄູນຂອງ x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="204"/>
        <source>x to the power y</source>
        <translation>x ເຖີງກໍາລັງ y</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="205"/>
        <source>Convert x to single-precision floating point</source>
        <translation>ປ່ຽນແກນ x ໄປຫາຈຸດລອຍຕົວທີ່ມີຄວາມທ່ຽງຕົງເພີ້ມຂ້ືນນື່ງເທົ່າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="206"/>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>ຕັດສີນໃຈ: 1 ຖາ້ວາ x ບໍ່ເເມນ,  ໐ ບໍ່ດັງນັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="207"/>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>ຕັດສີນໃຈ: a ຖາ້ວ່າ x ບໍ່ເເມນ ໐,໐ ບໍ່ດັງນັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="208"/>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>ຕັດສີນໃຈ:a ຖາ້ວ່າ x ບໍ່ເເມ່ນ ໐,b ບໍ່ດັງນັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="209"/>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>ຕັດສີນໃຈ:a ຖາ້ວ່າ x ໃຫຍ່ກວ່າ ໐ມນb ຖ້າຫາກວ່າ x ເປັນ ໐,c ຖ້າຫາກວ່າ x ນ້ອຍກວ່າ ໐ດັງນັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="210"/>
        <source>Convert x to integer [ truncates ]</source>
        <translation>ປ່ຽນ x ໃຫ້ເປັນຕົວເລກເຕັມ [ຕັດໃຫ້ສັ້ນລົງ]</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="211"/>
        <source>Check if x = NULL</source>
        <translation>ກວດສອບເບິ່ງູວ່າ ແກນ x ເທົ່າກັບສູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="212"/>
        <source>Natural log of x</source>
        <translation>ບົດບັນທຶກໂດຍທຳມະຊາດຂອງແກນ x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="213"/>
        <source>Log of x base b</source>
        <translation>ບົດບັນທຶກຂອງຖານ x ແລະ b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="215"/>
        <source>Largest value</source>
        <translation type="unfinished">ຄ່າໃຫຍ່ສຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="217"/>
        <source>Median value</source>
        <translation type="unfinished">ຄ່າປານກາງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="219"/>
        <source>Smallest value</source>
        <translation type="unfinished">ຄ່ານ້ອຍສຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="221"/>
        <source>Mode value</source>
        <translation>ຄ່າ ໂມດ (Mode)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="222"/>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1 ຖາ້ວາ x ເປັນ ໐ມນ,  ໐ ບໍ່ດັງນັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="223"/>
        <source>Current north-south resolution</source>
        <translation type="unfinished">ຄວາມລະອຽດຂອງຮູບທາງທິດໃຕ້-ເໜືອປະຈຸບັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="224"/>
        <source>NULL value</source>
        <translation type="unfinished">ຄ່າບໍ່ມີ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="225"/>
        <source>Random value between a and b</source>
        <translation>ຄ່າສົ່ງເດດລະຫວ່າງ a ແລະ b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="226"/>
        <source>Round x to nearest integer</source>
        <translation>x ເປັນຕົວເລກມົນໃກ້ຄຽງກັບເລກເຕັມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="227"/>
        <source>Current row of moving window (Starts with 1)</source>
        <translation>ແຖວປະຈຸບັນກຳລັງຍ້າຍວີນໂດ (ເລີ້ມດ້ວຍ 1)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Sine of x (x is in degrees)</source>
        <comment>






sin(x)</comment>
        <translation type="obsolete">ອັດຕາສ່ວນຂອງມຸມແຫຼມຕໍ່ມູມສາກຂອງແກນ x ເປັນອົງສາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="229"/>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>ຮາກຂັ້ນສອງຂອງ x</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Tangent of x (x is in degrees)</source>
        <comment>






tan(x)</comment>
        <translation type="obsolete">ເສັ້ນຊື່ຂອງແກນ x ( x ເປັນອົງສາ)້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="231"/>
        <source>Current x-coordinate of moving window</source>
        <translation>ຕົວປະສານປະຈຸບັນຂອງການຍ້າຍວີນໂດ </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="232"/>
        <source>Current y-coordinate of moving window</source>
        <translation>ຕົວປະສານປະຈຸບັນຂອງການຍ້າຍວີນໂດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1317"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="584"/>
        <source>Cannot get current region</source>
        <translation type="unfinished">ບໍ່ສາມາດໄດ້ຮັບຂົງເຂດປະຈຸບັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="561"/>
        <source>Cannot check region of map </source>
        <translation type="unfinished">ບໍ່ສາມາດກວດຂົງເຂດຂອງແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="617"/>
        <source>Cannot get region of map </source>
        <translation type="unfinished">ບໍ່ສາມາດໄດ້ຮັບຂົງເຂດຂອງແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="813"/>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>ບໍ່ມີແຜນທີ່ Rasters ຂອງໂປຼກຼາມ GRASS ຢູ່ໃນ QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1102"/>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>ບໍ່ສາມາດສ້າງໄດ້ເຣກທໍຣີ (Directory) ຂອງການຄຳນວນແຜນທີ່ໃໝ່ຢູ່ໃນຊຸດແຜນທີ່ປະຈຸບັນ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1112"/>
        <source>New mapcalc</source>
        <translation type="unfinished">ການຄຳນວນແຜນທີ່ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1113"/>
        <source>Enter new mapcalc name:</source>
        <translation type="unfinished">ໃສ່ຊື່ການຄຳນວນແຜນທີ່ໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1118"/>
        <source>Enter vector name</source>
        <translation type="unfinished">ໃສ່ຊື່ຕົວເລກບອກຄະໜາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1126"/>
        <source>The file already exists. Overwrite? </source>
        <translation type="unfinished">ແຟ້ມຍັງມີ່ຢູ່,ຈັດເກັບທັບຊື່ເກົ່າບໍ່?</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1164"/>
        <source>Save mapcalc</source>
        <translation type="unfinished">ຈັດເກັບການຄຳນວນແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1146"/>
        <source>File name empty</source>
        <translation type="unfinished">ຊື່ແຟ້ມເປົ່າວ່າງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1165"/>
        <source>Cannot open mapcalc file</source>
        <translation>ບໍ່ສາມາດເປີດແຟ້ມການຄຳນວນແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>The mapcalc schema (</source>
        <translation>ແຟ້ມການຄຳນວນແຜນທີ່ ແບບແຜນ (schema) (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>) not found.</source>
        <translation>) ບໍ່ພົບ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1302"/>
        <source>Cannot open mapcalc schema (</source>
        <translation>ບໍ່ສາມາດເປີດແຟ້ມການຄຳນວນແຜນທີ່ ແບບແຜນ (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1313"/>
        <source>Cannot read mapcalc schema (</source>
        <translation>ບໍ່ສາມາດອ່ານແຟ້ມການຄຳນວນແຜນທີ່ ແບບແຜນ (schema)(</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1314"/>
        <source>
at line </source>
        <translation type="unfinished">ຢູ່ເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1315"/>
        <source> column </source>
        <translation type="unfinished">ຖັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1388"/>
        <source>Output</source>
        <translation>ຜົນຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="228"/>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation type="unfinished">ອັດຕາສ່ວນຂອງມຸມແຫຼມຕໍ່ມູມສາກຂອງແກນ x ເປັນອົງສາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="230"/>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation type="unfinished">ເສັ້ນຊື່ຂອງແກນ x ( x ເປັນອົງສາ)້</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="16"/>
        <source>MainWindow</source>
        <translation type="unfinished">ວີນໂດ້ຫຼັກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="37"/>
        <source>Output</source>
        <translation>ຜົນຂໍ້ມູນ</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1380"/>
        <source>Run</source>
        <translation type="unfinished">ແລ່ນໂປຼກຼາມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1358"/>
        <source>Stop</source>
        <translation type="unfinished">ຢຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="205"/>
        <source>Module</source>
        <translation>ສ່ວນໂປຣກຣາມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1352"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="220"/>
        <source>The module file (</source>
        <translation>ສ່ວນໂປຣກຣາມແຟ້ມ (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="220"/>
        <source>) not found.</source>
        <translation>) ບໍ່ພົບ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="224"/>
        <source>Cannot open module file (</source>
        <translation>ບໍ່ສາມາດເປີດແຟ້ມ ສ່ວນໂປຣກຣາມ(</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="992"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="987"/>
        <source>Cannot read module file (</source>
        <translation>ບໍ່ສາມາດອ່ານແຟ້ມສ່ວນໂປຣກຣາມ (</translation>
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
        <translation type="unfinished">ຢູ່ເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="260"/>
        <source>Module </source>
        <translation>ສ່ວນໂປຣກຣາມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="260"/>
        <source> not found</source>
        <translation type="unfinished">ບໍ່ພົບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="301"/>
        <source>Cannot find man page </source>
        <translation>ບໍ່ສາມາດຊອກຫາແຟ້ມຊ່ວຍ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Not available, decription not found (</source>
        <translation type="obsolete">ບໍ່ມີ,ລາຍການບໍ່ພົບ (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="981"/>
        <source>Not available, cannot open description (</source>
        <translation>ບໍ່ມີ,ບໍ່ສາມາດເປີດລາຍການ (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="988"/>
        <source> column </source>
        <translation type="unfinished">ຖັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="992"/>
        <source>Not available, incorrect description (</source>
        <translation>ບໍ່ມີ,ລາຍການບໍ່ຖືກຕ້ອງ (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1179"/>
        <source>Cannot get input region</source>
        <translation type="unfinished">ບໍ່ສາມາດໄດ້ຮັບຂົງເຂດຂອງການປ້ອນຂໍມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1167"/>
        <source>Use Input Region</source>
        <translation type="unfinished">ນຳໃຊ້ຂົງເຂດການປ້ອນຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1281"/>
        <source>Cannot find module </source>
        <translation>ບໍ່ສາມາດຊອກຫາສ່ວນໂປຣກຣາມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1353"/>
        <source>Cannot start module: </source>
        <translation>ບໍ່ສາມາດເລີ້ມສ່ວນໂປຣກຣາມ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1369"/>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation>&lt;B&gt;ຈົບລົງຢ່າງປະສົບຜົນສຳເລັດ&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1375"/>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation>&lt;B&gt;ຈົບລົງດ້ວຍຂໍ້ຜິດພາດ&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1378"/>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation>&lt;B&gt;ເກນວັດແທກ)ແພຫຼືຖືກຂ້າ&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="978"/>
        <source>Not available, description not found (</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="13"/>
        <source>GRASS Module</source>
        <translation>ສ່ວນໂປຣກຣາມຂອງ GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="26"/>
        <source>Options</source>
        <translation type="unfinished">ທາງເລື້ອກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="31"/>
        <source>Output</source>
        <translation type="unfinished">ຂໍ້ມູນຂາອອກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="47"/>
        <source>Manual</source>
        <translation type="unfinished">ຄູ່ມື</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="118"/>
        <source>Run</source>
        <translation type="unfinished">ແລ່ນໂປຼກຼາມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="161"/>
        <source>Close</source>
        <translation type="unfinished">ປິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="141"/>
        <source>View output</source>
        <translation type="unfinished">ເບິ່ງຂໍມູນຂາອອກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="74"/>
        <source>TextLabel</source>
        <translation>ກາຫມາຍເອກະສານ</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2714"/>
        <source>Attribute field</source>
        <translation type="unfinished">ສະໜາມຄຸນລັກສະນະ</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2916"/>
        <source>File</source>
        <translation type="unfinished">ແຟ້ມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3029"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;ຄ່າຫາຍໄປ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3036"/>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation>:&amp;nbsp;ໄດເຣກທໍຣີ (Directory) ຍັງບໍ່ມີ</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2665"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2527"/>
        <source>Cannot find layeroption </source>
        <translation type="unfinished">ບໍ່ສາມາດພົບທາງເລື້ອກຂອງລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2669"/>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2692"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;ບໍ່ມີການປ້ອນຂໍ້ມູນ</translation>
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
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1984"/>
        <source>Cannot find typeoption </source>
        <translation type="unfinished">ບໍ່ສາມາດພົບທາງເລ້ືອກຂອງຊະນິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1993"/>
        <source>Cannot find values for typeoption </source>
        <translation type="unfinished">ບໍ່ສາມາດພົບຄ່າສຳລັບທາງເລື້ອກຂອງຊະນິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2054"/>
        <source>Cannot find layeroption </source>
        <translation type="unfinished">ບໍ່ສາມາດພົບທາງເລື້ອກຂອງລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source>GRASS element </source>
        <translation>ພື້ນຖານຂອງ GRASS </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source> not supported</source>
        <translation type="unfinished">ບໍ່ເຂົ້າກັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2095"/>
        <source>Use region of this map</source>
        <translation type="unfinished">ໃຊ້ຂົງເຂດຂອງແຜນທ່ີນີ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2431"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;ບໍ່ມີການປ້ອນຂໍ້ມູນ</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1900"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;ຄ່າຫາຍໄປ</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2803"/>
        <source>Attribute field</source>
        <translation type="unfinished">ສະໜາມຄຸນລັກສະນະ</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="886"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="369"/>
        <source>Cannot find module </source>
        <translation>ບໍ່ສາມາດພົບສ່ວນໂປຣກຣາມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="386"/>
        <source>Cannot start module </source>
        <translation>ບໍສາມາດເລີ້ມສ່ວນໂປຣກຣາມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="399"/>
        <source>Cannot read module description (</source>
        <translation>ບໍ່ສາມາດອ່ານລາຍການຂອງສ່ວນໂປຣກຣາມ (</translation>
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
        <translation type="unfinished">ຢູ່ເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="400"/>
        <source> column </source>
        <translation type="unfinished">ຖັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="424"/>
        <source>Cannot find key </source>
        <translation type="unfinished">ບໍ່ພົບກຸນແຈ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="560"/>
        <source>Item with id </source>
        <translation>ລາຍການ ກັບ ID </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="560"/>
        <source> not found</source>
        <translation type="unfinished">ບໍ່ພົບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="848"/>
        <source>Cannot get current region</source>
        <translation type="unfinished">ບໍ່ສາມາດໄດ້ຮັບຂົງເຂດປະຈຸບັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="825"/>
        <source>Cannot check region of map </source>
        <translation type="unfinished">ບໍ່ສາມາດກວດຂົງເຂດຂອງແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="887"/>
        <source>Cannot set region of map </source>
        <translation type="unfinished">ບໍ່ສາມາດຕັ້ງຂົງເຂດຂອງແຜນທີ່</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="124"/>
        <source>GRASS database</source>
        <translation>ຖານຂໍ້ມູນຂອງ GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="125"/>
        <source>GRASS location</source>
        <translation>ທີ່ຕັ້ງຂອງ GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="126"/>
        <source>Projection</source>
        <translation>ຄ້າດຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="127"/>
        <source>Default GRASS Region</source>
        <translation>ຂົງເຂດຂອງ GRASS ຕາຍຕົວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="184"/>
        <source>Mapset</source>
        <translation type="unfinished">ຊຸດແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="129"/>
        <source>Create New Mapset</source>
        <translation type="unfinished">ສ້າງຊຸດແຜນທີ່ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="158"/>
        <source>Tree</source>
        <translation>ຕົ້ນໄມ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="159"/>
        <source>Comment</source>
        <translation type="unfinished">ຄຳເຫັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="160"/>
        <source>Database</source>
        <translation type="unfinished">ຖານຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="164"/>
        <source>Location 2</source>
        <translation>ທີ່ຕັ້ງ 2</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="175"/>
        <source>User&apos;s mapset</source>
        <translation type="unfinished">ຊຸດແຜນທີ່ຂອງຜູ້ໃຊ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="177"/>
        <source>System mapset</source>
        <translation type="unfinished">ລະບົບຊຸດແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="171"/>
        <source>Location 1</source>
        <translation type="unfinished">ທີ່ຕັ້ງ1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="185"/>
        <source>Owner</source>
        <translation type="unfinished">ເຈົ້າຂອງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="233"/>
        <source>Enter path to GRASS database</source>
        <translation>ໃສ່ເສັ້ນທາງໄປຫາຖານຂໍ້ມູນຂອງ GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="241"/>
        <source>The directory doesn&apos;t exist!</source>
        <translation>ໄດເຣກທໍຣີ (Directory) ຍັງມີຢູ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="271"/>
        <source>No writable locations, the database not writable!</source>
        <translation type="unfinished">ທີ່ຕັ້ງຂຽນນບໍ່ໄດ້,ຖານຂໍ້ມູນຂຽນບໍ່ໄດ້!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="369"/>
        <source>Enter location name!</source>
        <translation>ໃສ່ຊື່ທີ່ຕັ້ງ!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="382"/>
        <source>The location exists!</source>
        <translation>ທີ່ຕັ້ງຍັງຄົງມີຢູ່!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="533"/>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>ການຄາດຫມາຍທີຖຶກເລືອກໃຊ້ບໍ່ໃດ້ກັບ GRASS!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1160"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="580"/>
        <source>Cannot create projection.</source>
        <translation>ບໍ່ສາມາດສ້າງການຄາດຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="629"/>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation>ບໍ່ສາມາດສາຍພາບຊຸດຂົງເຂດທີ່ຜ່ານມາໄດ້,ຊຸດຂົງເຂດຕາຍຕົວ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="764"/>
        <source>North must be greater than south</source>
        <translation type="unfinished">ທິດເຫນືອຕ້ອງໃຫຍ່ກວ່າທິດໃຕ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="769"/>
        <source>East must be greater than west</source>
        <translation type="unfinished">ທິດຕາເວັນອອກຕ້ອງໃຫຍ່ກວ່າທິດຕາເວັນຕົກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="816"/>
        <source>Regions file (</source>
        <translation>ແຟ້ມຂົງເຂດ  (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="816"/>
        <source>) not found.</source>
        <translation>) ບໍ່ພົບ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="821"/>
        <source>Cannot open locations file (</source>
        <translation>ບໍ່ສາມາດເປີດແຟ້ມທີ່ຕັ້ງໄດ້ (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="821"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="830"/>
        <source>Cannot read locations file (</source>
        <translation>ບໍ່ສາມາດອ້ານແຟ້ມທີ່ຕັ້ງໄດ້ (</translation>
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
        <translation type="unfinished">ຢູ່ເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="832"/>
        <source> column </source>
        <translation type="unfinished">ຖັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1161"/>
        <source>Cannot create QgsSpatialRefSys</source>
        <translation>ບໍ່ສາມາດສ້າງ QgsSpatialRefSys</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="968"/>
        <source>Cannot reproject selected region.</source>
        <translation>ບໍ່ສາມາດສາຍພາບຂົງເຂດທີ່ເລື້ອກໄວ້ຄືນໃໝ່ໄດ້.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1057"/>
        <source>Cannot reproject region</source>
        <translation type="unfinished">ບໍ່ສາມາດສາຍພາບຂົງເຂດຄືນໃໝ່ໄດ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1289"/>
        <source>Enter mapset name.</source>
        <translation>ໃສ່ຊື່ຊຸດແຜນທີ່.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1306"/>
        <source>The mapset already exists</source>
        <translation type="unfinished">ຊຸດແຜນທີ່ມີຢູ່ແລ້ວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1330"/>
        <source>Database: </source>
        <translation>ຖານຂໍ້ມູນ: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1341"/>
        <source>Location: </source>
        <translation>ທີ່ຕັ້ງ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1343"/>
        <source>Mapset: </source>
        <translation>ຊຸດແຜນທີ່:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1378"/>
        <source>Create location</source>
        <translation type="unfinished">ສ້າງທີ່ຕັ້ງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1380"/>
        <source>Cannot create new location: </source>
        <translation>ບໍ່ສາມາດສ້າງທີ່ຕັ້ງໃໝ່:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1427"/>
        <source>Create mapset</source>
        <translation type="unfinished">ສ້າງຊຸດແຜນທີ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cannot create new mapset dircetory</source>
        <translation type="obsolete">ບໍ່ສາມາດສ້າງໄດເຣກທໍຣີ (Directory) ຂອງຊຸດແຜນທີ່ໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1420"/>
        <source>Cannot open DEFAULT_WIND</source>
        <translation>ບໍ່ສາມາດເປີດ DEFAULT_WIND</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1427"/>
        <source>Cannot open WIND</source>
        <translation>ບໍ່ສາມາດເປີດ WIND</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1454"/>
        <source>New mapset</source>
        <translation type="unfinished">ຊຸດແຜນທີ່ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1450"/>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>ຊຸດແຜນທີ່ໃຫມ່ຖືກສ້າງສຳເລັດ, ແຕ່ບໍ່ສາມາດເປີດໃຊ້ໄດ້ </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1456"/>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>ຊຸດແຜນທີ່ໃຫມ່ຖືກສ້າງສຳເລັດແລະຕັ້ງໃຫ້ເປັນຊຸດແຜນທີ່ໆໃຊ້ງານປະຈຸບັນ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1410"/>
        <source>Cannot create new mapset directory</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2068"/>
        <source>Column 1</source>
        <translation type="unfinished">ຖັນ 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="88"/>
        <source>Example directory tree:</source>
        <translation>ໂຕຢ່າງ ໄດເເລກຕໍລີຮູບຮ່າງຕົ້ນໄມ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="95"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS data are stored in tree directory structure.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS data are stored in tree directory structure.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="399"/>
        <source>Database Error</source>
        <translation type="unfinished">ມີຂໍ້ຜິດພາດເກີຂື້ນກັບຖານຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2153"/>
        <source>Database:</source>
        <translation type="unfinished">ຖານຂໍ້ມູນ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="440"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="457"/>
        <source>Select existing directory or create a new one:</source>
        <translation>ເລື້ອກໄດເຣກທໍຣີ (Directory) ທີ່ມີຢູ່ແລ້ວຫຼືສ້າງຕົວໃໝ່:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="508"/>
        <source>Location</source>
        <translation type="unfinished">ທີ່ຕັ້ງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="535"/>
        <source>Select location</source>
        <translation type="unfinished">ເລື້ອກທີ່ຕັ້ງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="552"/>
        <source>Create new location</source>
        <translation type="unfinished">ສ້າງທີ່ຕັ້ງໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="832"/>
        <source>Location Error</source>
        <translation type="unfinished">ມີຂໍ້ຜິດພາດເກີດຂື້ນກັບທີ່ຕັ້ງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="848"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt; GRASS&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1159"/>
        <source>Projection Error</source>
        <translation>ການຄາດຫມາຍຜິດພາດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1174"/>
        <source>Coordinate system</source>
        <translation type="unfinished">ລະບົບປະສານງານ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1186"/>
        <source>Projection</source>
        <translation>ການຄາດຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1193"/>
        <source>Not defined</source>
        <translation type="unfinished">ບໍ່ໄດ້ກຳນົດຄວາມໝາຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1273"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1334"/>
        <source>Set current QGIS extent</source>
        <translation>ຕັ້ງຂອບເຂດປະຈຸບັນຂອງ QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1376"/>
        <source>Set</source>
        <translation>ຕັ້ງຄ່າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1396"/>
        <source>Region Error</source>
        <translation type="unfinished">ມີຂໍ້ຜິດພາດເກີດຂື້ນກັບຂອບເຂດ</translation>
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
        <translation type="unfinished">ຊຸດແຜນທີ່ໃຫມ່:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1988"/>
        <source>Mapset Error</source>
        <translation type="unfinished">ມີຂໍ້ຜິດພາດເກີດຂື້ນກັບຊຸດແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2045"/>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;ສູນກາງ&quot;&gt;masets ມີເເລ້ວ&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2101"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS mapset is a collection of maps used by one user. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A user can read maps from all mapsets in the location but &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS mapset is a collection of maps used by one user. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A user can read maps from all mapsets in the location but &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2174"/>
        <source>Location:</source>
        <translation type="unfinished">ທີ່ຕັ້ງ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2195"/>
        <source>Mapset:</source>
        <translation type="unfinished">ຊຸດແຜນທີ່:</translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="198"/>
        <source>GRASS</source>
        <translation type="unfinished">GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="788"/>
        <source>&amp;GRASS</source>
        <translation type="unfinished">&amp;GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="145"/>
        <source>Open mapset</source>
        <translation type="unfinished">ເປີດຊຸດແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="146"/>
        <source>New mapset</source>
        <translation type="unfinished">ຊຸດແຜນທີ່ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="147"/>
        <source>Close mapset</source>
        <translation type="unfinished">ປິດຊຸດແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="150"/>
        <source>Add GRASS vector layer</source>
        <translation>ເພີ້ມລະດັບຊັ້ນຂອງຕົວເລກບອກຄະໜາດແລະທິດທາງຂອງ GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="152"/>
        <source>Add GRASS raster layer</source>
        <translation>ເພີ້ມລະດັບຊັ້ນຂອງ GRASS Raster</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="168"/>
        <source>Open GRASS tools</source>
        <translation>ເປີດເຄື່ອງມື GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="157"/>
        <source>Display Current Grass Region</source>
        <translation>ສະແດງຂົງເຂດຂອງ Grass ປະຈຸບັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="161"/>
        <source>Edit Current Grass Region</source>
        <translation>ແກ້ໄຂຂົງເຂດຂອງ Grass ປະຈຸບັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="163"/>
        <source>Edit Grass Vector layer</source>
        <translation>ແກ້ໄຂລະດັບຊັ້ນຂອງ Grass ທີ່ບອກຕົວເລກຂະໜາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="166"/>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>ເພີ້ມລະດັບຊັ້ນຂອງ Grass ທີ່ບອກຕົວເລກຂະໜາດແລະທິດທາງໃສ່ພື້ນແຜນທີ່ງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="167"/>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>ເພີ້ມລະດັບຊັ້ນຂອງ Grass Rasterດທາງໃສ່ພື້ນແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="169"/>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>ສະແດງຂົງເຂດປະຈຸບັນຂອງ GRASS ໃຫ້ເປັນຮູບສີຫຼ່ຽມສາກຢູ່ເທີງພື້ນແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="170"/>
        <source>Edit the current GRASS region</source>
        <translation>ແກ້ໄຂຂົງເຂດຂອງ GRASS ປະຈຸບັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="171"/>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>ແກ້ໄຂ້ລະດັບຊັ້ນຂອງ Grass ທີ່ບອກຕົວເລກຂະໜາດແລະທິດທາງນທີ່ງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="82"/>
        <source>GrassVector</source>
        <translation>ຕົວເລກບອກຄະໜາດແລະທິດທາງຂອງ Grass</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="83"/>
        <source>0.1</source>
        <translation type="unfinished">0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="84"/>
        <source>GRASS layer</source>
        <translation>ລະດັບຊັ້ນ GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="164"/>
        <source>Create new Grass Vector</source>
        <translation>ສ້າງເສັ້ນ Grass ທີ່ບອກຄະຫນາດແລະທິດທາງໃຫມ່.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="758"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="470"/>
        <source>GRASS Edit is already running.</source>
        <translation>ການແກ້ໄຂ GRASS ກຳລັງເຮັດວຽກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="479"/>
        <source>New vector name</source>
        <translation>ຊື່ຂອງຕົວເລກບອກຄະໜາດແລະທິດທາງໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="495"/>
        <source>Cannot create new vector: </source>
        <translation>ບໍ່ສາມາດສ້າງຕົວເລກບອກຄະໜາດແລະທິດທາງໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="515"/>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation>ຕົວເລກບອກຄະໜາດແລະທິດທາງໃຫມ່ຖືກສ້າງຂື້ນແຕ່ບໍ່ສາມາດເປີດນຳໃຊ້ໄດ້ໂດຍຜູ້ສະໜອງຂໍ້ມູນ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="526"/>
        <source>Cannot start editing.</source>
        <translation>ບໍ່ສາມາດເລີ້ມການແກ້ໄຂ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="561"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>ຖານລະບົບ GISDBASE,LOCATION_NAME ຫລື MAPSET ຊື່ທີ່ຕັ້ງຫຼືຊຸດແຜນທີ່ບໍ່ໄດ້ຕັ້ງຄ່າ,ສະນັ້ນຈຶ່ງບໍ່ສາມາດສະແດງຂົງເຂດປະຈຸບັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="571"/>
        <source>Cannot read current region: </source>
        <translation>ບໍ່ສາມາດອ່ານຂົງເຂດປະຈຸບັນ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="675"/>
        <source>Cannot open the mapset. </source>
        <translation>ບໍ່ສາມາດເປີດຊຸດແຜນທີ່.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="693"/>
        <source>Cannot close mapset. </source>
        <translation>ບໍ່ສາມາດປິດຊຸດແຜນທີ່.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="749"/>
        <source>Cannot close current mapset. </source>
        <translation>ບໍ່ສາມາດປິດຊຸດແຜນທີ່ປະຈຸບັນ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="758"/>
        <source>Cannot open GRASS mapset. </source>
        <translation>ບໍ່ສາມາດເປີດຊຸດແຜນທີ່ຂອງ GRASS.</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="459"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="195"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>ຖານລະບົບ GISDBASE, LOCATION_NAME ຫລື MAPSET ຊື່ທີ່ຕັ້ງຫຼືຊຸດແຜນທີ່ບໍ່ໄດ້ຕັ້ງຄ່າ,ສະນັ້ນຈຶ່ງບໍ່ສາມາດສະແດງຂົງເຂດປະຈຸບັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="202"/>
        <source>Cannot read current region: </source>
        <translation>ບໍ່ສາມາດອ່ານຂົງເຂດປະຈຸບັນ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="459"/>
        <source>Cannot write region</source>
        <translation type="unfinished">ບໍ່ສາມາດຂຽນຂົງເຂດ</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="13"/>
        <source>GRASS Region Settings</source>
        <translation>ການຕ້ັງຄ່າຂົງເຂດຂອງ GRASS </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="76"/>
        <source>N</source>
        <translation type="unfinished">N</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="146"/>
        <source>W</source>
        <translation type="unfinished">W</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="172"/>
        <source>E</source>
        <translation type="unfinished">E</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="236"/>
        <source>S</source>
        <translation type="unfinished">S</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="280"/>
        <source>N-S Res</source>
        <translation type="unfinished">N-S Res</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="293"/>
        <source>Rows</source>
        <translation type="unfinished">ແຖວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="303"/>
        <source>Cols</source>
        <translation type="unfinished">ຖັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="316"/>
        <source>E-W Res</source>
        <translation type="unfinished">E-W Res</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="364"/>
        <source>Color</source>
        <translation>ສີ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="384"/>
        <source>Width</source>
        <translation type="unfinished">ຄວາມກ້ວາງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="464"/>
        <source>OK</source>
        <translation type="unfinished">ຕົກລົງ </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="487"/>
        <source>Cancel</source>
        <translation type="unfinished">ຍົກເລີກ</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="68"/>
        <source>Select GRASS Vector Layer</source>
        <translation>ເລື້ອກລະດັບຊັ້ນຕົວເລກທີ່ບອກຄະໜາດແລະທິດທາງຂອງ GRASS </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="75"/>
        <source>Select GRASS Raster Layer</source>
        <translation>ເລື້ອກລະດັບຊັ້ນ GRASS Raster</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="82"/>
        <source>Select GRASS mapcalc schema</source>
        <translation>ເລື້ອກ GRASS ແບບແຜນ (schema)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="90"/>
        <source>Select GRASS Mapset</source>
        <translation>ເລື້ອກລະດັບຂອງຊຸດແຜນທີ່ (GRASS Mapset)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="409"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="409"/>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation>ບໍ່ສາມາດເປີດຕົວເລກທີ່ບອກຄະໜາດແລະທິດທາງບົນລະດັບ (Topology ບໍ່ມີ)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="466"/>
        <source>Choose existing GISDBASE</source>
        <translation>ເລື້ອກຖານຂໍ້ມູນຂອງ GIS ທີ່ມີຢູ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="482"/>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>ຖານຂໍ້ມູນຂອງ GIS ຜິດອັນ.ບໍ່ມີທີ່ຕັ້ງ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="483"/>
        <source>Wrong GISDBASE</source>
        <translation>ຖານຂໍ້ມູນຂອງ GISDBASE ຜິດອັນ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="500"/>
        <source>Select a map.</source>
        <translation>ເລື້ອກແຜນທີ່.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="501"/>
        <source>No map</source>
        <translation>ບໍ່ມີແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="509"/>
        <source>No layer</source>
        <translation>ບໍ່ມີລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="510"/>
        <source>No layers available in this map</source>
        <translation>ບໍ່ມີລະດັບຊັ້ນຢູ່ໃນແຜນທີ່ນີ້ຂົ້າ</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="65"/>
        <source>Gisdbase</source>
        <translation type="unfinished">Gisdbase</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="78"/>
        <source>Location</source>
        <translation>ທີ່ຕັ້ງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="161"/>
        <source>Browse</source>
        <translation>ການເບິ່ງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="85"/>
        <source>Mapset</source>
        <translation>ຊຸດແຜນທີ່ (Mapset)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="118"/>
        <source>Map name</source>
        <translation>ຊື່ແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="125"/>
        <source>Layer</source>
        <translation>ລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="175"/>
        <source>OK</source>
        <translation type="unfinished">ຕົກລົງ </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="102"/>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>້ເລື້ອກຫຼືພີມຊື່ແຜນ (ໃສ່ເຄື່ອງໝາຍ &quot;*&quot; ແລະ &quot;?&quot; ຍອມຮັບໄດ້ສຳລັບ Raster)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="21"/>
        <source>Add GRASS Layer</source>
        <translation>ເພີ້ມລະດັບຊັ້ນຂອງ GRASS </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="168"/>
        <source>Cancel</source>
        <translation type="unfinished">ຍົກເລີກ</translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="19"/>
        <source>GRASS Shell</source>
        <translation>GRASS Shell</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="49"/>
        <source>Close</source>
        <translation type="unfinished">ປິດ</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="146"/>
        <source>Modules</source>
        <translation>ສ່ວນໂປຣກຣາມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="205"/>
        <source>Browser</source>
        <translation>ໂປຣກຣາມບາວເຊີ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="123"/>
        <source>GRASS Tools</source>
        <translation>ເຄື່ອງມື GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="470"/>
        <source>GRASS Tools: </source>
        <translation>ເຄື່ອງມື GRASS:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="358"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="271"/>
        <source>Cannot find MSYS (</source>
        <translation>ບໍ່ສາມາດຊອກຫາ MSYS (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="293"/>
        <source>GRASS Shell is not compiled.</source>
        <translation>ບໍ່ສາມາດຮຽບຮຽງ GRASS ກາບໃດ້.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="343"/>
        <source>The config file (</source>
        <translation>ເເຟ້ມປັບປຸງ (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="343"/>
        <source>) not found.</source>
        <translation>) ບໍ່ພົບ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="347"/>
        <source>Cannot open config file (</source>
        <translation>ບໍ່ສາມາດເປີດ ເເຟ້ມປັບປຸງ (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="347"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="355"/>
        <source>Cannot read config file (</source>
        <translation>ບໍ່ສາມາດອ່ານເເຟ້ມປັບປຸງ (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="356"/>
        <source>
at line </source>
        <translation type="unfinished">ຢູ່ເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="356"/>
        <source> column </source>
        <translation>ເເຖວຕັ້ງ</translation>
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
        <translation>c</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="94"/>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation>ສ້າງ Graticule (grid) ເເລະ ຮັກສາຜົນເເບບ shapefile </translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="136"/>
        <source>&amp;Graticules</source>
        <translation>&amp;Graticules</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="51"/>
        <source>QGIS - Grid Maker</source>
        <translation>ຜູ້ສ້າງ QGIS - Grid</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Longitude Interval is invalid - please correct and try again.</source>
        <translation type="obsolete">ເສັ້ນເເວງ ໄລຍະ ເເມ່ນຜິດພາດ - ເຮັດອິກເທື່ອ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Latitude Interval is invalid - please correct and try again.</source>
        <translation type="obsolete">ເສັ້ນຂະຫນານ ໄລຍະ ເເມ່ນຜິດພາດ - ເຮັດອິກເທື່ອ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Longitude Origin is invalid - please correct and try again..</source>
        <translation type="obsolete">ຈຸດເລີ້ມຕົ້ນ ເສັ້ນເເວງ ເເມ່ນຜິດພາດ - ເຮັດອິກເທື່ອ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Latitude Origin is invalid - please correct and try again.</source>
        <translation type="obsolete">ຈຸດເລີ້ມຕົ້ນ ເສັ້ນຂະຫນານ ເເມ່ນຜິດພາດ - ເຮັດອິກເທື່ອ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>End Point Longitude is invalid - please correct and try again.</source>
        <translation type="obsolete">ຈຸດສູດ ເສັ້ນເເວງ ເເມ່ນຜິດພາດ - ເຮັດອິກເທື່ອ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>End Point Latitude is invalid - please correct and try again.</source>
        <translation type="obsolete">ຈຸດສູດ ເສັ້ນຂະຫນານ  ເເມ່ນຜິດພາດ - ເຮັດອິກເທື່ອ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="108"/>
        <source>Choose a filename to save under</source>
        <translation type="unfinished">ເລື້ອກຊື່ແຟ້ມເພື່ອຈັດເກັບພາຍໃຕ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="110"/>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>ESRI Shapefile (*.shp)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="52"/>
        <source>Please enter the file name before pressing OK!</source>
        <translation>ກະລູນາລົວຊື່ເເຟ້ນກ່ອນຂະກົດປຸມ ຕົກລົງ!</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>QGIS Plugin Template</source>
        <translation type="obsolete">QGIS Plugin ໂຕຢ່າງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="256"/>
        <source>Graticule Builder</source>
        <translation>ໂຕສ້າງ Graticule </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>#000.00000; </source>
        <translation type="obsolete">#000.00000; </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Latitude:</source>
        <translation type="obsolete">ເສັ້ນຂະຫນານ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Longitude:</source>
        <translation type="obsolete">ເສັ້ນເເວງ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Latitude Interval:</source>
        <translation type="obsolete">ເສັ້ນຂະຫນານ ໄລຍະ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Longitude Interval:</source>
        <translation type="obsolete">ເສັ້ນເເວງ ໄລຍະ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:10pt;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:10pt;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="186"/>
        <source>Type</source>
        <translation>ຊະນິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="198"/>
        <source>Point</source>
        <translation>ຈຸດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Line</source>
        <translation type="obsolete">ເເຖວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="208"/>
        <source>Polygon</source>
        <translation>ຮູບຫຼາຍຫຼ່ຽມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="110"/>
        <source>Origin (lower left)</source>
        <translation>ຈຸດເລີ້ມຕົ້ນ (ລຸມສຸດ)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="69"/>
        <source>End point (upper right)</source>
        <translation>ຈຸດສູດ ( ຂວາເທີງ)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Graticle size (units in degrees)</source>
        <translation type="obsolete">Graticle ຂະຫນາດ (ຫນ່ວຍວັດເເທກ ເປັນ ອົງສາ)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="161"/>
        <source>Output (shape) file</source>
        <translation>ຜົນ (shape) ເເຟ້ມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="176"/>
        <source>Save As...</source>
        <translation>ເກັບເປັນ...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="13"/>
        <source>QGIS Graticule Creator</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="28"/>
        <source>Graticle size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="46"/>
        <source>Y Interval:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="56"/>
        <source>X Interval:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="128"/>
        <source>Y</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="138"/>
        <source>X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="221"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="185"/>
        <source>Quantum GIS Help - </source>
        <translation>Quantum GIS ຊ່ວຍ-</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="191"/>
        <source>Failed to get the help text from the database</source>
        <translation>ຜິດພາດຕອນເອົາຂໍ້ມູນຊ່ວຍເຫລືອຈາກຖານຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="214"/>
        <source>Error</source>
        <translation>ຜິ້ຜິດພາດ</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="215"/>
        <source>The QGIS help database is not installed</source>
        <translation>ຖານຂໍ້ມູນ QGIS ຊ່ວຍ ຍັງບໍ່ໃດ້ລົງ</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="139"/>
        <source>This help file does not exist for your language</source>
        <translation>ຂໍ່ມູນ ຊ່ວຍ ຍັງບໍ່ມີໃນພາສາເຂົ້າ</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="142"/>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation>ຖາ້ຢາກພັດທະນາ, ກາລຸນາຕິດຕໍກັບທີມງານ QGIS </translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="157"/>
        <source>Quantum GIS Help</source>
        <translation>Quantum GIS ຊ່ວຍເຫຼືອ</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="13"/>
        <source>QGIS Help</source>
        <translation>QGIS ຊ່ວຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="39"/>
        <source>&amp;Home</source>
        <translation>&amp;ຫນ້າຫຼັກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="42"/>
        <source>Alt+H</source>
        <translation type="unfinished">Alt+H</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="52"/>
        <source>&amp;Forward</source>
        <translation>&amp;ເດີນໜ້າ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="55"/>
        <source>Alt+F</source>
        <translation type="unfinished">Alt+F</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="65"/>
        <source>&amp;Back</source>
        <translation>&amp;ກັບຫຼັງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="68"/>
        <source>Alt+B</source>
        <translation type="unfinished">Alt+B</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="78"/>
        <source>&amp;Close</source>
        <translation>&amp;ປິດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="81"/>
        <source>Alt+C</source>
        <translation type="unfinished">Alt+C</translation>
    </message>
</context>
<context>
    <name>QgsHttpTransaction</name>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="234"/>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation>WMS Server ຕອບກັບເເບບບໍ່ຄາດຄະເນ ດ້ວຍມີ  HTTP Status Code %1 (%2)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="313"/>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>ການຕອບສະໜອງຂອງ HTTP ສົມບູນ,ແນວໃດກໍ່ຕາມຍັງມີຂໍ້ຜິດພາດເກີດຂື້ຢູ່: %1 </translation>
    </message>
    <message numerus="yes">
        <location filename="../src/core/qgshttptransaction.cpp" line="441"/>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation type="unfinished">
            <numerusform>ການເຊື່ອມຕໍ່ເຄືອຄ່າຍໝົດເວລາຫຼັງຈາກ %1 ວິນາທີ ຫຼັງຈາກບໍ່ມີການນຳໃຊ້.ອາດຈະມີປັນຫາໃນການເຊື່ອມຕໍ່ເຄືອຄ່າຍຫຼືຢູ່ທີ່ເຄືອຄ່າຍແມ່ຂອງ WMSາ.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="362"/>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>ການຈັດການ HTTP ສຳເລັດ,ແນວໃດກະຢ່າ,ມີຂໍ້ຜິດເກີດຂື້ນ: %1</translation>
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
        <translation>ລະບຸຜົນ-</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="44"/>
        <source>Feature</source>
        <translation>ຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="45"/>
        <source>Value</source>
        <translation>ຄ່າ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="106"/>
        <source>Run action</source>
        <translation type="unfinished">ແລ່ນການປະຕິບັດງານ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="196"/>
        <source>(Derived)</source>
        <translation>(ໄດ້ຮັບ)</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="13"/>
        <source>Identify Results</source>
        <translation>ລະບຸຜົນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="43"/>
        <source>Help</source>
        <translation type="unfinished">ຊ່ວຍເຫຼືອ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="46"/>
        <source>F1</source>
        <translation type="unfinished">F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="72"/>
        <source>Close</source>
        <translation type="unfinished">ປິດ</translation>
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
        <translation type="unfinished">ຜົນຂໍ້ມູນ</translation>
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
        <translation>ເຂົ້າກາຫ້ອງເຂດເເດນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="40"/>
        <source>Lower value</source>
        <translation type="unfinished">ຄ່າຕຳ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="79"/>
        <source>-</source>
        <translation type="unfinished">-</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>OK</source>
        <translation type="obsolete">ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">ຍົກເລີກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="66"/>
        <source>Upper value</source>
        <translation>ຄ່າສູງ່</translation>
    </message>
</context>
<context>
    <name>QgsLabelDialogBase</name>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="19"/>
        <source>Form1</source>
        <translation>ແບບ 1</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Field containing label:</source>
        <translation type="obsolete">ເຂດທີ່ມີກາຫມາຍ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Default label:</source>
        <translation type="obsolete">ກາຫມາຍເດີມ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="890"/>
        <source>Preview:</source>
        <translation>ເບິ່ງກ່ອນ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="908"/>
        <source>QGIS Rocks!</source>
        <translation>QGIS Rocks!</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Font Style</source>
        <translation type="obsolete">ຮູບແບບຕົວຫນັງສື</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="153"/>
        <source>Font</source>
        <translation type="unfinished">ແບບຕົວຫນັງສື</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="505"/>
        <source>Points</source>
        <translation type="unfinished">ຈຸດຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="512"/>
        <source>Map units</source>
        <translation type="unfinished">ຫົວໜ່ວຍແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="411"/>
        <source>%</source>
        <translation type="unfinished">%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="776"/>
        <source>Transparency:</source>
        <translation>ຄວາມໂປ່ງໃສ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Colour</source>
        <translation type="obsolete">ສີ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="104"/>
        <source>Position</source>
        <translation type="unfinished">ຕຳແໜງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>X Offset (pts):</source>
        <translation type="obsolete">X ລົບລ້າງ (pts):</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Y Offset (pts):</source>
        <translation type="obsolete">Y ລົບລ້າງ (pts):</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Buffer Labels?</source>
        <translation type="obsolete">ກາຫມາຍ Buffer?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="799"/>
        <source>Size:</source>
        <translation type="unfinished">ຄະໜາດ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="444"/>
        <source>Size is in map units</source>
        <translation type="unfinished">ຄະໜາດຄິດໄລ່ເປັນໃນຫົວໜ່ວຍແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="437"/>
        <source>Size is in points</source>
        <translation type="unfinished">ຄະໜາດຄິດໄລ່ເປັນຈຸດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="285"/>
        <source>Above</source>
        <translation type="unfinished">ຂ້າງເທິງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="275"/>
        <source>Over</source>
        <translation>ເທິງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="292"/>
        <source>Left</source>
        <translation type="unfinished">ຊ້າຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="268"/>
        <source>Below</source>
        <translation type="unfinished">ຂ້າງລຸ່ມ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="261"/>
        <source>Right</source>
        <translation type="unfinished">ຂວາ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="306"/>
        <source>Above Right</source>
        <translation type="unfinished">ເທິງຂວາ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="254"/>
        <source>Below Right</source>
        <translation type="unfinished">ຂ້າງລຸ່ມຂວາ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="313"/>
        <source>Above Left</source>
        <translation type="unfinished">ເທິງຊ້າຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="299"/>
        <source>Below Left</source>
        <translation type="unfinished">ຂ້າງລຸ່ມຊ້າຍ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Angle (deg):</source>
        <translation type="obsolete">ມຸມ (ອົງສາ):</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&#xb0;</source>
        <translation type="obsolete">°</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Data Defined Style</source>
        <translation type="obsolete">ຮູບແບບການກຳໜົດຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Font family:</source>
        <translation type="obsolete">&amp;ຕະກູນແບບຕົວໜັງສື:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Italic:</source>
        <translation type="obsolete">&amp;ຕົວເນີ້ງ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Underline:</source>
        <translation type="obsolete">&amp;ຂີດກ້ອງ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Bold:</source>
        <translation type="obsolete">&amp;ຕົວໜັງສືເຂັ້ມ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Size:</source>
        <translation type="obsolete">&amp;ຄະໜາດ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>X Coordinate:</source>
        <translation type="obsolete">ຕົວປະສານ X:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Y Coordinate:</source>
        <translation type="obsolete">ຕົວປະສານ Y:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Placement:</source>
        <translation type="obsolete">ການປ່ຽນແທນ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="323"/>
        <source>Font size units</source>
        <translation type="unfinished">ແບບຕົວຫນັງສື</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Font Alignment</source>
        <translation type="obsolete">ແບບຕົວຫນັງສື</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="731"/>
        <source>Placement</source>
        <translation type="unfinished">ການວາງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="99"/>
        <source>Buffer</source>
        <translation>ໜ່ວຍຄວາມຈຳຊົວຄາວ (Buffer)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="431"/>
        <source>Buffer size units</source>
        <translation>ຫົວໜ່ວຍຄະໜາດຄວາມຈຳ (Buffer)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="499"/>
        <source>Offset units</source>
        <translation>ຫນ່ວຍວັດເເທກລົບລ້າງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Data Defined Alignment</source>
        <translation type="obsolete">ການກຳໜົດການຈັດວາງຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Data Defined Buffer</source>
        <translation type="obsolete">ໜ່ວຍຄວາມຈຳຊົວຄາວ (Buffer) ກຳໜົດໄວ້ເພື່ອຂໍມູນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Data Defined Position</source>
        <translation type="obsolete">ຕຳແໜ່ງກຳໜົດໄວ້ເພື່ອຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Source</source>
        <translation type="obsolete">ແຫຼ່ງຂອງມູນ</translation>
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
        <translation type="unfinished">ສີ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="747"/>
        <source>Angle (deg)</source>
        <translation type="unfinished"></translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/ui/qgslabeldialogbase.ui" line="221"/>
        <source>°</source>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="35"/>
        <source>This layer appears to have no projection specification.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="37"/>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelectorBase</name>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="13"/>
        <source>Layer Projection Selector</source>
        <translation>ໂຕເລື້ອກຖານຊັ້ນຄາດຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="80"/>
        <source>OK</source>
        <translation>ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Define this layer&apos;s projection:&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This layer appears to have no projection specification. By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Define this layer&apos;s projection:&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This layer appears to have no projection specification. By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="113"/>
        <source>group</source>
        <translation type="unfinished">ການລວມກຸ່ມ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="426"/>
        <source>&amp;Remove</source>
        <translation>ເ&amp;ອົາອອກ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="419"/>
        <source>&amp;Make to toplevel item</source>
        <translation>&amp;ໄປຫາລາຍການຂັ້ນສູງ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="431"/>
        <source>Re&amp;name</source>
        <translation type="unfinished">ປ່ຽນຊື່</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="436"/>
        <source>&amp;Add group</source>
        <translation>&amp;ເພີ້ມກຸ່ມ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="437"/>
        <source>&amp;Expand all</source>
        <translation>&amp;ຂະຫຍາຍທັງໝົດ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="438"/>
        <source>&amp;Collapse all</source>
        <translation>&amp;ສະຫຼາຍທັງໝົດ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="440"/>
        <source>Show file groups</source>
        <translation type="unfinished">ສະແດງການລວມກຸ່ມຂອງແຟ້ມ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1827"/>
        <source>No Layer Selected</source>
        <translation type="unfinished">ບໍ່ມີລະດັບຊັ້ນຖືກເລ້ືອກ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1828"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>ເປີດຕາຕະລາງຄຸນລັກສະນະ,ທ່ານຕ້ອງເລື້ອກລະດັບຊັ້ນທີ່ບອກຕົວເລກຄະໜາດແລະທິດທາງຢູ່ໃນຄຳອະທິບາຍແຜນທີ່ົ້າ</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="490"/>
        <source>&amp;Zoom to layer extent</source>
        <translation type="unfinished">ຊຸມໄປຫາຂອບເຂດລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="493"/>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation>&amp;ຊຸມ (Zoom) ໄປຫາມາດຕາສ່ວນທີ່ເໝາະສົມ (100%)</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="497"/>
        <source>&amp;Show in overview</source>
        <translation>&amp;ສະແດງໂດຍສະຫຼຸບ້າ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="503"/>
        <source>&amp;Remove</source>
        <translation>ເ&amp;ອົາອອກ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="510"/>
        <source>&amp;Open attribute table</source>
        <translation>ເ&amp;ປີດຕາຕະລາງຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="534"/>
        <source>Save as shapefile...</source>
        <translation>ເເກັບເປັນ Shapefile...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="541"/>
        <source>Save selection as shapefile...</source>
        <translation>ດເກັບຕົວທີ່ເລື້ອກໄວເປັນ Shapefile.</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="551"/>
        <source>&amp;Properties</source>
        <translation>ຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="600"/>
        <source>More layers</source>
        <translation type="unfinished">ລະດັບຊັ້ນຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="601"/>
        <source>This item contains more layer files. Displaying more layers in table is not supported.</source>
        <translation>ລາຍການນີ້ມີແຟ້ມລະດັບຊັ້ນຫຼາຍ.ໃຫ້ເຫັນລະດັບຊັ້ນຫຼາຍໆອັນໃນຮູບຂອງຕາຕະລາງບໍ່ສາມາດເຮັດໄດ້.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="276"/>
        <source>Attribute table - </source>
        <translation>ຕາຕະລາງຄຸນລັກສະນະ- </translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="346"/>
        <source>Save layer as...</source>
        <translation>ດເກັບລະດັບຊັ້ນເປັນ...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="426"/>
        <source>Start editing failed</source>
        <translation type="unfinished">ບໍ່ສາມາດແກ້ໄຂໄດ້</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="427"/>
        <source>Provider cannot be opened for editing</source>
        <translation type="unfinished">ບໍ່ສາມາດເປີດຕົວສະຫນອງເພື່ອແກ້ໄຂໄດ້</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="440"/>
        <source>Stop editing</source>
        <translation type="unfinished">ຢຸດແກ້ໄຂ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="440"/>
        <source>Do you want to save the changes?</source>
        <translation type="unfinished">ຕ້ອງການຈັດເກັບຊິ່ງທີ່ໄດ້ປ່ຽນແປງບໍ?</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="457"/>
        <source>Error</source>
        <translation>ໍ້ຜິດພາດ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="446"/>
        <source>Could not commit changes</source>
        <translation type="unfinished">ບໍ່ສາມາດຈົດຈຳສີ່ງທີ່ໄດ້ແກ້ໄຂໄປ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="458"/>
        <source>Problems during roll back</source>
        <translation>ມີປັນຫາລະຫວ່າງດຶງຂໍ້ມູນກັບມາເບິ່ງ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="227"/>
        <source>Not a vector layer</source>
        <translation type="unfinished">ບໍ່ມີລະດັບຊັ້ນຂອງຈຳນວນຕົວເລກບອກຄະຫນາດແລະທິດທາງ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="228"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation type="unfinished">ເພື່ອເປີດຕາຕະລາງ,ເຈົ້າຕ້ອງເລື້ອກເອົາລະດັບຊັ້ນທີ່ບອກຂະຫນາດແລະທິດທາງຢູ່ໃນຄຳອະທິບາຍແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="393"/>
        <source>Saving done</source>
        <translation type="unfinished">ການຈັດເກັບສຳເລັດ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="393"/>
        <source>Export to Shapefile has been completed</source>
        <translation>ການສົ່ງອອກໄປຫາ Shapefile ໄດ້ໍສຳເລັດຢ່າງສົມບູນ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="397"/>
        <source>Driver not found</source>
        <translation>ໂປຼກຼາມໃຊ້ຄວບຄຸມອຸປະກອນຕ່າງໆບໍ່ພົບ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="397"/>
        <source>ESRI Shapefile driver is not available</source>
        <translation>ບໍ່ມີໂປຼກຼາມໃຊ້ຄວບຄຸມອຸປະກອນຕ່າງໆ ຂອງ ESRI Shapefile</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="401"/>
        <source>Error creating shapefile</source>
        <translation>ມີຂໍ້ຜິດພາດເກີດກັບ Shapefile</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="402"/>
        <source>The shapefile could not be created (</source>
        <translation>ບໍ່ສາມາດສ້າງ Shapefile</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="406"/>
        <source>Layer creation failed</source>
        <translation type="unfinished">ການສ້າງລະດັບຊັ້ນລົ້ມເຫຼວ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="501"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;ຊຸມໄປຫາຂອບເຂດລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="504"/>
        <source>&amp;Show in overview</source>
        <translation>&amp;ສະແດງໃຫ້ເຫັນໂດຍສະຫຼຸບ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="512"/>
        <source>&amp;Remove</source>
        <translation>ເ&amp;ອົາອອກ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="521"/>
        <source>&amp;Open attribute table</source>
        <translation>ເ&amp;ປີດຕາຕະລາງຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="535"/>
        <source>Save as shapefile...</source>
        <translation>ຈັດເກັບເປັນ Shapefile...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="537"/>
        <source>Save selection as shapefile...</source>
        <translation>ຈັດເກັບຕົວທີ່ໄວ້ແລ້ວເປັນ Shapefile</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="554"/>
        <source>&amp;Properties</source>
        <translation type="unfinished">&amp;ຄຸນສົມບັດ</translation>
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
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="410"/>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLineStyleDialogBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Select a line style</source>
        <translation type="obsolete">ເລື້ອກແບບເສັ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Styles</source>
        <translation type="obsolete">ຮູບແບບ
</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ok</source>
        <translation type="obsolete">ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">ຍົກເລີກ</translation>
    </message>
</context>
<context>
    <name>QgsLineStyleWidgetBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Form2</source>
        <translation type="obsolete">ຮູບແບບ2</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Outline Style</source>
        <translation type="obsolete">ຮູບແບບເສັ້ນລອບນອກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Width:</source>
        <translation type="obsolete">ຄວາມກ້ວາງ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Colour:</source>
        <translation type="obsolete">ສີ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>LineStyleWidget</source>
        <translation type="obsolete">LineStyleWidget</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>col</source>
        <translation type="obsolete">ສີ</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1224"/>
        <source>Could not draw</source>
        <translation type="unfinished">ບໍ່ສາມາດແຕ້ມເສັ້ນ້ນ</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1224"/>
        <source>because</source>
        <translation type="unfinished">ຍ້ອນວ່າ</translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
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
        <translation type="unfinished">ຈຸດເດັ່ນບໍ່ພົບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="568"/>
        <source>&lt;p&gt;No features were found within the search radius. Note that it is currently not possible to use the identify tool on unsaved features.&lt;/p&gt;</source>
        <translation>&lt;p&gt; ບໍ່ມີຈຸດເດັ່ນບໍ່ເຫັນຢູ່ໃນໂຂງເຂດລັດສະຫມີຊອກ. &lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>- %1 features found</source>
        <comment>







Identify results window title
</comment>
        <translation type="obsolete">ພົບຈຸດເດັ່ນ -%1</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="424"/>
        <source>(clicked coordinate)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="223"/>
        <source>WMS identify result for %1
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="491"/>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation type="unfinished">
            <numerusform>ພົບຈຸດເດັ່ນ -%1</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="86"/>
        <source>Split error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="86"/>
        <source>An error occured during feature splitting</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="51"/>
        <source>Snap tolerance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="52"/>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="57"/>
        <source>Could not snap segment.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="60"/>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="76"/>
        <source>Name for the map file</source>
        <translation type="unfinished">ຊື່ສຳລັບແຟ້ມແຜນທີ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>MapServer map files (*.map);;All files(*.*)</source>
        <comment>







Filter list for selecting files from a dialog box
</comment>
        <translation type="obsolete">ແຟ້ມຕ່າງໆຂອງ (*.map);;All files(*.*) ແຜນທີ່ຫນ່ວຍເຄື່ອຄ່າຍແມ່</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="84"/>
        <source>Choose the QGIS project file</source>
        <translation>ເລື້ອກເອົາແຟ້ມຫນາ້ວຽກຂອງ QGIS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>









Filter list for selecting files from a dialog box
</comment>
        <translation type="obsolete">ເລື້ອກເອົາແຟ້ມຫນ້າວຽກQGIS(*.qgs);;ເອົາແຟທັງຫມົດ(*.*)</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="197"/>
        <source>Overwrite File?</source>
        <translation type="unfinished">ຂຽນທັບແຟ້ມບໍ່?</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>






a filename is prepended to this text, and appears in a dialog box
</comment>
        <translation type="obsolete">ແຟ້ມເກົ່າຍັງມີຢູ່. 
ທ່ານຢາກຂຽນທັບບໍ່?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmapserverexport.cpp" line="74"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation>ແຟ້ມເກົ່າຍັງມີຢູ່. 
ທ່ານຢາກຂຽນທັບບໍ່?</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="77"/>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="85"/>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation type="unfinished">ເລື້ອກເອົາແຟ້ມຫນ້າວຽກQGIS(*.qgs);;ເອົາແຟທັງຫມົດ(*.*)</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="199"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a filename is prepended to this text, and appears in a dialog box</comment>
        <translation type="unfinished">ແຟ້ມເກົ່າຍັງມີຢູ່. 
ທ່ານຢາກຂຽນທັບບໍ່?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="14"/>
        <source>Export to Mapserver</source>
        <translation>ສົ່ງອອກໄປຍັງຫນ່ວຍເຄື່ອຄ່າຍແມ່ (Mapserver) </translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="352"/>
        <source>Map file</source>
        <translation type="unfinished">ແຟ້ມແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="403"/>
        <source>Export LAYER information only</source>
        <translation type="unfinished">ສົ່ງອອກພຽງຂໍ້ມູນລະດັບຊັ້ນເທົ່ານັ້ນ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="159"/>
        <source>Map</source>
        <translation type="unfinished">ແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="330"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="304"/>
        <source>Height</source>
        <translation type="unfinished">ຄວາມສູງ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="291"/>
        <source>Width</source>
        <translation type="unfinished">ຄວາມກ້ວາງ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="196"/>
        <source>dd</source>
        <translation type="unfinished">dd</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="201"/>
        <source>feet</source>
        <translation type="unfinished">ຫຼາຍຟຸດ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="206"/>
        <source>meters</source>
        <translation type="unfinished">ຫຼາຍແມັດ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="211"/>
        <source>miles</source>
        <translation type="unfinished">ຫຼາຍໄມ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="216"/>
        <source>inches</source>
        <translation type="unfinished">ຫຼາຍນີ້ວ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="221"/>
        <source>kilometers</source>
        <translation type="unfinished">ຫຼາຍກິໂລ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="182"/>
        <source>Units</source>
        <translation type="unfinished">ຫົວຫນ່ວຍ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="229"/>
        <source>Image type</source>
        <translation type="unfinished">ຊະນິດຂອງຮູບ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="243"/>
        <source>gif</source>
        <translation type="unfinished">gif</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="248"/>
        <source>gtiff</source>
        <translation type="unfinished">gtiff</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="253"/>
        <source>jpeg</source>
        <translation type="unfinished">jpeg</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="258"/>
        <source>png</source>
        <translation type="unfinished">png</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="263"/>
        <source>swf</source>
        <translation type="unfinished">swf</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="268"/>
        <source>userdefined</source>
        <translation type="unfinished">ຜູ້ນຳໃຊ້ລະບົບຖືກກຳຫນົດ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="273"/>
        <source>wbmp</source>
        <translation type="unfinished">wbmp</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="226"/>
        <source>MinScale</source>
        <translation type="unfinished">ມາດຕາສ່ວນຫນ້ອຍສຸດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="236"/>
        <source>MaxScale</source>
        <translation type="unfinished">ມາດຕາສ່ວນໃຫຍ່ສຸດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="252"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation>ຄໍາເສີມຫນ້າທີ່ຕິດກັບເເຜບທີ່, scalebar ເເລະ legend GIF filenames ໄດ້ສ້າງໂດຍໃຊ້ MapFile. ມັນຕ້ອງສັ້ນ ຯ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="29"/>
        <source>Web Interface Definition</source>
        <translation>ກຳໜົດຕົວປະສານ ຫນ້າ Web </translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="98"/>
        <source>Header</source>
        <translation>ຫົວ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="134"/>
        <source>Footer</source>
        <translation>ຕີນ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="58"/>
        <source>Template</source>
        <translation>ຕົວຢ່າງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="56"/>
        <source>&amp;Help</source>
        <translation>&amp;ຊ່ວຍເຫລືອ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="59"/>
        <source>F1</source>
        <translation type="unfinished">F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="85"/>
        <source>&amp;OK</source>
        <translation>&amp;ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="101"/>
        <source>&amp;Cancel</source>
        <translation>&amp;ຍົກເລິກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="116"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="362"/>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation>ຊື່ສຳລັບແຟ້ມແຜນທີ່ສາມາດສ້າງໄດ້ຈາກແຟ້ມໜ້າວຽກ QGIS</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="400"/>
        <source>If checked, only the layer information will be processed</source>
        <translation>ຖ້າກວດແລ້ວ,ມີພຽງຂໍມູນລະດັບຊັ້ນເທົ່ານັ້ນທີ່ຈະເຮັດວຽກ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="68"/>
        <source>Path to the MapServer template file</source>
        <translation>ທາງເຂົ້າໄປຫາ MapServer ຕົວຢ່າງ ເເຟ້ມ</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="340"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation>ຄໍາເສີມຫນ້າທີ່ຕິດກັບເເຜບທີ່, scalebar ເເລະ legend GIF filenames ໄດ້ສ້າງໂດຍໃຊ້ MapFile</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="376"/>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation>ເສັ້ນທາງໄປຫາແຟ້ມໜ້າວຽກ QGIS ເພື່ອສົ່ງອອກໄປຫາ ຮູບແບບການຈັດແຟ້ມແຜນທີ່ຂອງໜ່ວຍເຄືອຄ່າຍແມ່ເຕີ້</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="383"/>
        <source>QGIS project file</source>
        <translation>ແຟ້ມໜ້າວຽກຂອງ QGIS</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="369"/>
        <source>Browse...</source>
        <translation>ການເບີ່ງ...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="393"/>
        <source>Save As...</source>
        <translation>ດເກັບເປັ..ນ.</translation>
    </message>
</context>
<context>
    <name>QgsMarkerDialogBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Choose a marker symbol</source>
        <translation type="obsolete">ເລື້ອກຕົວໝາຍເຄື່ອງໝາຍ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Directory</source>
        <translation type="obsolete">ໄດເຣກທໍຣີ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ok</source>
        <translation type="obsolete">ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">ຍົກເລີກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>New Item</source>
        <translation type="obsolete">ລາຍການໃໝ່</translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="19"/>
        <source>Measure</source>
        <translation>ວັດແທກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="125"/>
        <source>New</source>
        <translation type="unfinished">ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="102"/>
        <source>Help</source>
        <translation type="unfinished">ຊ່ວຍເຫຼືອ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="132"/>
        <source>Cl&amp;ose</source>
        <translation>&amp;ປິດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="66"/>
        <source>Total:</source>
        <translation>ລວມ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="86"/>
        <source>Segments</source>
        <translation type="unfinished">ຫລາຍທ່ອນ</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="198"/>
        <source>Segments (in meters)</source>
        <translation>ຮູບຫລາຍທ່ອນ (ຄິດໄລເປັນແມັດ)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="201"/>
        <source>Segments (in feet)</source>
        <translation>ຮູບຫລາຍທ່ອນ (ຄິດໄລເປັນຟຸດດ)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="204"/>
        <source>Segments (in degrees)</source>
        <translation>ຮູບຫລາຍທ່ອນ (ຄິດໄລເປັນອົງສາ)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="207"/>
        <source>Segments</source>
        <translation>ຫລາຍທ່ອນ</translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="74"/>
        <source>Incorrect measure results</source>
        <translation>ຜົນຂອງການວັດບໍ່ຖືກຕ້ອງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggest that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation type="obsolete">&lt;p&gt;ລະບົບປະສານງານຂອງໜ້າວຽກ(ເຊັ່ນວ່າ:Mercator).ຖ້າເປັນເຊັ່ນນັ້ນ,ຜົນຈາກເສັ້ນແລະພື້ນທີ່ຈະບໍ່ຖືກຕ້ອງ.&lt;/p&gt;&lt;p&gt;ເພື່ອສ້ອມແປງບັນຫານີ້,ຕັ້ງຕົວປະສານງານຂອງແຜນທີ່ໃຫ້ເໝາະສົມໂດຍໃຊ້&lt;tt&gt;ການຕັ້ງຄ່າ:ຄຸນລັກສະນະຂອງໜ້າວຽກ&lt;/tt&gt;ລາຍການ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="82"/>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="13"/>
        <source>QGIS Message</source>
        <translation>ຂໍ້ຄວາມ QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="48"/>
        <source>Close</source>
        <translation type="unfinished">ປິດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="28"/>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished">ບໍ່ສະແດງຂໍ້ຄວາມອີກ</translation>
    </message>
</context>
<context>
    <name>QgsMySQLProvider</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Unable to access relation</source>
        <translation type="obsolete">ບໍ່ສາມາດເຂົ້າຫາການພົວພັນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Unable to access the </source>
        <translation type="obsolete">ບໍ່ສາມາດເຂົ້າຫາ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> relation.
The error message from the database was:
</source>
        <translation type="obsolete">ສໍາພັນ.
ຂໍ້ຄວາມຜິດພາດຈາກຖານຂໍ້ມູນວ່າ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>No GEOS Support!</source>
        <translation type="obsolete">ບໍ່ເຂົ້າກັບ GEOS!</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="obsolete">ການຕິດຕັ້ງ PostGIS ບໍ່ເຂົ້າກັບ GEOS ການເລື້ອກຈຸດເດັ່ນ ແລະ ລະບຸຈະເຮັດວຽກບໍ່ຖືກຕ້ອງ。ກະລຸນາຕິດຕັ້ງ PostGIS ທີ່ເຂົ້າກັນກັບ GEOS  (http://geos.refractions.net)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Save layer as...</source>
        <translation type="obsolete">ດດເກັບລະດັບຊັ້ນເປັນ...</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Error</source>
        <translation type="obsolete">ຜິດພາດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Error creating field </source>
        <translation type="obsolete">ມີຂໍ້ຜິດພາດເກີດຂື້ນໃນການສ້າງສະໜາມໍ້</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Layer creation failed</source>
        <translation type="obsolete">ການສ້າງລະດັບຊັ້ນລົ້ມເຫຼວ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Error creating shapefile</source>
        <translation type="obsolete">ມີຂໍ້ຜິດພາດໃນການສ້າງ Shapefile</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>The shapefile could not be created (</source>
        <translation type="obsolete">ບໍ່ສາມາດສ້າງ Shapefile (</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Driver not found</source>
        <translation type="obsolete">ຊອບເເວ ໃຊ້ຄວບຄຸມອຸປະກອນຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> driver is not available</source>
        <translation type="obsolete">ບໍ່ມີໂປຼກຼາມຄວບຄຸມອຸປະກອນຕ່າງໆ</translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="121"/>
        <source>Test connection</source>
        <translation type="unfinished">ທົດສອບການເຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="121"/>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>ການເຊື່ອມຕໍ່ລົ້ມເຫຼວ-ກວດເບິ່ງການຕັ້ງຄ່າແລະລອງໃໝ່ . 
ຂໍ້ມູນຜິດພາດ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="118"/>
        <source>Connection to %1 was successful</source>
        <translation>ການເຊື່ອມຕໍ່ %1 ປະສົບຜົນສຳເລັດ.</translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="21"/>
        <source>Create a New PostGIS connection</source>
        <translation>ສ້າງການເຊື່ອມຕໍ່ PostGIS ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="252"/>
        <source>OK</source>
        <translation type="unfinished">ຕົກລົງ </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="268"/>
        <source>Cancel</source>
        <translation type="unfinished">ຍົກເລີກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="284"/>
        <source>Help</source>
        <translation type="unfinished">ຊ່ວຍເຫຼືອ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="39"/>
        <source>Connection Information</source>
        <translation>ຂໍ້ມູນການເຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="147"/>
        <source>Host</source>
        <translation>ໜ່ວນແມ່ (Host)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="157"/>
        <source>Database</source>
        <translation type="unfinished">ຖານຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="177"/>
        <source>Username</source>
        <translation type="unfinished">ຊື່ຜູ້ໃຊ້</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="137"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="207"/>
        <source>Name of the new connection</source>
        <translation type="unfinished">ຊື່ຂອງການເຊື່ອມຕໍ່ຄັ້ງໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="187"/>
        <source>Password</source>
        <translation type="unfinished">ລະຫັດຜ່ານ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="112"/>
        <source>Test Connect</source>
        <translation type="unfinished">ທົດສອບການເຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="105"/>
        <source>Save Password</source>
        <translation>ດເກັບລະຫັດຜ່ານ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="287"/>
        <source>F1</source>
        <translation type="unfinished">F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="167"/>
        <source>Port</source>
        <translation>ພອດ (Port)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="220"/>
        <source>5432</source>
        <translation type="unfinished">5432</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Restrict the search to the public schema for spatial tables not in the geometry_columns table&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Restrict the search to the public schema for spatial tables not in the geometry_columns table&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="67"/>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation>ເບິ່ງເຂົ້າໄປໃນແບບແຜນທົ່ວໄປ ແບບແຜນ (schema) ເປີດເຜີຍ </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;&quot;&gt;Restrict the displayed tables to those that are in the geometry_columns table&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;&quot;&gt;Restrict the displayed tables to those that are in the geometry_columns table&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;&quot;&gt;Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;&quot;&gt;Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="90"/>
        <source>Only look in the geometry_columns table</source>
        <translation>ເບິ່ງເຂົ້າໃນຕາຕະລາງຖັນຄະນິດສາດ້າ</translation>
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
        <location filename="" line="135533324"/>
        <source>Create a New WMS connection</source>
        <translation type="obsolete">ສ້າງການເຊື່ອມຕໍ່ WMS ໃຫມ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Connection Information</source>
        <translation type="obsolete">ຂໍ້ມູນການເຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="31"/>
        <source>Name</source>
        <translation>ຊື່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="60"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Proxy Host</source>
        <translation type="obsolete">ພຣອກຊີໜ່ວຍແມ່ (Proxy Host)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Proxy Port</source>
        <translation type="obsolete">ພຣອກຊີພອດ (Proxy Port)t)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Proxy User</source>
        <translation type="obsolete">ພອກຊີຜູ້ໃຊ້ (Proxy User)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Proxy Password</source>
        <translation type="obsolete">ພອກຊີລະຫັດຜ່ານ (Proxy Password)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Your user name for the HTTP proxy (optional)</source>
        <translation type="obsolete">ຊື່ຜູ້ໃຊ້ເຄືອຄ່າຍຂອງທ່ານສຳລັບ ພອກຊີ HTTP (ທາງເລື້ອກ)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Password for your HTTP proxy (optional)</source>
        <translation type="obsolete">ລະຫັດຜ່ານ ສຳລັບພອກຊີ HTTP (ທາງເລື້ອກ)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="50"/>
        <source>Name of the new connection</source>
        <translation>ຊື່ການເຊື່ອມຕໍ່ໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="73"/>
        <source>HTTP address of the Web Map Server</source>
        <translation>ທີ່ຢູ່ HTTP ສຳລັບເວບແຜນທີ່ຂອງໜ່ວຍເຄືອຄ່າຍແມ່ (Web Map Server)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name of your HTTP proxy (optional)</source>
        <translation type="obsolete">ຊື່ຂອງພອກຊີ HTTP ຂອງທ່ານ (ທາງເລື້ອກ)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Port number of your HTTP proxy (optional)</source>
        <translation type="obsolete">ຈຳນວນພອດຂອງພອກຊີ HTTP (ທາງເລື້ອກ)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>OK</source>
        <translation type="obsolete">ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">ຍົກເລີກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Help</source>
        <translation type="obsolete">ຊ່ວຍເຫຼືອ</translation>
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
        <translation>ຂ້າງລຸ່ມເບື້ອງຊາຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Top Right</source>
        <translation>ຂ້າງເທິງເບື້ອງຂວາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Bottom Right</source>
        <translation>ຂ້າງລຸ່ມເບື້ອງຂວາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Top Left</source>
        <translation>ຂ້າງເທິງເບື້ອງຊາຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="96"/>
        <source>&amp;North Arrow</source>
        <translation>&amp;ລູກສອນເຫນືອ (North Arrow)</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="97"/>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>ສ້າງລູກສອນເຫນືອ(North Arrow) ທີ່ສະແດງຢູ່ພື້ນແຜນທີ່ພື້ນແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="255"/>
        <source>&amp;Decorations</source>
        <translation type="unfinished">ການຕົກແຕ່ງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="246"/>
        <source>North arrow pixmap not found</source>
        <translation>ບໍ່ເຫັນ ລູກສອນເຫນືອ pixmap </translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugingui.cpp" line="157"/>
        <source>Pixmap not found</source>
        <translation>ບໍ່ພົບຮູບແຜນທີ່ (pixmap)</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="235"/>
        <source>North Arrow Plugin</source>
        <translation>ໂປຼກຼາມເສີມ ລູກສອນເຫນືອ(North Arrow)</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="35"/>
        <source>Properties</source>
        <translation>ຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="55"/>
        <source>Angle</source>
        <translation>ມຸມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="62"/>
        <source>Placement</source>
        <translation>ການວາງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="75"/>
        <source>Set direction automatically</source>
        <translation>ຕັ້ງທິດທາງໂດຍອັດຕະໂນມັດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="85"/>
        <source>Enable North Arrow</source>
        <translation>ເປີດລູກສອນເຫນືອ (North Arrow)</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="141"/>
        <source>Top Left</source>
        <translation>ຂ້າງເທິງເບື້ອງຊາຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="146"/>
        <source>Top Right</source>
        <translation>ຂ້າງເທິງເບື້ອງຂວາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="151"/>
        <source>Bottom Left</source>
        <translation>ຂ້າງລຸ່ມເບື້ອງຊາຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="156"/>
        <source>Bottom Right</source>
        <translation>ຂ້າງລຸ່ມເບື້ອງຊາຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="137"/>
        <source>Placement on screen</source>
        <translation>ການວ່າງໃສ່ໜ້າຈໍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="164"/>
        <source>Preview of north arrow</source>
        <translation>ເບິ່ງພາບລູກສອນເຫນືອ (North Arrow) ກ່ອນພີມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="183"/>
        <source>Icon</source>
        <translation>ຮູບ ໄອກອນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>New Item</source>
        <translation type="obsolete">ລາຍການໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="198"/>
        <source>Browse...</source>
        <translation>ການເບີ່ງ.</translation>
    </message>
</context>
<context>
    <name>QgsOGRFactory</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Wrong Path/URI</source>
        <translation type="obsolete">ເສັ້ນທາງ (path) ແລະ (URI) ຜິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>The provided path for the dataset is not valid.</source>
        <translation type="obsolete">ເສັ້ນທາງ (Path) ທີ່ປ້ອນໃສ່ສຳລັບຊຸດແຜນທີ່ໃຊ້ການບໍ່ໄດ້</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="154"/>
        <source>Detected active locale on your system: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="332"/>
        <source>to vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="336"/>
        <source>to segment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="340"/>
        <source>to vertex and segment</source>
        <translation type="unfinished"></translation>
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
        <translation> ທາງເລຶ້ກຂອງ QGIS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>epiphany</source>
        <translation type="obsolete">epiphany</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>galeon</source>
        <translation type="obsolete">galeon</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>konqueror</source>
        <translation type="obsolete">konqueror</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>mozilla</source>
        <translation type="obsolete">mozilla</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>opera</source>
        <translation type="obsolete">opera</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Search Radius for Identifying Features</source>
        <translation type="obsolete">ຊອກຫາເສັ້ນລັດສະໝີເພື່ອລະບຸຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="212"/>
        <source>Hide splash screen at startup</source>
        <translation>ເຊື່ອງ Splash screen ຢູ່ Startup</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Appearance</source>
        <translation type="obsolete">&amp;Appearance</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Icon Theme</source>
        <translation type="obsolete">&amp;Icon Theme</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Theme</source>
        <translation type="obsolete">Theme</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="188"/>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;ສັ່ງເກດ:&lt;/b&gt; ການປ່ຽນແປງຂອງ Theme ມີຜົນ &lt;b&gt; ເທື່ອໜ້າຕ້ອງເລີ້ມດ້ວຍ QGIS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Help &amp;Browser</source>
        <translation type="obsolete">&amp;ບາວເຊີ ຊ່ວຍ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Open help documents with</source>
        <translation type="obsolete">ເປີດເອກະສານຊ່ວຍເຫຼືອດ້ວຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="236"/>
        <source>&amp;Rendering</source>
        <translation>&amp;ການປະສົມປະສານ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Update display after reading</source>
        <translation type="obsolete">ຍົກລະດັບພາບທີ່ສະແດງຫຼັງການອ່ານ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="265"/>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>ການສະແດງແຜນທີ່ຈະໄດ້ຮັບການຍົກລະດັບ (ແຕ້ມຮູບ) ພາຍຫຼັງຈຸດເດັ່ນໄດ້ຖືກອ່ານຈາກແຫຼ່ງຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>features</source>
        <translation type="obsolete">ຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>(Set to 0 to not update the display until all features have been read)</source>
        <translation type="obsolete">(ຕັ້ງຄ່າໄປທີ່ 0 ບໍ່ໄດ້ຍົກລະດັບພາບທີ່ສະແດງອອກມາຈົນກະທັ້ງຈຸດເດັ່ນທັງໝົດຖືກອ່ານ)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Initial Visibility</source>
        <translation type="obsolete">ການເບິ່ງເຫັນເບື້ອງຕົ້ນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="819"/>
        <source>Select Global Default ...</source>
        <translation>ເລື້ອກຄ່າຕາຍຕົວຂອງໂລກ...</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Prompt for projection.</source>
        <translation type="obsolete">ກະຕຸ້ນສຳລັບ ກາບຄາດຄະເນ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Project wide default projection will be used.</source>
        <translation type="obsolete">ໂຄງການທົ່ວໄປຈະໃຊ້ ກາບຄາດຄະເນເດີມ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>firefox</source>
        <translation type="obsolete">firefox</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>mozilla-firefox</source>
        <translation type="obsolete">mozilla-firefox</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Splash screen</source>
        <translation type="obsolete">&amp;Splash ຈໍ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Default Map Appearance (Overridden by project properties)</source>
        <translation type="obsolete">ຮູບຮ່າງ ຕາຍຕົວຂອງແຜນທີ່ (...)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Background Color:</source>
        <translation type="obsolete">ສີພື້ນ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Selection Color:</source>
        <translation type="obsolete">ການເລື້ອກສີ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Appearance</source>
        <translation type="obsolete">ຮູບຮ່າງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Capitalise layer name</source>
        <translation type="obsolete">ພີມຊື່ລະດັບຊັ້ນເປັນຕົວໜັງສືໃຫຍ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="306"/>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>ເຮັດເເຖວເບີ່ງບໍ່ເປັນເເຂ້ວຫລາຍ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="248"/>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>ໂດຍລະດັບ&amp;ຊັ້ນຕາຍຕົວໃໝ່ທີ່ເພີ້ມເຂົ້າໄປໃນແຜນທີ່ຄວາມຖືກສະແດງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Update during drawing</source>
        <translation type="obsolete">&amp;ຍົກລະດັບໃນລະຫວ່າງລາກເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="437"/>
        <source>Measure tool</source>
        <translation>ເຄື່ອງມືວັດແທກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ellipsoid for distance calculations:</source>
        <translation type="obsolete">ການຄິດໄລ່ຄວາມຫ່າງຂອງຮູບຊົງໄຂ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="507"/>
        <source>Search radius</source>
        <translation>ຄົ້ນຫາເສັ້ນລັດສະໝີ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="788"/>
        <source>Pro&amp;jection</source>
        <translation>&amp;ຄາດຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="829"/>
        <source>When layer is loaded that has no projection information</source>
        <translation>ເຫມື່ອລະດັບຊັ້ນໄດ້ໂລດມາທີ່ບໍ່ມີຂໍ້ມູນຄາດຫມາຍ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Global default projection displa&amp;yed below will be used.</source>
        <translation type="obsolete">ຄາດຫມາຍຕາຍຕົວໂລກ ສະເເດງຢູ່ຂ້າວລູມຈະຖຶກໃຊ້.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; The browser must be in your PATH or you can specify the full path above</source>
        <translation type="obsolete">&lt;b&gt;ສັ່ງເກດ&lt;/b&gt; ໂປຼກຼາມຫຼີ້ນອີນເຕີເນດຕ້ອງຢູ່ໃນເສັ້ນທາງ ຫຼື ທ່ານສາມາດລະບຸເສັ້ນທາງເຕັ້ມຂ້າງເທິງ້າ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Rendering</source>
        <translation type="obsolete">ການປະສົມປະສານ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Selecting this will unselect the &apos;make lines less&apos; jagged toggle&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Selecting this will unselect the &apos;make lines less&apos; jagged toggle&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="316"/>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>ແກ້ໄຂບັນຫາດ້ວຍການເຕີມຮູບຫຼາຍຫຼ່ຽມ (polygons) ທີ່ບໍ່ຖືກຕ້ອງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="323"/>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>ແຕ້ມແຜນທີ່ຄືນໃຫມ່ໃຫ້ມີຄວາມຕໍ່ເນື່ອງເມື່ອລາກຄຳອະທິບາຍແຜນທີ/ຕົວແບ່ງແຜນທີ່ຄືນ.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="347"/>
        <source>&amp;Map tools</source>
        <translation>ເ&amp;ຄື່ອງມືແຜນທີ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Note:&lt;/span&gt; Specify the search radius as a percentage of the map width.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Note:&lt;/span&gt; Specify the search radius as a percentage of the map width.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="545"/>
        <source>%</source>
        <translation type="unfinished">%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="365"/>
        <source>Panning and zooming</source>
        <translation>ການແພນ (Panning) ແລະ ຊຸມພາບ (Zooming)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="384"/>
        <source>Zoom</source>
        <translation>Zoom</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="389"/>
        <source>Zoom and recenter</source>
        <translation>ຊຸມ ແລະ ຈັດໃຫ້ໄປຢູ່ຈຸດກາງໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="399"/>
        <source>Nothing</source>
        <translation>ບໍ່ມີຫຍັງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Zoom factor:</source>
        <translation type="obsolete">ຊຸມຫາຕົວຄູນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Mouse wheel action:</source>
        <translation type="obsolete">ການປະຕິບັດງານ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="32"/>
        <source>&amp;General</source>
        <translation>&amp;ທົວໄປ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>General</source>
        <translation type="obsolete">ທົ່ວໄປ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ask to save project changes when required</source>
        <translation type="obsolete">ຖາມເພື່ອຈັດເກັບການປ່ຽນແປງໜ້າວຽກເມື່ອຕ້ອງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Rubberband color:</source>
        <translation type="obsolete">ສີຂອງ Rubberband:</translation>
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
        <location filename="../src/ui/qgsoptionsbase.ui" line="313"/>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="394"/>
        <source>Zoom to mouse cursor</source>
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
        <location filename="../src/ui/qgsoptionsbase.ui" line="578"/>
        <source>Digitizing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="584"/>
        <source>Rubberband</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="590"/>
        <source>Line width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="600"/>
        <source>Line width in pixels</source>
        <translation type="unfinished">ຄວາມໜາຂອງເສັ້ນເປັນ Pixels</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="610"/>
        <source>Line colour</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="636"/>
        <source>Snapping</source>
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
        <location filename="../src/ui/qgsoptionsbase.ui" line="872"/>
        <source>Locale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="878"/>
        <source>Override system locale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="887"/>
        <source>Locale to use instead</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="900"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Enabling / changing overide on local requires an application restart</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="926"/>
        <source>Additional Info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="932"/>
        <source>Detected active locale on your system:</source>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="974"/>
        <source>Port</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished">ລະຫັດຜ່ານ</translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="16"/>
        <source>Paste Transformations</source>
        <translation>ນຳເອົາການປ່ຽນແປງ (Transformations) ອອກມາໃຊ້</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="39"/>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation>&lt;b&gt;ສັງເກດ: ຄຳສັ່ງນີ້ຍັງໃຊ້ການບໍ່ໄດ້!&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="62"/>
        <source>Source</source>
        <translation type="unfinished">ແຫຼ່ງຂອງມູນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="83"/>
        <source>Destination</source>
        <translation>ຈຸດໝາຍປາຍທາງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="122"/>
        <source>&amp;Help</source>
        <translation>&amp;ຊ່ວຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="125"/>
        <source>F1</source>
        <translation type="unfinished">F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="151"/>
        <source>Add New Transfer</source>
        <translation>ເພີ້ມການຖ່າຍໂອນໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="158"/>
        <source>&amp;OK</source>
        <translation>&amp;ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="174"/>
        <source>&amp;Cancel</source>
        <translation>&amp;ຍົກເລິກ</translation>
    </message>
</context>
<context>
    <name>QgsPatternDialogBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Select a fill pattern</source>
        <translation type="obsolete">ເລື້ອກແບບເຕີມ (fill pattern)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">ຍົກເລີກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ok</source>
        <translation type="obsolete">ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>No Fill</source>
        <translation type="obsolete">ບໍ່ໄດ້ເຕີມ</translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="117"/>
        <source>Buffer features in layer %1</source>
        <translation>ສ້າງໜ່ວຍຄວາມຈຳຊົ່ວຄາວໃສ່ຈຸດເດັ່ນຢູ່ໃນລະດັບຊັ້ນ %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="340"/>
        <source>Error connecting to the database</source>
        <translation>ມີຂໍ້ຜິດພາດເກີດຂື້ນກັບການເຊື່ອມຕໍ່ກັບຖານຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="78"/>
        <source>&amp;Buffer features</source>
        <translation>&amp;ຈຸດເດັ່ນມີຄວາມຈຳຊົ່ວຄາວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation>ລະດັບຊັ້ນໃໝ່ຖືກສ້າງຂື້ນຢູ່ຖານຂໍ້ມູນໂດຍມີຈຸດເດັ່ນທີ່ມີຄວາມຈຳຊົ່ວຄາວ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="410"/>
        <source>&amp;Geoprocessing</source>
        <translation>&amp;Geoprocessing</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="325"/>
        <source>Unable to add geometry column</source>
        <translation>ບໍ່ສາມາດເພີ້ມຖັນເລຂາຄະນິດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="327"/>
        <source>Unable to add geometry column to the output table </source>
        <translation>ບໍ່ສາມາດເພີ້ມຖັນເລຂາຄະນິດໄປໃສ່ຕາຕະລາງຂາອອກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="331"/>
        <source>Unable to create table</source>
        <translation>ບໍ່ສາມາດສ້າງຕາຕະລາງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="333"/>
        <source>Failed to create the output table </source>
        <translation>ບໍ່ສາມາດສ້າງຕາຕະລາງຂາອອກ </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="346"/>
        <source>No GEOS support</source>
        <translation>ບໍ່ເຂົ້າກັບ GEOS</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="347"/>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation>ຄຳສັ່ງຂອງໜ່ວຍຄວາມຈຳຊົວຄາວຕ້ອງການໆເຂົ້າກັນກັບ PostGIS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Not a PostgreSQL/PosGIS Layer</source>
        <translation type="obsolete">ບໍ່ແມ່ນລະດັບຂອງ PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> is not a PostgreSQL/PosGIS layer.
</source>
        <translation type="obsolete">ບໍ່ແມ່ນລະດັບຊັ້ນຂອງ PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Geoprocessing functions are only available for PostgreSQL/PosGIS Layers</source>
        <translation type="obsolete">Geoprocessing ການເຮັດວຽກເເມ່ນ ສະເພາະເເຕ່ PostgreSQL/PostGIS ລະດັບຊັ້ນເທົ້ານັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="356"/>
        <source>No Active Layer</source>
        <translation>ລະດັບຊັ້ນບໍ່ເຮັດວຽກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="357"/>
        <source>You must select a layer in the legend to buffer</source>
        <translation>ທ່ານຕ້ອງເລື້ອກລະດັບຊັ້ນຢູ່ໃນຄຳອະທິບາຍແຜນທີ່ເພື່ອນຳເຂົ້າໄປໜ່ວຍຄວາມຈຳຊົ່ວຄາວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation>ສ້າງໜ່ວຍຄວາມຈຳຊົ່ວຄາວສຳລັບ ລະດັບຊັ້ນ PostgreSQL </translation>
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
        <location filename="../src/app/qgspgquerybuilder.cpp" line="83"/>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>ຕາຕະລາງ &lt;b&gt;%3&lt;/b&gt; ຖານຂໍ້ມູນ &lt;b&gt;%2&lt;/b&gt; ໜ່ວຍເຄື່ອຄ່າຍແມ່ &lt;b&gt;%1&lt;/b&gt;, ຜູ້ໃຊ້ &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="279"/>
        <source>Query Result</source>
        <translation>ຜົນຂອງການນຳຂໍ້ມູນມາໃຊ້</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="281"/>
        <source>The where clause returned </source>
        <translation>ປະໂຫຍກນ້ອຍກັບຄືນມາ</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="281"/>
        <source> rows.</source>
        <translation>ແຖວ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="285"/>
        <source>Query Failed</source>
        <translation>ການນຳຂໍ້ມູນອອກມາໃຊ້ລົ້ມເຫຼວ</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="287"/>
        <source>An error occurred when executing the query:</source>
        <translation>ມີຂໍ້ຜິດພາດເກີດຂື້ນເມື່ອຈັດການກັບ ການນຳຂໍ້ມູນມາໃຊ້ (Query):</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="67"/>
        <source>Connection Failed</source>
        <translation>ການເຊື່ອມຕໍ່ລົ້ມເຫຼວ</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="67"/>
        <source>Connection to the database failed:</source>
        <translation>ການເຊື່ອມຕໍ່ຫາຖານຂໍ້ມູນລົ້ມເຫລວ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="204"/>
        <source>Database error</source>
        <translation>ມີຂໍ້ຜິດພາດເກີດຂື້ນກັບຖານຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Failed to get sample of field values</source>
        <translation type="obsolete">ບໍ່ສາມາດໄດ້ຮັບຕົວຢ່າງຂອງຄ່າສະໜາມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="340"/>
        <source>No Records</source>
        <translation type="unfinished">ບໍ່ມີບັນທຶກ</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="340"/>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>ຄໍາຊັ່ງທີລະບຸນັ້ນຜົນເເມ່ນ 0 ຂໍ້ມູນກັບຄືນມາ. ລະດັບຊັ້ນ PostgreSQL ເເທ້ຕ້ອງມີ 1 ຈຸດເດັນຂື້ນໄປ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="204"/>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;ບໍ່ສາມາດໄດ້ຮັບຕົວຢ່າງຂອງຄ່າສະໜາມໃນເມື່ອນຳໃຊ້ SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="268"/>
        <source>No Query</source>
        <translation>ບໍ່ມີຕົວຕັ້ງຄຳຖາມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="268"/>
        <source>You must create a query before you can test it</source>
        <translation>ຕ້ອງສ້າງຕົວຕັ້ງຄຳຖາມກ່ອນທົດລອງມັນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="334"/>
        <source>Error in Query</source>
        <translation>ຂໍ້ຜິດພາດໄນຕົວຕັ້ງຄຳຖາມ</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="21"/>
        <source>PostgreSQL Query Builder</source>
        <translation>ຕົວສ້າງ ການຕິດຕໍ່ພົວພັນ PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="328"/>
        <source>Clear</source>
        <translation>ລົບລ້າງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="338"/>
        <source>Test</source>
        <translation>ທົດລອງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="348"/>
        <source>Ok</source>
        <translation type="unfinished">ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="358"/>
        <source>Cancel</source>
        <translation type="unfinished">ຍົກເລີກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="274"/>
        <source>SQL where clause</source>
        <translation>SQL ໃສ ປະໂຫຍກນ້ອຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="149"/>
        <source>Operators</source>
        <translation type="unfinished">ຕົວປະຕິບັດງານ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="167"/>
        <source>=</source>
        <translation type="unfinished">=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="209"/>
        <source>IN</source>
        <translation type="unfinished">IN</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="216"/>
        <source>NOT IN</source>
        <translation type="unfinished">NOT IN</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="174"/>
        <source>&lt;</source>
        <translation type="unfinished">&lt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="230"/>
        <source>&gt;</source>
        <translation type="unfinished">&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="202"/>
        <source>%</source>
        <translation type="unfinished">%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="258"/>
        <source>&lt;=</source>
        <translation type="unfinished">&lt;=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="251"/>
        <source>&gt;=</source>
        <translation type="unfinished">&gt;=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="223"/>
        <source>!=</source>
        <translation type="unfinished">!=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="237"/>
        <source>LIKE</source>
        <translation>ຄືກັນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="195"/>
        <source>AND</source>
        <translation type="unfinished">ແລະ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="244"/>
        <source>ILIKE</source>
        <translation type="unfinished">ILIKE</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="188"/>
        <source>OR</source>
        <translation type="unfinished">ຫຼື</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="181"/>
        <source>NOT</source>
        <translation type="unfinished">ບໍ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="83"/>
        <source>Values</source>
        <translation type="unfinished">ຄ່າ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="139"/>
        <source>All</source>
        <translation type="unfinished">ທັງໝົດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="126"/>
        <source>Sample</source>
        <translation type="unfinished">ຕົວຢ່າງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="46"/>
        <source>Fields</source>
        <translation type="unfinished">ສະໜາມ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Datasource:</source>
        <translation type="obsolete">ແຫຼ່ງຂໍ້ມູນ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="33"/>
        <source>Datasource</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="64"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of fields in this vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="101"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of values for the current field.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="120"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Take a &lt;span style=&quot; font-weight:600;&quot;&gt;sample&lt;/span&gt; of records in the vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="133"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Retrieve &lt;span style=&quot; font-weight:600;&quot;&gt;all&lt;/span&gt; the record in the vector file (&lt;span style=&quot; font-style:italic;&quot;&gt;if the table is big, the operation can consume some time&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Choose a directory</source>
        <translation type="obsolete">ເລື້ອກໄດເຣກທໍຣີ (Directory)</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="203"/>
        <source>No Plugins</source>
        <translation>ບໍ່ມີໂປຼກຼາມເສີມ (Plugins)</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="203"/>
        <source>No QGIS plugins found in </source>
        <translation>ບໍ່ມີໂປຼກຼາມເສີມ (Plugin) ຂອງ QGIS ຖືກພົບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="84"/>
        <source>&amp;Select All</source>
        <translation type="unfinished">ເ&amp;ລື້ອກທັງໝົດ</translation>
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
        <location filename="" line="135533324"/>
        <source>QGIS Plugin Manger</source>
        <translation type="obsolete">ຕົວຈັດການໂປຼກຼາມເສີມ (Plugin)ຂອງ QGIS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name</source>
        <translation type="obsolete">ຊື່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Description</source>
        <translation type="obsolete">ລາຍການ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Library Name</source>
        <translation type="obsolete">ຊື່ຫໍສະຫມຸດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Plugin Directory</source>
        <translation type="obsolete">ໄດເຣກທໍຣີຂອງໂປຼກຼາມເສີມ (Plugin)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>To load a plugin, click the checkbox next to the plugin and click Ok</source>
        <translation type="obsolete">ເພື່ອໂຫຼດໂປຼກຼາມເສີມ (Plugin),ກົດໄປຫາ Checkbox ຖັດຈາກthe plugin ແລະກົດ OK</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Version</source>
        <translation type="obsolete">ລຸ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Select All</source>
        <translation type="obsolete">ເ&amp;ລື້ອກທັງໝົດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Alt+S</source>
        <translation type="obsolete">Alt+S</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>C&amp;lear All</source>
        <translation type="obsolete">&amp;ລົບລ້າງທັງໝົດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Alt+L</source>
        <translation type="obsolete">Alt+L</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Ok</source>
        <translation type="obsolete">&amp;ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Alt+O</source>
        <translation type="obsolete">Alt+O</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Close</source>
        <translation type="obsolete">&amp;ປິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Alt+C</source>
        <translation type="obsolete">Alt+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="16"/>
        <source>QGIS Plugin Manager</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished">ໄດເຣກທໍຣີ</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="488"/>
        <source>Zoom In</source>
        <translation>ດື່ງພາບເຂົ້າໃກ້ (Zoom In)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="487"/>
        <source>z</source>
        <translation type="unfinished">z</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="493"/>
        <source>Zoom Out</source>
        <translation type="unfinished">ດືງພາບອອກໄກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="492"/>
        <source>Z</source>
        <translation type="unfinished">Z</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="496"/>
        <source>Zoom To Layer</source>
        <translation>ຊຸມ (Zoom) ໄປຫາລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="498"/>
        <source>Zoom to Layer</source>
        <translation>ຊຸມ (Zoom)ໄປຫາລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="501"/>
        <source>Pan Map</source>
        <translation type="unfinished">ຕັດເອົາພາບແຜ່ນທີ່ໆຕ້ອງການເບີ່ງສະເພາະ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="502"/>
        <source>Pan the map</source>
        <translation type="unfinished">ຕັດເອົາພາບແຜ່ນທີ່ໆຕ້ອງການເບີ່ງສະເພາະ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="505"/>
        <source>Add Point</source>
        <translation type="unfinished">ເພີ້ມຈຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="506"/>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="507"/>
        <source>Capture Points</source>
        <translation type="unfinished">ຈັບຈຸດຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="510"/>
        <source>Delete Point</source>
        <translation type="unfinished">ໍລືບຈຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="511"/>
        <source>Delete Selected</source>
        <translation type="unfinished">ລືບໂຕຖືກເລືອກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="559"/>
        <source>Linear</source>
        <translation>ເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="560"/>
        <source>Helmert</source>
        <translation>Helmert</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="198"/>
        <source>Choose a name for the world file</source>
        <translation type="unfinished">ເລື້ອກເອົາຊື່ຂອງແຟ້ມໂລກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>-modified</source>
        <comment>
Georeferencer:QgsPointDialog.cpp - used to modify a user given filename
</comment>
        <translation type="obsolete">-ດັດແປງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="265"/>
        <source>Warning</source>
        <translation type="unfinished">ເຕືອນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modifed raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;ການປ່ຽນແປງຂອງ Helmert ຕ້ອງການໆດັດແກ້ຢູ່ໃນລະດັບຊັ້ນຂອງ Raster,້າ&lt;/p&gt;&lt;pRaster ທີ່ໄດ້ຮັບການດັດແກ້ຈະຖືກຈັດເກັບໃຫ້ເປັນແຟ້ມໃໝ່ແລະແຟ້ມໂລກ (World) ຈະສ້າງແຟ້ມໃໝ່ແທນຂົ້າ&lt;/pທ່ານໝັ້ນໃຈບໍ່ວ່ານີ້ຄື່ຊິ່ງທີ່ທ່ານຕ້ອງການ?&gt;&lt;p&gt;&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="280"/>
        <source>Affine</source>
        <translation>ການຜູກພັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="290"/>
        <source>Not implemented!</source>
        <translation>ບໍ່ໄດ້ລົງມືປະຕິບັດ!</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="285"/>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;ປຽນເເປງການຜູກພັນ ຈໍາເປັນມີປຽນເເຟ້ມຂອງ raster ເດີມ. ເເຕ່ມັນບໍ່ທັນເຂົ້າກັນ.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="292"/>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="293"/>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation>ການປ່ຽນແປງຍັງບໍ່ທັນເຂົ້າກັນ.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="324"/>
        <source>Error</source>
        <translation>ຂໍ້ຜິດພາດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="325"/>
        <source>Could not write to </source>
        <translation>ບໍ່ສາມາດຂຽນໄປຍັງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="272"/>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>ແຟ້ມທັງໝົດທີ່ຖືກດັດແກ້ປະຈຸບັນຈະຖືກຂຽນໃຫ້ເປັນ ຮູບແບບການຈັດແຟ້ມTIFF.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="216"/>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given filename</comment>
        <translation type="unfinished">-ດັດແປງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="271"/>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Georeferencer</source>
        <translation type="obsolete">Georeferencer</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="65"/>
        <source>Transform type:</source>
        <translation>ປ່ຽນຊະນິດ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="178"/>
        <source>Zoom in</source>
        <translation>ດືງພາບເຂົ້າໃກ້ (Zoom in)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="200"/>
        <source>Zoom out</source>
        <translation>ດືງພາບອອກໄກ (Zoom out)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="222"/>
        <source>Zoom to the raster extents</source>
        <translation>ຊຸມ (Zoom) ໄປຫາຂອບເຂດ Raster</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="244"/>
        <source>Pan</source>
        <translation>Pan</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="105"/>
        <source>Add points</source>
        <translation>ເພີ້ມຈຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="130"/>
        <source>Delete points</source>
        <translation>ລືບຈຸດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Generate world file and load layer</source>
        <translation type="obsolete">ສ້າງແຟ້ມມໂລກ  ແລະ ໂຫຼນີດລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Generate world file</source>
        <translation type="obsolete">ກຳເໜີດແຟ້ມໂລກ </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">ຍົກເລີກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="52"/>
        <source>World file:</source>
        <translation>ແຟ້ມໂລກ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="45"/>
        <source>Modified raster:</source>
        <translation>Raster ທີ່ໄດ້ຮັບການດັດແປງ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Save As...</source>
        <translation type="obsolete">ດເກັບໃຫ້ເປັນ...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="13"/>
        <source>Reference points</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="38"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="75"/>
        <source>Create</source>
        <translation type="unfinished">ສ້າງ</translation>
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
        <location filename="" line="135533324"/>
        <source>Form3</source>
        <translation type="obsolete">ແບບ3</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Symbol Style</source>
        <translation type="obsolete">ຮູບແບບສັນຍາລັກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Scale</source>
        <translation type="obsolete">ມາດຕາສ່ວນ</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="141"/>
        <source>Unable to access relation</source>
        <translation>ບໍ່ສາມາດເຂົ້າໄປຫາສາຍພົວພັນ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="121"/>
        <source>Unable to access the </source>
        <translation>ບໍ່ສາມາດເຂົ້າໄປຫາ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="144"/>
        <source> relation.
The error message from the database was:
</source>
        <translation>ສາຍພົວພັນ.
ຂໍ້ຄວາມສະແດງຂໍ້ຜິດພາດຈາກຖານຂໍ້ມູນແມ່ນ:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="105"/>
        <source>No GEOS Support!</source>
        <translation>ບໍ່ເຂົ້າກັນກັບ GEOS!</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="109"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>ການຕິດຕັ້ງ PostGIS ຂອງທ່ານ  ບໍ່ເຂົ້າກັນກັບ GEOS ການເລື້ອກຈຸດເດັ່ນແລະຮູບປະພັນສັນຖານຈະບໍ່ເຮັດວຽກຢ່າງເໝາະສົມ。ກະລຸນາຕິດຕັ້ງ PostGIS ເພື່ອໃຫ້ເຂົາກັນກັບ GEOS (http://geos.refractions.net)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="907"/>
        <source>No suitable key column in table</source>
        <translation>ບໍ່ມີຖັນກຸນແຈທີ່ເໝາະສົມຢູ່ໃນຕາຕະລາງ້າ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="911"/>
        <source>The table has no column suitable for use as a key.

Qgis requires that the table either has a column of type
int4 with a unique constraint on it (which includes the
primary key) or has a PostgreSQL oid column.
</source>
        <translation>ຕາຕະລາງບໍ່ມີຖັນທີ່ເໝາະສົມເພື່ອໃຊ້ເປັນກຸນແຈ. 

Qgis ຕ້ອງການທັງຕາຕະລາງທີ່ມີຖັນຊະນິດ int4 ກັບ ຂໍ້ຈຳກັດພຽງນຶ່ງດຽວຢູ່ບົນຕາຕະລາງ(ຊຶ່ງລວມທັງກຸນແຈເບຶ້ອງຕົ້ນ) ແລະ ຖັນ PostgreSQL oid. </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="952"/>
        <source>The unique index on column</source>
        <translation>ດັດສະນີຊີບອກພຽງນຶ່ງດຽວຢູ່ບົນຖັນ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="954"/>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation>ບໍ່ເໝາະສົມຍັ້ອນວ່າ Qgis ປະຈຸບັນບໍ່ເຂົ້າກັນກັບຖັນຊະນິດ non-int4 ຊຶ່ງເປັນກກຸນແເຂົ້າໄປໃນຕາຕະລາງຈາ.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="977"/>
        <source>and </source>
        <translation>ແລະ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="983"/>
        <source>The unique index based on columns </source>
        <translation>ດັດສະນີຊີ້ບອກພຽງນຶ່ງດຽວອີງຕາມຖັນ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="985"/>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation>ໃຊ້ບໍ່ໃດ້ເພາະວ່າ Qgis ທັນຊຸກຍູ້ຫລາຍຖັນເທື່ອ.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1028"/>
        <source>Unable to find a key column</source>
        <translation>ບໍ່ສາມາດພົບກຸນແຈຖັນ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1108"/>
        <source> derives from </source>
        <translation>ສືບມາຈາກ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1112"/>
        <source>and is suitable.</source>
        <translation>ແລະເໝາະສົມ.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1116"/>
        <source>and is not suitable </source>
        <translation>ແລະບໍ່ເໝາະສົມ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1117"/>
        <source>type is </source>
        <translation>ຊະນິດແມ່ນ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1119"/>
        <source> and has a suitable constraint)</source>
        <translation>ແລະບໍ່ມີຂໍ້ຈຳກັດທີ່ເໝາະສົມ)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1121"/>
        <source> and does not have a suitable constraint)</source>
        <translation>ບໍ່ມີຂໍ້ຈຳກັດທີ່ເໝາະສົມ)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1221"/>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation>ການກວດເບິ່ງຮູບພາບທີ່ທ່ານໄດ້ເລື້ອກມີຖັນດັ່ງຕໍ່ໄປນີ້,ບໍ່ມີອັນໃດທີ່ເຮັດໃຫ້ເງື່ອນໄຂຂ້າງເທິງນັ້ນໄດ້ຮັບຄວາມເພິ່ງພໍໃຈ:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1227"/>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation>Qgis ຕ້ອງການໃຫ້ເບີງມີຖັນທີ່ໃຊ້ເປັນກຸນແຈ. ຖັນນີ້ເເມ່ນມາຈາກຖັນຕາຕະລາງຊະນິດ int4 ເເລະ ຕ້ອງເປັນກຸນແຈຫລັກທີ້ມີການບັງຄັບ, ຫລື ຈະເປັນ ຖັນ PostgreSQL oid. ໃຫ້ທໍາງານດີ ຖັນຕ້ອງເເມ່ນເເບບດັດຊະນີ. </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1228"/>
        <source>The view </source>
        <translation>ການກວດເບິ່ງຮູບພາບ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1229"/>
        <source>has no column suitable for use as a unique key.
</source>
        <translation>ບໍ່ມີຖັນທີ່ເໝາະສົມເພື່ອໃຊ້ເປັນລູກກຸນແຈ້ພຽງແຕ່ນຶ່ງດຽວ.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1230"/>
        <source>No suitable key column in view</source>
        <translation>ບໍ່ມີຖັນທີ່ເປັນລູກກຸນແຈທີ່ເໝາະສົມໃຫ້ເບິ່ງ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2530"/>
        <source>Unknown geometry type</source>
        <translation>ຊະນິດຂອງເລຂາຄະນິດບໍ່ຮູ້</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2531"/>
        <source>Column </source>
        <translation type="unfinished">ຖັນ </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2541"/>
        <source> in </source>
        <translation>ໃນ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2533"/>
        <source> has a geometry type of </source>
        <translation>ມີຊະນິດຄະນິດສາດຂອງ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2533"/>
        <source>, which Qgis does not currently support.</source>
        <translation>, Qgis ຊຶ່ງບໍ່ເຂົ້າກັນກັບ.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2542"/>
        <source>. The database communication log was:
</source>
        <translation>. ບັນທຶກການຕິດຕໍ່ຖານຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2543"/>
        <source>Unable to get feature type and srid</source>
        <translation>ບໍ່ສາມາດໄດ້ຮັບຊະນິດແລະ srid ຂອງຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1207"/>
        <source>Note: </source>
        <translation>ຂໍ້ສັງເກດ:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1209"/>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation>ຂໍ້ມູນທີ່ປາກົດອອກມາເບື້ອງຕົ້ນເໝາະສົມແຕ່ບໍ່ມີຂໍ້ມູນນຶ່ງດຽວ,ດັ່ງນັ້ນມັນຈື່ງບໍ່ເໝາະສົມ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>INSERT error</source>
        <translation type="obsolete">ມີຂໍ້ຜິດພາດເກີດຂື້ນກັບການສອດຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>An error occured during feature insertion</source>
        <translation type="obsolete">ມີຂໍ້ຜິດພາດເກີດຂື້ນລະຫວ່າງສອດຈຸດເດັ່ນເຂົ້າໄປ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>DELETE error</source>
        <translation type="obsolete">ມີຂໍ້ຜິດພາດເກີດຂື້ນກັບການລົບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>An error occured during deletion from disk</source>
        <translation type="obsolete">ມີຂໍ້ຜິດພາດເກີດຂື້ນລະຫວ່າງລືບຈາກແຜນດີດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>PostGIS error</source>
        <translation type="obsolete">ຂໍ້ຜິດ ັບ PostGIS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>When trying: </source>
        <translation type="obsolete">ເມື່ອພະຍາຍາມ:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2540"/>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation>Qgis ບໍ່ສາມາດກຳໜົດຊະນິດແລະ srid ຂອງຖັນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>An error occured contacting the PostgreSQL database</source>
        <translation type="obsolete">າມີຂໍ້ຜິດພາດເກີດຂື້ໃນລະຫວ່າງນຕິດຕໍ່ຖານຂໍ້ມູນຂອງ PostgreSQL</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>The PostgreSQL database returned: </source>
        <translation type="obsolete">ຖານຂໍ້ມູນຂອງ PostgreSQL ກັບຄືນມາ:</translation>
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
        <translation>ຄຸນລັກສະນະຂອງໜ້າວຽກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Map Units</source>
        <translation type="obsolete">ຫົວໜ່ວຍແຜນທີ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="157"/>
        <source>Meters</source>
        <translation>ແມັດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="167"/>
        <source>Feet</source>
        <translation>ຟຸດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="174"/>
        <source>Decimal degrees</source>
        <translation>ເລກ ອັງສາ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="54"/>
        <source>Default project title</source>
        <translation>ຫົວຂໍ້ໜ້າວຽກຕາຍຕົວ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="32"/>
        <source>General</source>
        <translation type="unfinished">ທົວໄປ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Line Width:</source>
        <translation type="obsolete">ຄວາມໜາຂອງເສັ້ນ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Line Colour:</source>
        <translation type="obsolete">ສີເສັ້ນ: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="196"/>
        <source>Automatic</source>
        <translation>ອັດຕະໂນມັດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="190"/>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>ອັດຕະໂນມັດ ປັບ ຕໍາເເຫນ່ງໂຕເລກໃນທີ່ສະເເດງຕໍາເເຫມງຂອງມາວ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="193"/>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>ຕໍາເເຫນ່ງໂຕເລກໃດ້ຖຶກໃຊ້ ກັບຕອນສະເເດງຕໍາເເຫມງຂອງມາວ  ເເມ່ນເເບບອັດຕະໂນມັດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="212"/>
        <source>Manual</source>
        <translation>ດ້ວຍມື</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="209"/>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>ປັບ ຕໍາເເຫນ່ງໂຕເລກໃນທີ່ສະເເດງຕໍາເເຫມງຂອງມາວ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="222"/>
        <source>The number of decimal places for the manual option</source>
        <translation>ຕໍາເເຫນ່ງເລກສໍາລັບເລື້ອກດ້ວຍມື</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="235"/>
        <source>decimal places</source>
        <translation>ຕໍາເເຫນ່ງ ເລກ </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Map Appearance</source>
        <translation type="obsolete">ການປະກົດຕົວຂອງແຜນທີ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Selection Color:</source>
        <translation type="obsolete">ການເລື້ອກສີ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Project Title</source>
        <translation type="obsolete">ຫົວຂໍ້ໜ້າວຽກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="282"/>
        <source>Projection</source>
        <translation>ຄາດຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="300"/>
        <source>Enable on the fly projection</source>
        <translation>ສາມາດ ຄາດຫມາຍ ຕອນບີນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Background Color:</source>
        <translation type="obsolete">ສີພື້ນ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="184"/>
        <source>Precision</source>
        <translation>ຄວາມທ່ຽງຕົງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="251"/>
        <source>Digitizing</source>
        <translation type="unfinished">ການປ້ອນຂໍ້ມູນເປັນຕົວເລກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="51"/>
        <source>Descriptive project name</source>
        <translation>ຊື່ໃຊ້ອະທິບາຍລາຍການໜ້າວຽກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Line width in pixels</source>
        <translation type="obsolete">ຄວາມໜາຂອງເສັ້ນເປັນ Pixels</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Snapping tolerance in map units</source>
        <translation type="obsolete">ຄວ້າເອົວຄວາມອົດທົນໃນ ຫມວດເເຜນທີ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Snapping Tolerance (in map units):</source>
        <translation type="obsolete">ຄວ້າເອົວຄວາມອົດທົນ (ໃນ ຫມວດເເຜນທີ່)</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="257"/>
        <source>Enable topological editing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="264"/>
        <source>Avoid intersections of new polygons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="271"/>
        <source>Snapping options...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelector</name>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="783"/>
        <source>QGIS SRSID: </source>
        <translation type="unfinished">QGIS SRSID: </translation>
    </message>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="784"/>
        <source>PostGIS SRID: </source>
        <translation type="unfinished">PostGIS SRID: </translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelectorBase</name>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="19"/>
        <source>Projection Selector</source>
        <translation>ໂຕເລື້ອກ ການຄາດຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="52"/>
        <source>Projection</source>
        <translation>ການຄາດຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="82"/>
        <source>Search</source>
        <translation type="unfinished">ຄົ້ນຫາ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="112"/>
        <source>Find</source>
        <translation>ຊອກຫາ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="164"/>
        <source>Postgis SRID</source>
        <translation type="unfinished">Postgis SRID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="151"/>
        <source>EPSG ID</source>
        <translation type="unfinished">EPSG ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="138"/>
        <source>QGIS SRSID</source>
        <translation type="unfinished">QGIS SRSID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="125"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="221"/>
        <source>Spatial Reference System</source>
        <translation>ລະບົບອ້າງອິງໄລຍະ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="226"/>
        <source>Id</source>
        <translation type="unfinished">Id</translation>
    </message>
</context>
<context>
    <name>QgsPythonDialog</name>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="13"/>
        <source>Python console</source>
        <translation>ຈໍ Python</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans Condensed&apos;; font-size:10pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;To access Quantum GIS environment from this python console use object &lt;span style=&quot; font-weight:600;&quot;&gt;iface&lt;/span&gt; from global scope which is an instance of QgisInterface class.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Usage e.g.: iface.zoomFull()&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans Condensed&apos;; font-size:10pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;To access Quantum GIS environment from this python console use object &lt;span style=&quot; font-weight:600;&quot;&gt;iface&lt;/span&gt; from global scope which is an instance of QgisInterface class.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Usage e.g.: iface.zoomFull()&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="58"/>
        <source>&gt;&gt;&gt;</source>
        <translation>&gt;&gt;&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans Condensed&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans Condensed&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="33"/>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsQuickPrint</name>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="845"/>
        <source> km</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="850"/>
        <source> mm</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="855"/>
        <source> cm</source>
        <translation type="unfinished">ຊມ</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="859"/>
        <source> m</source>
        <translation type="unfinished">ມ</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="864"/>
        <source> miles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="869"/>
        <source> mile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="874"/>
        <source> inches</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="879"/>
        <source> foot</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="883"/>
        <source> feet</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="888"/>
        <source> degree</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="890"/>
        <source> degrees</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="893"/>
        <source> unknown</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.h" line="513"/>
        <source>Not Set</source>
        <translation>ບໍ່ໄດ້ຕັ້ງ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3425"/>
        <source>Driver:</source>
        <translation>ໂປຼກຼາມຄວບຄຸມອຸປະກອນຕ່າງໆ:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3502"/>
        <source>Dimensions:</source>
        <translation>ຄະໜາດ:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3505"/>
        <source>X: </source>
        <translation type="unfinished">X: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3506"/>
        <source> Y: </source>
        <translation type="unfinished"> Y: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3506"/>
        <source> Bands: </source>
        <translation>ແທບເສັ້ນ:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3621"/>
        <source>Origin:</source>
        <translation>ຈຸດເລີ້ມຕົ້ນ:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3630"/>
        <source>Pixel Size:</source>
        <translation>ຄະໜາດ Pixel:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2202"/>
        <source>Raster Extent: </source>
        <translation>ຂອບເຂດ Raster:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2205"/>
        <source>Clipped area: </source>
        <translation>ພື້ນທີ່ ຄັດ:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3572"/>
        <source>Pyramid overviews:</source>
        <translation>ພາບປີລະມິດໂດຍສະຫລຸບ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Property</source>
        <translation type="obsolete">ຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Value</source>
        <translation type="obsolete">ຄ່າ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4793"/>
        <source>Band</source>
        <translation type="unfinished">ແທບເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3655"/>
        <source>Band No</source>
        <translation>ບໍ່ມີ ແທບເສັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3667"/>
        <source>No Stats</source>
        <translation type="unfinished">ບໍ່ມີສະຖານະ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3670"/>
        <source>No stats collected yet</source>
        <translation type="unfinished">ຍັງບໍ່ທັນເກັບກຳສະຖານະ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3680"/>
        <source>Min Val</source>
        <translation type="unfinished">ຄ່າຕຳ່ສຸດ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3688"/>
        <source>Max Val</source>
        <translation type="unfinished">ຄ່າສູງສຸດ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3696"/>
        <source>Range</source>
        <translation>ຂອບເຂດ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3704"/>
        <source>Mean</source>
        <translation>ຄ່າສະເລ່ຍ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3712"/>
        <source>Sum of squares</source>
        <translation type="unfinished">ຈຳນວນແມັດກ້ອນ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3720"/>
        <source>Standard Deviation</source>
        <translation type="unfinished">ມາດຕະຖານຄວາມຜິດພ້ຽນ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3728"/>
        <source>Sum of all cells</source>
        <translation>ຈຳນວນເຊວ (Cell) ທັງໝົດ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3736"/>
        <source>Cell Count</source>
        <translation>ການນັບເຊວ (cell)</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3528"/>
        <source>Data Type:</source>
        <translation type="unfinished">ຊະນິດຂໍ້ມູນ:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3534"/>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation type="unfinished">GDT_Byte - 8 bit unsigned integer</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3537"/>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation type="unfinished">GDT_UInt16 - 16 bit unsigned integer </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3540"/>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation type="unfinished">GDT_Int16 - 16 bit signed integer </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3543"/>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation type="unfinished">GDT_UInt32 - 32 bit unsigned integer </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3546"/>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation type="unfinished">GDT_Int32 - 32 bit signed integer </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3549"/>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation type="unfinished">GDT_Float32 - 32 bit floating point </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3552"/>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation type="unfinished">GDT_Float64 - 64 bit floating point </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3555"/>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation type="unfinished">GDT_CInt16 - Complex Int16 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3558"/>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation type="unfinished">GDT_CInt32 - Complex Int32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3561"/>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation type="unfinished">GDT_CFloat32 - Complex Float32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3564"/>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation type="unfinished">GDT_CFloat64 - Complex Float64 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3567"/>
        <source>Could not determine raster data type.</source>
        <translation>ບໍ່ສາມາດລະບຸຊະນິດຂໍ້ມູນຂອງ Raster</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3822"/>
        <source>Average Magphase</source>
        <translation>ຄ່າສະເລ່ຍ Magphase</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3827"/>
        <source>Average</source>
        <translation>ຄ່າສະເລຍ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3593"/>
        <source>Layer Spatial Reference System: </source>
        <translation type="unfinished">ລະດັບຊັ້ນອ້າງອິງໄລຍະ:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4749"/>
        <source>out of extent</source>
        <translation type="unfinished">ອອກນອກຂອບເຂດ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4787"/>
        <source>null (no data)</source>
        <translation type="unfinished">ບໍ່ມີຄ່າ(ຂໍ້ມູນບໍ່ມີ)</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3450"/>
        <source>Dataset Description</source>
        <translation type="unfinished">ລາຍການຊຸດຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3513"/>
        <source>No Data Value</source>
        <translation type="unfinished">ຂໍ້ມູນບໍ່ມີຄ່າ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="242"/>
        <source>and all other files</source>
        <translation type="unfinished">ແລະແຟ້ມອື່ນໆທັງໝົດ</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3471"/>
        <source>Band %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3522"/>
        <source>NoDataValue not set</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;h3&gt;Multiband Image Notes&lt;/h3&gt;&lt;p&gt;This is a multiband image. You can choose to render it as grayscale or color (RGB). For color images, you can associate bands to colors arbitarily. For example, if you have a seven band landsat image, you may choose to render it as:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Visible Blue (0.45 to 0.52 microns) - not mapped&lt;/li&gt;&lt;li&gt;Visible Green (0.52 to 0.60 microns) - not mapped&lt;/li&gt;&lt;/li&gt;Visible Red (0.63 to 0.69 microns) - mapped to red in image&lt;/li&gt;&lt;li&gt;Near Infrared (0.76 to 0.90 microns) - mapped to green in image&lt;/li&gt;&lt;li&gt;Mid Infrared (1.55 to 1.75 microns) - not mapped&lt;/li&gt;&lt;li&gt;Thermal Infrared (10.4 to 12.5 microns) - not mapped&lt;/li&gt;&lt;li&gt;Mid Infrared (2.08 to 2.35 microns) - mapped to blue in image&lt;/li&gt;&lt;/ul&gt;</source>
        <translation type="obsolete">&lt;h3&gt;Multiband Image Notes&lt;/h3&gt;&lt;p&gt;This is a multiband image. You can choose to render it as grayscale or color (RGB). For color images, you can associate bands to colors arbitarily. For example, if you have a seven band landsat image, you may choose to render it as:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Visible Blue (0.45 to 0.52 microns) - not mapped&lt;/li&gt;&lt;li&gt;Visible Green (0.52 to 0.60 microns) - not mapped&lt;/li&gt;&lt;/li&gt;Visible Red (0.63 to 0.69 microns) - mapped to red in image&lt;/li&gt;&lt;li&gt;Near Infrared (0.76 to 0.90 microns) - mapped to green in image&lt;/li&gt;&lt;li&gt;Mid Infrared (1.55 to 1.75 microns) - not mapped&lt;/li&gt;&lt;li&gt;Thermal Infrared (10.4 to 12.5 microns) - not mapped&lt;/li&gt;&lt;li&gt;Mid Infrared (2.08 to 2.35 microns) - mapped to blue in image&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;h3&gt;Paletted Image Notes&lt;/h3&gt; &lt;p&gt;This image uses a fixed color palette. You can remap these colors in different combinations e.g.&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Red - blue in image&lt;/li&gt;&lt;li&gt;Green - blue in image&lt;/li&gt;&lt;li&gt;Blue - green in image&lt;/li&gt;&lt;/ul&gt;</source>
        <translation type="obsolete">&lt;h3&gt;ຂໍ້ສັງເກດກະດານແຕ້ມຮູບ&lt;/h3&gt; &lt;p&gt;ຮູບນີ້ໃຊ້ກະດານສີແຕ້ມຮູບຕາຍຕົວ,ທ່ານສາມາດແຕ້ມແຜນທີ່ໃໝ່ໂດຍນຳໃຊ້ສີເລົ່ານີ້ຕາມການປະສົມສີທີ່ຫຼາກຫຼາຍ&lt;/p&gt;&lt;ul&gt;&lt;li&gt;ແດງ -ຟ້າ&lt;/li&gt;&lt;li&gt;ຢູ່ໃນຮູບ&lt;/li&gt;&lt;li&gt;ຂຽວ-ຟ້າຢູ່ໃນຮູບ,ຟ້າ-ຂຽວຢູ່ໃນຮູບ&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;h3&gt;Grayscale Image Notes&lt;/h3&gt; &lt;p&gt;You can remap these grayscale colors to a pseudocolor image using an automatically generated color ramp.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;h3&gt;ຄໍາເຕືອນ ຮູບ ສີຂີ້ເທົ່າ &lt;/h3&gt; &lt;p&gt;You can remap these grayscale colors to a pseudocolor image using an automatically generated color ramp.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1032"/>
        <source>Grayscale</source>
        <translation type="unfinished">ມາດຕາສ່ວນສີຂີ້ເທົ່າ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2412"/>
        <source>Pseudocolor</source>
        <translation>ສີປອມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2412"/>
        <source>Freak Out</source>
        <translation>ຜິດປົກກະຕິ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="177"/>
        <source>Palette</source>
        <translation type="unfinished">ຈານປະສົມສີ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="56"/>
        <source>Not Set</source>
        <translation>ບໍ່ໄດ້ຕັ້ງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="825"/>
        <source>Columns: </source>
        <translation>ຖັນ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="826"/>
        <source>Rows: </source>
        <translation>ແຖວ: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="827"/>
        <source>No-Data Value: </source>
        <translation>ຂໍ້ມູນບໍ່ມີຄ່າ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="827"/>
        <source>n/a</source>
        <translation>ບໍ່ເຫມາະສົມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1849"/>
        <source>Write access denied</source>
        <translation type="unfinished">ປະຕິເສດການຂຽນເຂົ້າ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1849"/>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation>ປະຕິເສດການຂຽນເຂົ້າ. ປັບການອະນຸຍາດຂອງແຟ້ມແລ້ວລອງໃໝ່.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1594"/>
        <source>Building pyramids failed.</source>
        <translation>ການສ້າງຮູບປີຣະມິດລົ້ມເຫລວ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1590"/>
        <source>The file was not writeable. Some formats can not be written to, only read. You can also try to check the permissions and then try again.</source>
        <translation>ແຟ້ມນີ້ບໍ່ສາມາດຂຽນໄດ້.ຮູບແບບການຈັດລະບົບຄອມພີວເຕີບໍ່ສາມາດຖືກຂຽນລົງໄປໄດ້.ໄດ້ພ່ຽງແຕ່ອ່ານ.ທ່ານສາມາດກວດເບິ່ງການໃຫ້ອະນຸຍາດແລ້ວລອງໃໝ່.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1595"/>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation>ການສ້າງພາບປີຣະມິດ (Pyramid) ໂດຍສະຫຼຸບບໍ່ເຂົ້າກັນກັບຊະຂອງ Raster ນີ້.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1714"/>
        <source>Custom Colormap</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2777"/>
        <source>No Stretch</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2782"/>
        <source>Stretch To MinMax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2787"/>
        <source>Stretch And Clip To MinMax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2792"/>
        <source>Clip To MinMax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1427"/>
        <source>Discrete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="907"/>
        <source>Linearly</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2622"/>
        <source>Equal interval</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2644"/>
        <source>Quantiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1821"/>
        <source>Red</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1821"/>
        <source>Green</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1821"/>
        <source>Blue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="332"/>
        <source>Description</source>
        <translation type="unfinished">ລາຍການ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="333"/>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="334"/>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="335"/>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="336"/>
        <source>Please note that building pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="337"/>
        <source>Please note that building pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1838"/>
        <source>Percent Transparent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1834"/>
        <source>Gray</source>
        <translation type="unfinished">ສີຂີ້ເທົ່າ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1838"/>
        <source>Indexed Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2774"/>
        <source>User Defined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="819"/>
        <source>No-Data Value: Not Set</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1806"/>
        <source>Save file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2308"/>
        <source>Textfile (*.txt)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1818"/>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2308"/>
        <source>Open file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2381"/>
        <source>Import Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2381"/>
        <source>The following lines contained errors

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2386"/>
        <source>Read access denied</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2386"/>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2412"/>
        <source>Color Ramp</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2999"/>
        <source>Default Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2984"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3005"/>
        <source>QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3006"/>
        <source>Unknown style format: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="13"/>
        <source>Raster Layer Properties</source>
        <translation>ຄຸນລັກສະນະຂອງລະດັບຊັ້ນ Raster</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1306"/>
        <source>General</source>
        <translation type="unfinished">ທົວໄປ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Layer Source:</source>
        <translation type="obsolete">ແຫຼ່ງລະດັບຊັ້ນ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Display Name:</source>
        <translation type="obsolete">ສະແດງຊື່:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1570"/>
        <source>Legend:</source>
        <translation type="unfinished">ຄຳບັນຍາຍບົນແຜນທີ່:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1464"/>
        <source>No Data:</source>
        <translation type="unfinished">ບໍ່ມີຂໍ້ມູນ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="32"/>
        <source>Symbology</source>
        <translation>ເຄື້ອງຫມາຍ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Transparency:</source>
        <translation type="obsolete">ຄວາມໂປ່ງໃສ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="881"/>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;ຂວາ&quot;&gt;ເຕັມ&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="835"/>
        <source>None</source>
        <translation type="unfinished">ບໍ່ມີຫຍັງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Invert Color Map</source>
        <translation type="obsolete">ປີ້ນກັບສີແຜນທີ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>0%</source>
        <translation type="obsolete">0%</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Band</source>
        <translation type="obsolete">ແທບເສັ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;&lt;font color=&quot;#00ff00&quot;&gt;Green&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&quot;#00ff00&quot;&gt;ສີຂຽວ&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;&lt;font color=&quot;#ff0000&quot;&gt;Red&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&quot;#ff0000&quot;&gt;ສີແດງ&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;&lt;font color=&quot;#0000ff&quot;&gt;Blue&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&quot;#0000ff&quot;&gt;ສີຟ້າ&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Color</source>
        <translation type="obsolete">ສີ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Gray</source>
        <translation type="obsolete">ສີຂີ້ເທົ່າ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Std Deviations</source>
        <translation type="obsolete">ມາດຖານ (Std) Deviations</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Color Map</source>
        <translation type="obsolete">ແຜນທີ່ສີ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1679"/>
        <source>Metadata</source>
        <translation>Metadata</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1701"/>
        <source>Pyramids</source>
        <translation type="unfinished">ຮູບປີຣະມິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Resampling Method</source>
        <translation type="obsolete">ຮູບແບບໃຫ້ຕົວຢ່າງຄືນໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1790"/>
        <source>Average</source>
        <translation type="unfinished">ໂດຍສະເລຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1795"/>
        <source>Nearest Neighbour</source>
        <translation type="unfinished">ເພື່ອບ້ານໃກ້ຄຽງທີ່ສຸດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Build Pyramids</source>
        <translation type="obsolete">ສ້າງຮູບປີຣະມິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Pyramid Resolutions</source>
        <translation type="obsolete">ລາຍລະອຽດຂອງຮູບປີຣະມິດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1517"/>
        <source>Thumbnail</source>
        <translation>ສັ້ນຯ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1450"/>
        <source>Columns:</source>
        <translation type="unfinished">ຖັນ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1457"/>
        <source>Rows:</source>
        <translation type="unfinished">ແຖວ: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1623"/>
        <source>Palette:</source>
        <translation>ກະດານໃສ່ສີ່ແຕ້ມຮູບ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Maximum 1:</source>
        <translation type="obsolete">ສູງສຸດ 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1380"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>ມາດຕາສ່ວນສູງສຸດຊຶ່ງລະດັບຊັ້ນນີ້ຈະສະແດງ。</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Minimum 1:</source>
        <translation type="obsolete">ຕຳ່ສຸດ 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1403"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation type="unfinished">ມາດຕາສ່ວນຕຳ່ສຸດຊຶ່ງລະດັບຊັ້ນນີ້ຈະສະແດງ。</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1813"/>
        <source>Histogram</source>
        <translation>Histogram</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1907"/>
        <source>Options</source>
        <translation type="unfinished">ທາງເລື້ອກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Out Of Range OK?</source>
        <translation type="obsolete">ອອກນອກຂອບເຂດ ໃດ້ບໍ່?</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Allow Approximation</source>
        <translation type="obsolete">ໃຫ້ອະນຸຍາດໂດຍປະມານ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1862"/>
        <source>Chart Type</source>
        <translation type="unfinished">ຊະນິດຂອງແຜນວາດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Bar Chart</source>
        <translation type="obsolete">ທ້ອນຂອງແຜນວາດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Line Graph</source>
        <translation type="obsolete">ເສັ້ນກຼາບ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1900"/>
        <source>Refresh</source>
        <translation type="unfinished">ກະຕຸ້ນໃຫມ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Spatial Reference System</source>
        <translation type="obsolete">ລະບົບອ້າງອິງໄລຍະ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1342"/>
        <source>Change</source>
        <translation type="unfinished">ປ່ຽນແປງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Display</source>
        <translation type="obsolete">ສະແດງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Grayscale Image</source>
        <translation type="obsolete">ຮູບມາດຕາສ່ວນເປັນສີຂີ້ເທົ່າ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Color Image</source>
        <translation type="obsolete">ຮູບເປັນສີ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot;font-size:9pt;font-family:Sans Serif&quot;&gt;
&lt;p style=&quot;margin-top:14px&quot; dir=&quot;ltr&quot;&gt;&lt;span style=&quot;font-weight:600&quot;&gt;Notes&lt;/span&gt;&lt;/p&gt;
&lt;/body&gt;&lt;/html&gt;
</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot;font-size:9pt;font-family:Sans Serif&quot;&gt;
&lt;p style=&quot;margin-top:14px&quot; dir=&quot;ltr&quot;&gt;&lt;span style=&quot;font-weight:600&quot;&gt;Notes&lt;/span&gt;&lt;/p&gt;
&lt;/body&gt;&lt;/html&gt;
</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>DebugInfo</source>
        <translation type="obsolete">ຂໍ້ມູນແກ້ໄຂປັນຫາ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Scale Dependent Visibility</source>
        <translation type="obsolete">ການເບີ່ງຕາມມາດຕາສ່ວນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Column Count:</source>
        <translation type="obsolete">ການນັບຖັນ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Transparent</source>
        <translation type="obsolete">ໂປ່ງໃສ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot;font-size:9pt;font-family:Sans Serif&quot;&gt;
&lt;p style=&quot;margin-top:18px&quot; dir=&quot;ltr&quot;&gt;&lt;span style=&quot;font-size:14pt;font-weight:600&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;Large resolution raster layers can slow navigation in QGIS. By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom. You must have write access in the directory where the original data is stored to build pyramids. &lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;&lt;span style=&quot;color:#ff0000&quot;&gt;Please note that building pyramids may alter the original data file and once created they cannot be removed.&lt;/span&gt;&lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;&lt;span style=&quot;color:#ff0000&quot;&gt;Please note that building pyramids could corrupt your image - always make a backup of your data first!&lt;/span&gt;&lt;/p&gt;
&lt;/body&gt;&lt;/html&gt;
</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot;font-size:9pt;font-family:Sans Serif&quot;&gt;
&lt;p style=&quot;margin-top:18px&quot; dir=&quot;ltr&quot;&gt;&lt;span style=&quot;font-size:14pt;font-weight:600&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;Large resolution raster layers can slow navigation in QGIS. By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom. You must have write access in the directory where the original data is stored to build pyramids. &lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;&lt;span style=&quot;color:#ff0000&quot;&gt;Please note that building pyramids may alter the original data file and once created they cannot be removed.&lt;/span&gt;&lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;&lt;span style=&quot;color:#ff0000&quot;&gt;Please note that building pyramids could corrupt your image - always make a backup of your data first!&lt;/span&gt;&lt;/p&gt;
&lt;/body&gt;&lt;/html&gt;
</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="38"/>
        <source>Render as</source>
        <translation type="unfinished"></translation>
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
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="546"/>
        <source>Min</source>
        <translation type="unfinished">ຕຳ່ສຸດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="584"/>
        <source>Max</source>
        <translation type="unfinished">ສູງສຸດ</translation>
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
        <translation type="unfinished">ກາຫມາຍເອກະສານ</translation>
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
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="861"/>
        <source> 00%</source>
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
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1098"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
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
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1127"/>
        <source>Colormap</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1185"/>
        <source>Number of entries</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1217"/>
        <source>Delete entry</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1224"/>
        <source>Classify</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1240"/>
        <source>1</source>
        <translation type="unfinished">1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1245"/>
        <source>2</source>
        <translation type="unfinished">2</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2009"/>
        <source>Save As Default</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2016"/>
        <source>Load Style ...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2023"/>
        <source>Save Style ...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRunProcess</name>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="146"/>
        <source>Unable to run command</source>
        <translation type="unfinished">ບໍ່ສາມາດແລ່ນຄຳສັ່ງ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="59"/>
        <source>Starting</source>
        <translation type="unfinished">ເລີ້ມຕົ້ນ</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="115"/>
        <source>Done</source>
        <translation type="unfinished">ທຳສຳເລັດ</translation>
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
        <translation type="unfinished">ທ່ານໝັ້ນໃຈບໍ່ວ່າຈະເອົາອອກ</translation>
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
        <translation type="unfinished">ການເຊື່ອມຕໍ່ໜ່ວຍເຄືອຄ່າຍແມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="49"/>
        <source>&amp;New</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="59"/>
        <source>Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="69"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="95"/>
        <source>C&amp;onnect</source>
        <translation type="unfinished">ເ&amp;ຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="108"/>
        <source>Offerings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="118"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="123"/>
        <source>Id</source>
        <translation type="unfinished"></translation>
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
        <translation>ແມັດ/ກມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="281"/>
        <source> feet</source>
        <translation type="unfinished">ຫຼາຍຟຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="288"/>
        <source> degrees</source>
        <translation type="unfinished">ຫຼາຍອົງສາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="243"/>
        <source> km</source>
        <translation>ກມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="248"/>
        <source> mm</source>
        <translation> ມມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="253"/>
        <source> cm</source>
        <translation>ຊມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="257"/>
        <source> m</source>
        <translation>ມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="277"/>
        <source> foot</source>
        <translation type="unfinished">ຟຸດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="286"/>
        <source> degree</source>
        <translation type="unfinished">ອົງສາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="291"/>
        <source> unknown</source>
        <translation type="unfinished">ບໍ່ຮູ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Left</source>
        <translation type="unfinished">ຂ້າງເທິງເບື້ອງຊ້າຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="78"/>
        <source>Bottom Left</source>
        <translation type="unfinished">ຂ້າງລຸ່ມເບື້ອງຊ້າຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Right</source>
        <translation type="unfinished">ຂ້າງເທິງເບື້ອງຂວາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Bottom Right</source>
        <translation type="unfinished">ຂ້າງລຸ່ມເບື້ອງຂວາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="81"/>
        <source>Tick Down</source>
        <translation>ຍັບລົງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Tick Up</source>
        <translation>ຍັບຂື້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Bar</source>
        <translation type="unfinished">ທ່ອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Box</source>
        <translation type="unfinished">ກອງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="102"/>
        <source>&amp;Scale Bar</source>
        <translation>&amp;ທ່ອນມາດຕາສ່ວນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="103"/>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation type="unfinished">ສ້າງທ່ອນບອກມາດຕາສ່ວນເຊິ່ງສະແດງຢູ່ພື້ນແຜນທີ່.ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="543"/>
        <source>&amp;Decorations</source>
        <translation type="unfinished">ການຕົກແຕ່ງ</translation>
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
        <translation type="unfinished"></translation>
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
        <translation type="unfinished">ທ່ອນມາດຕາສ່ວນຂອງໂປຼກຼາມເສີ້ມ(Plugin)</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="143"/>
        <source>Top Left</source>
        <translation type="unfinished">ຂ້າງເທິງເບື້ອງຊ້າຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="148"/>
        <source>Top Right</source>
        <translation type="unfinished">ຂ້າງເທິງເບື້ອງຂວາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation type="unfinished">ຂ້າງລຸ່ມເບື້ອງຊ້າຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="158"/>
        <source>Bottom Right</source>
        <translation type="unfinished">ຂ້າງລຸ່ມເບື້ອງຂວາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="94"/>
        <source>Size of bar:</source>
        <translation type="unfinished">ຄະໜາດຂອງທ່ອນ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="254"/>
        <source>Placement:</source>
        <translation type="unfinished">ການປ່ຽນແທນ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="211"/>
        <source>Tick Down</source>
        <translation>ຍັບລົງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="216"/>
        <source>Tick Up</source>
        <translation>ຍັບຂື້ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="221"/>
        <source>Box</source>
        <translation type="unfinished">ກອງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="226"/>
        <source>Bar</source>
        <translation type="unfinished">ທ່ອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="207"/>
        <source>Select the style of the scale bar</source>
        <translation type="unfinished">ເລື້ອກຮູບແບບຂອງທ່ອນມາດຕາສ່ວນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="127"/>
        <source>Colour of bar:</source>
        <translation type="unfinished">ສີຂອງທ່ອນ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="192"/>
        <source>Scale bar style:</source>
        <translation type="unfinished">ຮູບແບບຂອງທ່ອນມາດຕາສ່ວນ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="174"/>
        <source>Enable scale bar</source>
        <translation type="unfinished">ເປີດທ່ອນມາດຕາສ່ວນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="109"/>
        <source>Automatically snap to round number on resize</source>
        <translation>ຖ່າຍຮູບແບບໂອໂຕແມດຕິກເພື່ອໃຫ້ເປັນຕົວເລກມົນເມື່ອປ່ຽນຄະໜາດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="76"/>
        <source>Click to select the colour</source>
        <translation type="unfinished">ກົດເພື່ອເລື້ອກສີ</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="274"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message numerus="yes">
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="171"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform>ພົບຈຸດເດັ່ນທີ່ແທດເຫມາະກັນ %d</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="173"/>
        <source>No matching features found.</source>
        <translation>ບໍ່ພົບຈຸດເດັ່ນທີ່ແທດເຫມາະກັນໃ</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="174"/>
        <source>Search results</source>
        <translation type="unfinished">ຊອກຫາຜົນຮັບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="183"/>
        <source>Search string parsing error</source>
        <translation>ຊອກຫາຄໍາຊັ່ງຜີດພາດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="239"/>
        <source>No Records</source>
        <translation type="unfinished">ບໍ່ມີການບັນທຶກ</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="239"/>
        <source>The query you specified results in zero records being returned.</source>
        <translation>ຄໍາຊັ່ງທີລະບຸນັ້ນຜົນເເມ່ນ 0 ຂໍ້ມູນກັບຄືນມາ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="41"/>
        <source>Search query builder</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Are you sure you want to remove the </source>
        <comment>








#, fuzy
</comment>
        <translation type="obsolete">ທ່ານໝັ້ນໃຈບໍ່ວ່າຈະເອົາອອກ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="173"/>
        <source> connection and all associated settings?</source>
        <translation type="unfinished">ການເຊື່ອມຕໍ່ແລະການຕັ້ງຄ່າທີ່ພົວພັນກັບມັນທັງໝົດ?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="174"/>
        <source>Confirm Delete</source>
        <translation type="unfinished">ຢືນຢັນລືບ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="399"/>
        <source>WMS Provider</source>
        <translation>ຜູ້ສະໜອງ WMS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="401"/>
        <source>Could not open the WMS Provider</source>
        <translation>ບໍ່ສາມາດເປີດຜູ້ສະໜອງ WMS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="410"/>
        <source>Select Layer</source>
        <translation type="unfinished">ເລື້ອກລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="410"/>
        <source>You must select at least one layer first.</source>
        <translation>ທ່ານຕ້ອງເລື້ອກຢ່າງໜ້ອຍນຶ່ງລະດັບຊັ້ັ້ນກ່ອນ.</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsserversourceselect.cpp" line="520"/>
        <source>Coordinate Reference System (%1 available)</source>
        <translation type="unfinished">
            <numerusform>ລະບົບປະສານງານທີ່ເປັນຕົວອ້າງອີງ (ມີຢູ່%1)</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="679"/>
        <source>Could not understand the response.  The</source>
        <translation>ບໍ່ເຂົ້າໃຈຕົວຕອບສະໜອງ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="680"/>
        <source>provider said</source>
        <translation type="unfinished">ຜູ້ສະໜອງເວົ້າວ່າ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="731"/>
        <source>WMS proxies</source>
        <translation type="unfinished">WMS Proxies</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;p&gt;Several WMS servers have been added to the server list. Note that the proxy fields have been left blank and if you access the internet via a web proxy, you will need to individually set the proxy fields with appropriate values.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;ໜ່ວຍເຄືອຄາຍແມ່ຂອງ WMS ຫຼາຍຕົວໄດ້ຖືກເພີ້ມເຂົ້າໄປໃນລາຍຊື່ຂອງໜ່ວຍເຄື່ອຄ່າຍແມ່.ສັງເກດເບິ່ງວ່າ,ບ່ອນວ່າງຂອງ Proxy ໄດ້ປະບ່ອນວ່າງແລະຖ້າຫາກວ່າທ່ານເຂົ້າໄປອີນເຕີເນກ (Internet) ໂດຍຜ່ານເວບພຣອກຊີ (Web Proxy), ທ່ານຈຳເປັນຕ້ອງຕັ້ງບ່ອນວ່າງຂອງ proxy ໄປແຕ່ລະຕົວດ້ວຍຄ່າທີ່ເໝາະສົມ.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="414"/>
        <source>Coordinate Reference System</source>
        <translation type="unfinished">ລະບົບປະສານງານທີ່ເປັນຕົວອ້າງອີງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="414"/>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation type="unfinished">ບໍ່ມີລະບົບຕົວປະສານງານໄວ້ອ້າງອິງສຳລັບລະດັບຊັ້ນແຕ່ລະອັນທີ່ທ່ານໄດ້ຕັ້ງ.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="173"/>
        <source>Are you sure you want to remove the </source>
        <translation type="unfinished">ທ່ານໝັ້ນໃຈບໍ່ວ່າຈະເອົາອອກ</translation>
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
        <translation>ເພີ້ມລະດັບຊັ້ນຈາກໜ່ວຍເຄືອຄ່າຍແມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="309"/>
        <source>C&amp;lose</source>
        <translation>&amp;ປິດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="312"/>
        <source>Alt+L</source>
        <translation type="unfinished">Alt+L</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="296"/>
        <source>Help</source>
        <translation type="unfinished">ຊ່ວຍເຫຼືອ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="299"/>
        <source>F1</source>
        <translation type="unfinished">F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="270"/>
        <source>Image encoding</source>
        <translation type="unfinished">ການໃສ່ລະຫັດຮູບ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="203"/>
        <source>Layers</source>
        <translation type="unfinished">ລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="230"/>
        <source>ID</source>
        <translation type="unfinished">ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="235"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="240"/>
        <source>Title</source>
        <translation type="unfinished">ຫົວຂໍ້</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="245"/>
        <source>Abstract</source>
        <translation>ບໍ່ລົງເລີກ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="187"/>
        <source>&amp;Add</source>
        <translation>&amp;ເພີ້ມ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="190"/>
        <source>Alt+A</source>
        <translation type="unfinished">Alt+A</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="34"/>
        <source>Server Connections</source>
        <translation>ການເຊື່ອມຕໍ່ໜ່ວຍເຄືອຄ່າຍແມ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="108"/>
        <source>&amp;New</source>
        <translation>&amp;ໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="101"/>
        <source>Delete</source>
        <translation type="unfinished">ລືບ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="91"/>
        <source>Edit</source>
        <translation type="unfinished">ແກ້ໄຂ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="81"/>
        <source>C&amp;onnect</source>
        <translation>ເ&amp;ຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="174"/>
        <source>Ready</source>
        <translation type="unfinished">ພ້ອມໃຊ້ງານ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="118"/>
        <source>Coordinate Reference System</source>
        <translation type="unfinished">ລະບົບປະສານງານທີ່ເປັນຕົວອ້າງອີງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="156"/>
        <source>Change ...</source>
        <translation>ປ່ຽນ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="46"/>
        <source>Adds a few example WMS servers</source>
        <translation type="unfinished">ເພີ້ມຕົວຢ່າງເຂົ້າໄປໃສ່ໜ່ວຍເຄື່ອຄ່າຍແມ່ສອງສາມຢ່າງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="52"/>
        <source>Add default servers</source>
        <translation type="unfinished">ເພີ້ມໜ່ວຍເຄື່ອຄ່າຍແມ່ທີ່ເປັນຄ່າຕາຍຕົວມານຳມັນ</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="419"/>
        <source>The database gave an error while executing this SQL:</source>
        <translation>ຖານຂໍ້ມູນໄດ້ກໍ່ໃຫ້ເກີດຂໍ້ຜິດພາດໃນຄະນະທີ່ກຳລັງຈັດການ SQL ນີ້:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="427"/>
        <source>The error was:</source>
        <translation>ຂໍ້ຜິດພາດແມ່ນ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>... (rest of SQL trimmed)</source>
        <comment>



is appended to a truncated SQL statement
</comment>
        <translation type="obsolete">...(ສ່ວນທີ່ເຫຼືອຂອງ SQL ທີ່ຖືກຕັດອອກ)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="424"/>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation type="unfinished">...(ສ່ວນທີ່ເຫຼືອຂອງ SQL ທີ່ຖືກຕັດອອກ)</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="121"/>
        <source>Solid Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="122"/>
        <source>Dash Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="123"/>
        <source>Dot Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="124"/>
        <source>Dash Dot Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="125"/>
        <source>Dash Dot Dot Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="126"/>
        <source>No Pen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="131"/>
        <source>Solid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="132"/>
        <source>Horizontal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="133"/>
        <source>Vertical</source>
        <translation type="unfinished"></translation>
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
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="145"/>
        <source>No Brush</source>
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
        <translation>ເຄື່ອງໝາຍງ່ນຶ່ງດຽວ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Fill Patterns:</source>
        <translation type="obsolete">ໃສ່ຮູບແບບເຂົ້າໄປ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Point</source>
        <translation type="obsolete">ຈຸດ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="95"/>
        <source>Size</source>
        <translation type="unfinished">ຄະໜາດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Symbol</source>
        <translation type="obsolete">ສັນຍາລັກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Outline Width:</source>
        <translation type="obsolete">ຄວາມໜາຂອງເສັ້ນລອບນອກ: </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Fill Color:</source>
        <translation type="obsolete">ເຕີມສີ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Outline color:</source>
        <translation type="obsolete">ສີຂອງເສັ້ນລອບນອກ: </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Outline Style:</source>
        <translation type="obsolete">ຮູບແບບຂອງເສັ້ນລອບນອກ: </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Label:</source>
        <translation type="obsolete">ກາຫມາຍ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>No Fill</source>
        <translation type="obsolete">ບໍ່ມີການເຕີມ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Browse:</source>
        <translation type="obsolete">ການເບີ່ງ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="45"/>
        <source>Label</source>
        <translation type="unfinished">ກາຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="73"/>
        <source>Point Symbol</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="105"/>
        <source>Area scale field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="115"/>
        <source>Rotation field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="182"/>
        <source>Style Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="188"/>
        <source>Outline style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="220"/>
        <source>Outline color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="255"/>
        <source>Outline width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="278"/>
        <source>Fill color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="313"/>
        <source>Fill style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="341"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="147"/>
        <source>to vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="151"/>
        <source>to segment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="89"/>
        <source>to vertex and segment</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="13"/>
        <source>Snapping options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="26"/>
        <source>Layer</source>
        <translation type="unfinished">ລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="31"/>
        <source>Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="36"/>
        <source>Tolerance</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="148"/>
        <source>Are you sure you want to remove the [</source>
        <translation>ທ່ານໝັ້ນໃຈບໍ່ທີ່ຕ້ອງການເອົາອອກ [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="148"/>
        <source>] connection and all associated settings?</source>
        <translation>] ການເຊື່ອມຕໍ່ແລະການຕັ້ງຄ່າທີ່ມີການພົວພັນກັບມັນທັງໝົດ?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="149"/>
        <source>Confirm Delete</source>
        <translation type="unfinished">ຢືນຢັນລືບ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> - Edit Column Names</source>
        <translation type="obsolete">- ແກ້ໄຂຊື່ຖັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="268"/>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>Shapefile ດັ່ງຕໍ່ໄປນີ້ບໍ່ສາມາດໂຫຼດໄດ້:

</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="272"/>
        <source>REASON: File cannot be opened</source>
        <translation type="unfinished">ເຫດຜົນ: ແຟ້ມບໍ່ສາມາດເປີດໄດ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="277"/>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>ເຫດຜົນ:ນຶ່ງຫຼືສອງອັນຂອງແຟ້ມ Shapefile (*.dbf, *.shx) ຂາດຫາຍ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="354"/>
        <source>General Interface Help:</source>
        <translation type="unfinished">ການຊ່ວຍເຫຼືອຂອງຕົວປະສານງານລະຫວ່າງຄອມພີວເຕີແລະຄົນໂດຍທົ່ວໄປ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="356"/>
        <source>PostgreSQL Connections:</source>
        <translation>ການເຊື່ອມຕໍ່ຂອງ PostgreSQL:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="358"/>
        <source>[New ...] - create a new connection</source>
        <translation type="unfinished">[ໃໝ່ ...] -ສ້າງການເຊື່ອມຕໍ່ໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="359"/>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation type="unfinished">[ແກ້ໄຂ ...] -ແກ້ໄຂການເຊື່ອມຕໍ່ປະຈຸບັນທີ່ຖືກເລື້ອກໄວ້ແລ້ວ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="360"/>
        <source>[Remove] - remove the currently selected connection</source>
        <translation type="unfinished">[ເອົາອອກ] -ເອົາການເຊື່ອມຕໍ່ປະຈຸບັນທີ່ຖືກເລື້ອກອອກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="361"/>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation>-ທ່ານຕ້ອງເລື້ອກການເຊື່ອມຕໍ່ທີ່ທຳງານ (ເຊື່ອມຢ່າງຖືກຕ້ອງ) ເພື່ອທີ່ຈະນຳເຂົ້າແຟ້ມ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="362"/>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation>-ຕອນປຽນການເຂົ້າສູ່ລະບົບ ເເບບເເຜນ ໂລກ (Global Schema) ຈະປຽນໄປນໍາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="363"/>
        <source>Shapefile List:</source>
        <translation>ລາຍຊື່ຂອງ Shapefile :</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="365"/>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation type="unfinished">[ເພີ້ມ ...] -ເປີດລາຍການແຟ້ມແລະເລື້ອກເອົາແຟ້ມທີ່ຕ້ອງການເພື່ອນຳເຂົ້າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="366"/>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation type="unfinished">[ເອົາອອກ] -ເອົາແຟ້ມທີ່ຖືກເລື້ອກໄວ້ອອກຈາກລາຍຊື່</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="367"/>
        <source>[Remove All] - remove all the files in the list</source>
        <translation type="unfinished">[ເອົາອອກທັງໝົດ] -ເອົາແຟ້ມອອກທັງໝົດຈາກລາຍຊື່</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="368"/>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] -ID ອ້າງອິງສຳລັບ Shapefile ທີ່ຈະນຳເຂົ້າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="369"/>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation type="unfinished">[ນຳໃຊ້ (SRIDມານຳມັນ)] -ຕັ້ງ SRID-1 </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="370"/>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation type="unfinished">[ຊື່ຖັນທາງເລຂາຄະນິດ] -ຖັນທາງເລຂາຄະນິດຢູ່ໃນຖານຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="371"/>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation>[ນຳໃຊ້ຄ່າຕາຍຕົວ (ຊື່ຂອງຖັນເລຂາຄະນິດ)] -ຕັ້ງຊື່ຖັນໄປໃສ່ &apos;the_geom&apos; </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="372"/>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation>[ເເບບເເຜນ ໂລກ (Glogal Schema)] - ປັບ ເເບບເເຜນ (Schema) ສໍາລັບທຸກ ຯ ເເຟ້ມ ທີ່ຈະນໍາເຂົ້າມາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="374"/>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[ນຳເຂົ້າ] - ນຳເຂົ້າ Shapefile ປະຈຸບັນຢູ່ໃນລາຍຊື່</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="375"/>
        <source>[Quit] - quit the program
</source>
        <translation type="unfinished">[ອອກ] -ອອກໂປຼກຼາມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="376"/>
        <source>[Help] - display this help dialog</source>
        <translation type="unfinished">[ຊ່ວຍເຫຼືອ] -ສະແດງຄຳບັນຍາຍການຊ່ວຍເຫຼືອ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="833"/>
        <source>Import Shapefiles</source>
        <translation>ນຳເຂົ້າ Shapefile</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="833"/>
        <source>You need to specify a Connection first</source>
        <translation type="unfinished">ທ່ານຕ້ອງໄດ້ລະບຸການເຊື່ອມຕໍ່ກ່ອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="428"/>
        <source>Connection failed - Check settings and try again</source>
        <translation type="unfinished">ການເຊື່ອມຕໍ່ລົ້ມເຫລວ-ກວດເບິ່ງການຕັ້ງຄ່າແລ້ວລອງໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="513"/>
        <source>You need to add shapefiles to the list first</source>
        <translation>ທ່ານຕ້ອງການເພີ້ມ Shapefile ໃສ່ລາຍຊື່ກ່ອນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="580"/>
        <source>Importing files</source>
        <translation type="unfinished">ນຳແຟ້ມເຂົ້າມາ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="518"/>
        <source>Cancel</source>
        <translation type="unfinished">ຍົກເລີກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="522"/>
        <source>Progress</source>
        <translation type="unfinished">ຄວາມຄືບໜ້າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="531"/>
        <source>Problem inserting features from file:</source>
        <translation>ມີປັນຫາໃນການສອດຈຸດເດັ່ນຈາກແຟ້ມ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="538"/>
        <source>Invalid table name.</source>
        <translation>ຊື່ຕາຕະລາງໃຊ້ການບໍ່ໄດ້.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="548"/>
        <source>No fields detected.</source>
        <translation>ບໍ່ມີສະໜາມຖືກຄົ້ນພົບ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="573"/>
        <source>The following fields are duplicates:</source>
        <translation type="unfinished">ສະໜາມທີ່ຕາມມານີ້ແມ່ນສຳເນົາ:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="672"/>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>ນຳເຂົ້າ Shapefile, ສາຍພົວພັນຍົງຄົງມີຢູ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="673"/>
        <source>The Shapefile:</source>
        <translation type="unfinished">Shapefile:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>will use [</source>
        <translation type="unfinished">ຈະນຳໃຊ້ [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>] relation for its data,</source>
        <translation>] ພົວພັນກັບຂໍ້ມູນມັນເອງ,</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>which already exists and possibly contains data.</source>
        <translation>ຊຶ່ງມີຢູ່ລຽບລ້ອຍແລ້ວແລະເປັນໄປໄດ້ທີ່ຈະມີຂໍ້ມູນ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>ເພື່ອຫຼີກລ້ຽງການສູນເສຍຂໍ້ມູນ,ຈົ່ງປ່ຽນຊື່ທີ່ມີ\&quot;ການພົວພັນກັນກັບ DB\&quot;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation> ສຳລັບ Shapefile ນີ້ຢູ່ໃນລາຍຊື່ແຟ້ມບັນຍາຍຫຼັກ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>Do you want to overwrite the [</source>
        <translation type="unfinished">ທ່ານຕ້ອງການຂຽນຖັບ [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>] relation?</source>
        <translation>] ພົວພັນບໍ້?</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Use the table below to edit column names. Make sure that none of the columns are named using a PostgreSQL reserved word</source>
        <translation type="obsolete">ນຳໃຊ້ຕາຕະລາງຂ້າງລຸ່ມນີ້ເພື່ອແກ້ໄຂຊື່.ຈົ່ງເຮັດໃຫ້ໝັ້ນໃຈວ່າບໍ່ມີຖັນໂຕໃດໃສ່ຊື່ໂດຍໃຊ້ຄຳເວົ້າທີ່ຖືກຈອງໄວ້ແລ້ວໂດຍ PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>File Name</source>
        <translation type="unfinished">ຊື່ແຟ້ມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>Feature Class</source>
        <translation type="unfinished">ຊັ້ນຂອງຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Features</source>
        <translation type="unfinished">ຈຸດເດັ່ນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>DB Relation Name</source>
        <translation>ຊື່ທີ່ພົວພັນກັບ DB</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Schema</source>
        <translation>ແບບແຜນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>New Connection</source>
        <translation type="obsolete">ການເຊື່ອມຕໍ່ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="172"/>
        <source>Add Shapefiles</source>
        <translation>ເພີ້ມ Shapefiles</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="174"/>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation>Shapefiles(*.shp);;ແຟ້ມ Shapefiles ທັງໝົດ(*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="467"/>
        <source>PostGIS not available</source>
        <translation>PostGIS ບໍ່ມີ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="469"/>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>ຖານຂໍ້ມູນທີ່ໄດ້ເລື້ອກໄວ້ແລ້ວນັ້ນບໍ່ໄດ້ຮັບການຕິດຕັ້ງ PostGIS,ແຕ່ PostGIS ຈຳເປັນສຳລັບເກັບມ້ຽນຂໍ້ມູນໄລຍະ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="564"/>
        <source>Checking to see if </source>
        <translation type="unfinished">ກວດສອບເບິ່ງວ່າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="814"/>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt; ມີຂໍ້ຜິດພາດເກີດຂື້ນໃນຄະນະທີຈັດການ SQL&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="815"/>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation type="unfinished">ຖານຂໍ້ມູນເວົ້າວ່າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="408"/>
        <source>Password for </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="409"/>
        <source>Please enter your password:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="829"/>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation>ການເຊື່ອມຕໍ່ຂອງ PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="188"/>
        <source>Remove</source>
        <translation type="unfinished">ເອົາອອກ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Geometry Column Name</source>
        <translation type="obsolete">ຊື່ຖັນທາງເລຂາຄະນິດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>SRID</source>
        <translation type="obsolete">SRID</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="201"/>
        <source>Remove All</source>
        <translation type="unfinished">ເອົາອອກທັງໝົດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="273"/>
        <source>Global Schema</source>
        <translation>ເເບບເເຜນໂລກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="175"/>
        <source>Add</source>
        <translation type="unfinished">ເພີ້ມ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="172"/>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>ເພີ້ມ Shapefile ເຂົ້າໄປຫາລາຍຊື່ຂອງແຟ້ມທີ່ຈະຖືກນຳເຂົ້າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="185"/>
        <source>Remove the selected shapefile from the import list</source>
        <translation> ນຳເອົາ Shapefile ທີ່ຖືກເລື້ອກໄວ້ແລ້ສນັ້ນົດອອກຈາກລາຍຊື່ນຳເຂົ້າ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="198"/>
        <source>Remove all the shapefiles from the import list</source>
        <translation> ນຳເອົາ Shapefile ທັງໝົດອອກຈາກລາຍຊື່ນຳເຂົ້າ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Use Default SRID</source>
        <translation type="obsolete">ນຳໃຊ້ຄ່າຕາຍຕົວຂອງ SRID</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="224"/>
        <source>Set the SRID to the default value</source>
        <translation>ຕັ້ງຄ່າຂອງ SRID ໃຫ້ເປັນຄ່າຕາຍຕົວມານຳມັນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Use Default Geometry Column Name</source>
        <translation type="obsolete">ນຳໃຊ້ຊື້ຕາຍຕົວຂອງຖັນທີ່ເປັນເລຂາຄະນິດນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="237"/>
        <source>Set the geometry column name to the default value</source>
        <translation>ຕັ້ງຊື້ຂອງຖັນທີ່ເປັນເລຂາຄະນິດໃຫ້ເປັນຄ່າື່ຕາຍຕົວມານຳມັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="121"/>
        <source>New</source>
        <translation type="unfinished">ໃຫມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="118"/>
        <source>Create a new PostGIS connection</source>
        <translation>ສ້າງການເຊື່ອມຕໍ່ຂອງ PostGIS ໃໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="105"/>
        <source>Remove the current PostGIS connection</source>
        <translation>ເອົາການເຊື່ອມຕໍ່ຂອງ PostGIS ປະຈຸບັນອອກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="134"/>
        <source>Connect</source>
        <translation type="unfinished">ເຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="95"/>
        <source>Edit</source>
        <translation type="unfinished">ແກ້ໄຂ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="92"/>
        <source>Edit the current PostGIS connection</source>
        <translation> ແກ້ໄຂການເຊື່ອມຕໍ່ PostGIS ປະຈຸບັນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="19"/>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT - ເຄື່ອງມືນຳເຂົ້າ Shapefile ໄປຫາ PostGIS </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Shapefile to PostGIS Import Tool</source>
        <translation type="obsolete">ເຄື່ອງມືນຳເຂົ້າ າShapefil eໄປຫ າPostGIS</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Shapefile List</source>
        <translation type="obsolete">ລາຍຊື່ຂອງ Shapefile</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="131"/>
        <source>Connect to PostGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="144"/>
        <source>Import options and shapefile list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="227"/>
        <source>Use Default SRID or specify here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="240"/>
        <source>Use Default Geometry Column Name or specify here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="263"/>
        <source>Primary Key Column Name</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="68"/>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>&amp;ນຳເຂົ້າ Shapefile ໄປຫາ PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="70"/>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>ນຳ Shapefile ເຂົ້າໄປໃນຖານຂໍ້ມູນ PostgreSQL  ທີ່ໄດ້ເປີດ PostGIS ໄວ້ແລ້ວ.ແບບແຜນ ແລະ ຊື່ສະໜາມສາມາດຈັດການຕາມຄວາມຕ້ອງການຂອງຕົນເມື່ອນຳເຂົ້າ.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="93"/>
        <source>&amp;Spit</source>
        <translation>&amp;Spit</translation>
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
        <translation type="unfinished"></translation>
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
        <translation>ແບບ1</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Classification Field:</source>
        <translation type="obsolete">ສະໜາມການຈັດໝວດໝູ່:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Delete class</source>
        <translation type="obsolete">ໍລືບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="93"/>
        <source>Classify</source>
        <translation type="unfinished">ຈັດໝວດໝູ່</translation>
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
        <location filename="../src/core/qgsvectorlayer.cpp" line="2470"/>
        <source>Could not commit the added features.</source>
        <translation type="unfinished">ບໍ່ສາມາດຈົດຈຳຈຸດເດັ່ນທີ່ໄດ້ໄດ້ເພີ້ມຕື່ມ.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2561"/>
        <source>No other types of changes will be committed at this time.</source>
        <translation>ບໍ່ມີການປ່ຽນແປງຊະນິດໃດໆຈະໄດ້ຖືກຈົດຈຳໃນເວລານີ້.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2492"/>
        <source>Could not commit the changed attributes.</source>
        <translation type="unfinished">ບໍ່ສາມາດຈົດຈຳຄຸນລັກສະນະທີ່ໄດ້ປ່ຽນແປງໄດ້ເພີ້ມຕື່ມ.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2551"/>
        <source>However, the added features were committed OK.</source>
        <translation type="unfinished">ແນວໃດກໍ່ຕາມ,ຈຸດເດັ່ນດທີ່ໄດ້ຮັບການປ່ຽນແປງຖືກຈົດຈຳ ງ . ຕົກລົງ .</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2518"/>
        <source>Could not commit the changed geometries.</source>
        <translation type="unfinished">ບໍ່ສາມາດຈົດຈຳຮູບເລຂາຄະນິດທີ່ໄດ້ປ່ຽນແປງໄປດໄດມຕື່ມ.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2555"/>
        <source>However, the changed attributes were committed OK.</source>
        <translation>ແນວໃດກໍ່ຕາມ,ຄຸນລັກສະນະດທີ່ໄດ້ຮັບການປ່ຽນແປງຖືກຈົດຈຳ ໃດ້ບໍ່.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2548"/>
        <source>Could not commit the deleted features.</source>
        <translation type="unfinished">ບໍ່ສາມາດຈົດຈຳຈຸດເດັ່ນທີ່ໄດ້ລືບໄປ.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2559"/>
        <source>However, the changed geometries were committed OK.</source>
        <translation>ແນວໃດກໍ່ຕາມ,ເລຂາຄະນິດທີ່ໄດ້ຮັບການປ່ຽນແປງຖືກຈົດຈຳ ໃດ້ບໍ່ .</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="113"/>
        <source>Transparency: </source>
        <translation>ຄວາມໂປ່ງໃສ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="205"/>
        <source>Single Symbol</source>
        <translation>ເຄື່ອງໝາຍງ່ນຶ່ງດຽວ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="208"/>
        <source>Graduated Symbol</source>
        <translation type="unfinished">ເຄື່ອງໝາຍທີ່ໄດ້ຮັບຈັດອັນດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="209"/>
        <source>Continuous Color</source>
        <translation type="unfinished">ສີຕໍ່ເນື່ອງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="210"/>
        <source>Unique Value</source>
        <translation type="unfinished">ຄ່າພ່ຽງນຶ່ງດຽວ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="164"/>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation>ປຸ່ມນີ້ເປີດຕົວສ້າງຄຳຖາມ PostgreSQL ແລະ ເຮັດໃຫ້ທ່ານສາມາດສ້າງຊຸດຍ່ອຍຂອງຈຸດເດັ່ນເພື່ອສະແດງພື້ນແຜນທີ່ຫຼາຍກວ່າສະແດງຈຸດເດັ່ນທັງໝົດຢູ່ໃນລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="167"/>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>ການສອບຖາມໃຊ້ເພື່ອຈຳກັດຈຸດເດັ່ນໃນລະດັບຊັ້ນທີ່ສະແດງໃນທີ່ນີ້.ນີ້ຈະເຂົ້າກັນກັບລະດັບຊັ້ນ PostgreSQL ,ເພື່ອໃສ່ຫຼືດັດແກ້ຄຳຖາມ,ກົດໃສ່ປຸ່ມສ້າງຄຳຖາມ (The Query Builder Button)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="398"/>
        <source>Spatial Index</source>
        <translation type="unfinished">ດັດສະນີທີ່ຊີ້ບອກໄລຍະ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="393"/>
        <source>Creation of spatial index successfull</source>
        <translation type="unfinished">ການສ້າງດັດສະນີບອກໄລຍະທາງປະສົບຜົນສຳເລັດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="398"/>
        <source>Creation of spatial index failed</source>
        <translation type="unfinished">ການສ້າງດັດສະນີບອກໄລຍະທາງລົ້ມເຫລວ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="411"/>
        <source>General:</source>
        <translation type="unfinished">ທົ່ວໄປ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="426"/>
        <source>Storage type of this layer : </source>
        <translation>ຊະນິດຂອງລະດັບຊັ້ນນີ້ທີ່ເປັນບ່ອນຈັດເກັບ :</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="432"/>
        <source>Source for this layer : </source>
        <translation>ແຫຼ່ງສຳລັບລະດັບຊັ້ນີ້:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="449"/>
        <source>Geometry type of the features in this layer : </source>
        <translation type="unfinished">ຊະນິດຂອງຈຸດເດັ່ນທີ່ເປັນເລຂາຄະນິດຢູ່ໃນລະດັບຊັ້ນນີ້:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="457"/>
        <source>The number of features in this layer : </source>
        <translation type="unfinished">ຈຳນວນຈຸດເດັ່ນໃນລະດັບຊັ້ນນີ້:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="462"/>
        <source>Editing capabilities of this layer : </source>
        <translation type="unfinished">ແກ້ໄຂຄວາມສາມາດຂອງລະດັບຊັ້ນ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="469"/>
        <source>Extents:</source>
        <translation type="unfinished">ຂອບເຂດ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="474"/>
        <source>In layer spatial reference system units : </source>
        <translation>ລະບົບອ້າງອິງໄລຍະທີ່ເປັນລະດັບຊັ້ນ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="475"/>
        <source>xMin,yMin </source>
        <translation>x ຕຳ່ສຸດ,y ຕຳ່ສຸດ </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="479"/>
        <source> : xMax,yMax </source>
        <translation> : x ສູງສຸດ,y ສູງສຸດ </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="534"/>
        <source>In project spatial reference system units : </source>
        <translation type="unfinished">ຫົວໜ່ວຍລະບົບອ້າງອິງໄລຍະຂອງໜ້າວຽກ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="508"/>
        <source>Layer Spatial Reference System:</source>
        <translation type="unfinished">ລະບົບອ້າງອິງໄລຍະທີ່ເປັນລະດັບຊັ້ນ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="545"/>
        <source>Attribute field info:</source>
        <translation type="unfinished">ຂໍ້ມູນລັກສະນະຂອງສະໜາມ:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="552"/>
        <source>Field</source>
        <translation type="unfinished">ສະໜາມ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="555"/>
        <source>Type</source>
        <translation type="unfinished">ຊະນິດ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="558"/>
        <source>Length</source>
        <translation type="unfinished">ຄວາມຍາວ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="561"/>
        <source>Precision</source>
        <translation type="unfinished">ຄວາມທ່ຽງຕົງ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="419"/>
        <source>Layer comment: </source>
        <translation type="unfinished">ຄຳເຫັນລະດັບຊັ້ນ: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="564"/>
        <source>Comment</source>
        <translation type="unfinished">ຄຳເຫັນ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="766"/>
        <source>Default Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="748"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="772"/>
        <source>QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="773"/>
        <source>Unknown style format: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="19"/>
        <source>Layer Properties</source>
        <translation>ຄຸນນະສົມບັດ ລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Legend type:</source>
        <translation type="obsolete">ຊະນິດຂອງຄຳອະທິບາຍແຜນທີ່: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="38"/>
        <source>Symbology</source>
        <translation>ເຄື່ອງຫມາຍ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Transparency:</source>
        <translation type="obsolete">ຄວາມໂປ່ງໃສ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="145"/>
        <source>General</source>
        <translation type="unfinished">ທົ່ວໄປ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="217"/>
        <source>Use scale dependent rendering</source>
        <translation>ໃຊ້ມາດຕາສ່ວນ ເພື່ອທໍາການປະສົມ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Maximum 1:</source>
        <translation type="obsolete">ສູງ່ສຸດ 1:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Minimum 1:</source>
        <translation type="obsolete">ຕຳ່ສຸດ 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="258"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>ມາດຕາສ່ວນຕຳ່ສຸດຊຶ່ງລະດັບຊັ້ນນີ້ຈະສະແດງ.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="271"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>ມາດຕາສ່ວນຕຳ່ສຸດຊຶ່ງລະດັບຊັ້ນນີ້ຈະສະແດງ.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="157"/>
        <source>Display name</source>
        <translation type="unfinished">ສະແດງຊື່</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="186"/>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation>ນຳໃຊ້ຕົວຄວບຄຸມນີ້ເພື່ອຕັ້ງຄ່າສະໜາມທີ່ວາງໃສ່ເທິງຂອງກອງຂໍ້ຄວາມທີ່ຊີ້ບອກຊິ່ງຕ່າງໆ.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Spatial Reference System</source>
        <translation type="obsolete">ລະບົບອ້າງອິງໄລຍະ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Change</source>
        <translation type="obsolete">ປ່ຽນແປງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="170"/>
        <source>Display field for the Identify Results dialog box</source>
        <translation type="unfinished">ສະແດງສະໜາມສຳລັບກອງຂໍ້ຄວາມທີລະບຸຜົນການຊີ້ບອກຊິ່ງຕ່າງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="173"/>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation type="unfinished">ສິ່ງນີ້ຕັ້ງເພື່ອສະແດງສະໜາມສຳລັບກອງຂໍ້ຄວາມທີລະບຸຜົນການຊີ້ບອກຊິ່ງຕ່າງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="176"/>
        <source>Display field</source>
        <translation type="unfinished">ສະແດງສະໜາມ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="287"/>
        <source>Subset</source>
        <translation type="unfinished">ຊຸດຍ່ອຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="334"/>
        <source>Query Builder</source>
        <translation type="unfinished">ຖາມຫາຜູ້ສ້າງ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Spatial Index</source>
        <translation type="obsolete">ດັດສະນີທີ່ຊີ້ບອກໄລຍະ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="200"/>
        <source>Create Spatial Index</source>
        <translation type="unfinished">ສ້າງດັດສະນີທີ່ຊີ້ບອກັບໄລຍະ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Create</source>
        <translation type="obsolete">ສ້າງ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="345"/>
        <source>Metadata</source>
        <translation>Metadata</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="374"/>
        <source>Labels</source>
        <translation>ຫລາຍກາຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="386"/>
        <source>Display labels</source>
        <translation>ສະແດງຫລາຍກາຫມາຍ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="410"/>
        <source>Actions</source>
        <translation type="unfinished">ການປະຕິບັດ</translation>
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
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="453"/>
        <source>Restore Default Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="460"/>
        <source>Save As Default</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="467"/>
        <source>Load Style ...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="474"/>
        <source>Save Style ...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorSymbologyWidgetBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Form2</source>
        <translation type="obsolete">ຮູບແບບ2</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Label</source>
        <translation type="obsolete">ກາຫມາຍ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Min</source>
        <translation type="obsolete">ຕຳ່ສຸດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Max</source>
        <translation type="obsolete">ສູງສຸດ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Symbol Classes:</source>
        <translation type="obsolete">ຊັ້ນເຄື່ອງໝາຍ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Count:</source>
        <translation type="obsolete">ນັບ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Mode:</source>
        <translation type="obsolete">ແບບ:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Field:</source>
        <translation type="obsolete">ສະໜາມ:</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="59"/>
        <source>&amp;Add WFS layer</source>
        <translation>ເ&amp;ພີ້ມລະດັບຊັ້ນຂອງ WFS</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1391"/>
        <source>unknown</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1397"/>
        <source>received %1 bytes from %2</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="260"/>
        <source>Are you sure you want to remove the </source>
        <translation type="unfinished">ທ່ານໝັ້ນໃຈບໍ່ວ່າຈະເອົາອອກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="260"/>
        <source> connection and all associated settings?</source>
        <translation type="unfinished">ການເຊື່ອມຕໍ່ແລະການຕັ້ງຄ່າທີ່ພົວພັນກັບມັນທັງໝົດ?</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="261"/>
        <source>Confirm Delete</source>
        <translation type="unfinished">ຢືນຢັນລືບ</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="29"/>
        <source>Title</source>
        <translation type="unfinished">ຫົວຂໍ້</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="34"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="39"/>
        <source>Abstract</source>
        <translation>ບໍ່ລົງເລີກ</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="47"/>
        <source>Coordinate Reference System</source>
        <translation type="unfinished">ລະບົບປະສານງານທີ່ເປັນຕົວອ້າງອີງ</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="85"/>
        <source>Change ...</source>
        <translation>ປ່ຽນ ...</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="95"/>
        <source>Server Connections</source>
        <translation>ການເຊື່ອມຕໍ່ໜ່ວຍເຄືອຄ່າຍແມ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="107"/>
        <source>&amp;New</source>
        <translation>ໃ&amp;ໝ່</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="117"/>
        <source>Delete</source>
        <translation type="unfinished">ລືບ</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="127"/>
        <source>Edit</source>
        <translation type="unfinished">ແກ້ໄຂ</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="153"/>
        <source>C&amp;onnect</source>
        <translation>ເ&amp;ຊື່ອມຕໍ່</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Note: this plugin not considered stable yet. Use it on your own risk</source>
        <translation type="obsolete">ຂໍ້ສັງເກດ:ໂປຼກຼາມເສີມນີ້ບໍ່ຖືວ່າຍັງມີສະເຖຍລະພາຍເທື່ອ. ທ່ານສາມາດນຳໃຊ້ມັນໂດຍການສ່ຽງຂອງທ່ານເອງ</translation>
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
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="706"/>
        <source>Tried URL: </source>
        <translation>ລອງໃຊ້ທີ່ຢູ່ຂອງອີນເຕີເນດ URL:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="686"/>
        <source>HTTP Exception</source>
        <translation>ຂໍ້ຍົກເວັ້ນຂອງ HTTP</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="649"/>
        <source>WMS Service Exception</source>
        <translation>ຂໍ້ຍົກເວັ້ນຂອງການໃຫ້ບໍລິການ WMS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1500"/>
        <source>DOM Exception</source>
        <translation>ຂໍ້ຍົກເວັ້ນຂອງ DOM</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="760"/>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>ບໍ່ສາມາດຮັບເອົາຄວາມສາມາດຂອງ WMS: %1 ຢູ່ເສັ້ນ %2 ຖັນ %3</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="791"/>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>ສິ່ງນີ້ອາດຈະເກີດຈາກ URL ຂອງໜ່ວຍເຄືອຄ່າຍແມ່ WMS.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="787"/>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>ບໍ່ສາມາດໄດ້ຮັບຄວາມອາດສາມາດຂອງ WMS ຢູ່ໃນການຈັດຮູບແບບຂອງຄອມພີວເຕີທີ່ຄາດໄວ້ (DTD): ບໍ່ພົບ %1 ຫຼື %2</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1502"/>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>ບໍ່ສາມາດຮັບເອົາການບໍລິການຂອງ WMS ຍົກເວັ້ນທີ່ %1:%2 ຢູ່ເສັ້ນ %3 ຖັນ %4</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1552"/>
        <source>Request contains a Format not offered by the server.</source>
        <translation>ຄຳຂໍຂໍ້ມູນຈຸດເດັ່ນທີ່ບັນຈຸຄ່າຮູບເເບບທີ່ບໍ່ໃດ້ຈາກເເມ່ບໍລິການ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1556"/>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>ຄຳຂໍຂໍ້ມູນຈຸດເດັ່ນທີ່ບັນຈຸຄ່າ CRS ທີ່ບໍ່ໃດ້ຈາກເເມ່ບໍລິການສະເພາະ 1 ຫລື ຫລາຍ ລະດັບຊັ້ນທີ່ຖຶກຂໍ.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1560"/>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>ຄຳຂໍຂໍ້ມູນຈຸດເດັ່ນທີ່ບັນຈຸຄ່າ SRS ທີ່ບໍ່ໃດ້ຈາກເເມ່ບໍລິການສະເພາະ 1 ຫລື ຫລາຍ ລະດັບຊັ້ນທີ່ຖຶກຂໍ.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1565"/>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation>GetMap ຄໍາຮ້ອງຂໍເເມ່ນສໍາລັບທີ່ບັນລະດັບຊັ້ນ ທີ່ບໍ່ໃດ້ຈາກເເມ່ບໍລິການ, ຫລື GetFeatureInfo ຄຳຂໍທີ່ບັນຈຸ ເເມ່ນສໍາລັບລະດັບຊັ້ນທີ່ບໍ່ສະເເດງໃນເເຜນທີ່.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1569"/>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>ຄຳຂໍເເມ່ນສໍາລັບລະດັບຊັ້ນທີ່ບໍ່ໃດ້ຈາກເເມ່ບໍລິການ.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1573"/>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>GetFeatureInfo ຄຳຂໍເເມ່ນສໍາລັບກ່ຽວຂ້ອງເຖີງລະດັບຊັ້ນທີ່ບໍ່ໃດ້ບອກວ່າຮ້ອງຂໍໃດ້.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1577"/>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>ໄດ້ຮັບຄຳຂໍຂໍ້ມູນຈຸດເດັ່ນທີ່ບັນຈຸຄ່າ (GetFeatureInfo) X ຫຼື Y ທີ່ໃຊ້ການບໍ່ໄດ້.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1582"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation>ຄາ່ຂອງ (ທາງເລື້ອກ) UpdateSequence ຂອບເຂດ ໃນ GetCapabilities ຄຳຂໍ ເເມ່ນເທົ້າກັບ ຄ່າບໍລິການ metadata ເລກລຽງປັບປຸງ.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1587"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation>ຄາ່ຂອງ (ທາງເລື້ອກ) UpdateSequence ຂອບເຂດ ໃນ GetCapabilities ຄຳຂໍ ເເມ່ນສູງກວາ ຄ່າບໍລິການ metadata ເລກລຽງປັບປຸງ.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1592"/>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation>ຄຳຂໍບໍ່ມີຄ່າມິຕິຕົວຢາງ, ເເລະ ຕົວເເມ່ບໍລິການກໍ່ບໍ່ໄດ້ລະບຸຄ່າເດີມສະເພາະມິຕິນັ້ນ.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1596"/>
        <source>Request contains an invalid sample dimension value.</source>
        <translation>ຄຳຂໍທີ່ບັນຈຸຄ່າມິຕິຕົວຢາງໃຊ້ບໍ່ໄດ້.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1600"/>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>ຄຳຂໍເເມ່ນສະເພາະການເຮັດວຽກເລື້ອກທີ່ຍັງບໍ່ຮັບຮອງໃດ້ຈາກເເມ່ບໍລິການ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1604"/>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(ບໍ່ຮູ້ເລກລະຫັດຜີດພາດມາຈາກ-1.3 WMS ຕົວບໍລິການ)</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1607"/>
        <source>The WMS vendor also reported: </source>
        <translation>ຜູ້ຂາຍ WMS ກໍ່ລາຍງານຄືກັນ:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1610"/>
        <source>This is probably due to a bug in the QGIS program.  Please report this error.</source>
        <translation>ປັນຫານີອາດຈະເເມ່ນມາຈາກໂປຣກຣາມ QGIS້. ກະລູນາລາຍງານຄວາມຜິດພາດ.າ.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1794"/>
        <source>Server Properties:</source>
        <translation>ຄຸນລັກສະນະໜ່ວຍເຄືອຄ່າຍແມ່:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1925"/>
        <source>Property</source>
        <translation>ຄຸນລັກສະນະ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1928"/>
        <source>Value</source>
        <translation type="unfinished">ຄ່າ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1811"/>
        <source>WMS Version</source>
        <translation>ລູ້ນ WMS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2057"/>
        <source>Title</source>
        <translation>ຫົວເລື່ອງ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2065"/>
        <source>Abstract</source>
        <translation>ບໍ່ລົງເລີກ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1835"/>
        <source>Keywords</source>
        <translation>ຄໍາສັບສໍາຄັນ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1843"/>
        <source>Online Resource</source>
        <translation>ແຫຼ່ງຂໍ້ມູນທາງອອນລາຍ (Online)</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1851"/>
        <source>Contact Person</source>
        <translation type="unfinished">ການຕິດຕໍ່ຄົນ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1863"/>
        <source>Fees</source>
        <translation type="unfinished">ຄ່າທຳນຽມຕ່າງໆ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1871"/>
        <source>Access Constraints</source>
        <translation>ເຂົ້າຫາຂໍ້ຈຳກັດ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1879"/>
        <source>Image Formats</source>
        <translation>ບການຈັດຮູບໃນລະບົບຄອບພີວເຕີ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1887"/>
        <source>Identify Formats</source>
        <translation type="unfinished">ລະບຸຮູບແບບການຈັດລະບົບຄອມພີວເຕີ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1895"/>
        <source>Layer Count</source>
        <translation type="unfinished">ການນັບລະດັບຊັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1915"/>
        <source>Layer Properties: </source>
        <translation>ຄຸນລັກສະນະລະດັບຊັ້ນ:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1933"/>
        <source>Selected</source>
        <translation type="unfinished">ຖືກເລື້ອກ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1990"/>
        <source>Yes</source>
        <translation>ຕົກລົງ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1990"/>
        <source>No</source>
        <translation type="unfinished">ບໍ່</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1942"/>
        <source>Visibility</source>
        <translation type="unfinished">ການເບີ່ງເຫັນ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1948"/>
        <source>Visible</source>
        <translation type="unfinished">ສາມາດເຫັນໄດ້</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1949"/>
        <source>Hidden</source>
        <translation type="unfinished">ບໍ່ສາມາດເຫັນໄດ້</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1950"/>
        <source>n/a</source>
        <translation>ບໍ່ກຽ່ວຂ້ອງ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1971"/>
        <source>Can Identify</source>
        <translation type="unfinished">ສາມາດລະບຸຮູບປະພັນ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1979"/>
        <source>Can be Transparent</source>
        <translation>ອາດເປັນໂປ່ງໃສ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1987"/>
        <source>Can Zoom In</source>
        <translation type="unfinished">ສາມາດດືງຮູບເຂົ້າມາເບິ່ງໃກ້ໆໄດ້</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1995"/>
        <source>Cascade Count</source>
        <translation>ນັບເປັນຂັ້ນ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2003"/>
        <source>Fixed Width</source>
        <translation type="unfinished">ຄວາມກ້ວາງຄົງທີ່</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2011"/>
        <source>Fixed Height</source>
        <translation type="unfinished">ຄວາມສູງຄົງທີ່</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2019"/>
        <source>WGS 84 Bounding Box</source>
        <translation>WGS 84 ກອງ ບາວດິງ  (Bounding)</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2029"/>
        <source>Available in CRS</source>
        <translation>ມີຢູ່ໃນ CRS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2040"/>
        <source>Available in style</source>
        <translation type="unfinished">ມີຢູ່ໃນຮູບແບບ</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2049"/>
        <source>Name</source>
        <translation type="unfinished">ຊື່</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2150"/>
        <source>Layer cannot be queried.</source>
        <translation type="unfinished">ລະດັບຊ້ັນບໍ່ສາມາດຕິດຕໍ່ໄດ້.</translation>
    </message>
</context>
<context>
    <name>QuickPrintGui</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="129"/>
        <source>Portable Document Format (*.pdf)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="154"/>
        <source>quickprint</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="155"/>
        <source>Unknown format: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="13"/>
        <source>QGIS Quick Print Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="38"/>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="48"/>
        <source>Output</source>
        <translation type="unfinished">ຜົນຂໍ້ມູນ</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="60"/>
        <source>Use last filename but incremented.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="67"/>
        <source>last used filename but incremented will be shown here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="77"/>
        <source>Prompt for file name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="93"/>
        <source>Page Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="103"/>
        <source>Copyright</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="116"/>
        <source>Map Name e.g. Water Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="129"/>
        <source>Map Title e.g. ACME inc.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="158"/>
        <source>Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="75"/>
        <source>Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="77"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="unfinished">ປ່ຽນແທນອັນນີ້ດ້ວຍລາຍການສັ້ນໆຂອງສິ່ງທີ່ໂປຼກຼາມເສີມ (Plugin) ໄດ້ເຮັດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="82"/>
        <source>&amp;Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>RepositoryDetailsDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="13"/>
        <source>Repository details</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="19"/>
        <source>Name:</source>
        <translation type="unfinished">ຊື່:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="29"/>
        <source>URL:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="36"/>
        <source>http://</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>[pluginname]Gui</name>
    <message>
        <location filename="" line="135533324"/>
        <source>QGIS Plugin Template</source>
        <translation type="obsolete">ຕົວຢ່າງໂປຼກຼາມເສີມ QGIS </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Plugin Template</source>
        <translation type="obsolete">ຕົວຢາງໂປຣກຣາມເສີມ</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-weight:600;&quot;&gt;2007&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>[pluginname]GuiBase</name>
    <message>
        <location filename="../src/plugins/plugin_template/pluginguibase.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/pluginguibase.ui" line="47"/>
        <source>Plugin Template</source>
        <translation type="unfinished">ຕົວຢາງໂປຣກຣາມເສີມ</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="75"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation>ປ່ຽນແທນອັນນີ້ດ້ວຍລາຍການສັ້ນໆຂອງສິ່ງທີ່ໂປຼກຼາມເສີມ (Plugin) ໄດ້ເຮັດ</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="73"/>
        <source>[menuitemname]</source>
        <translation type="unfinished">[ຊື່ຂອງເມນູ]</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="80"/>
        <source>&amp;[menuname]</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
