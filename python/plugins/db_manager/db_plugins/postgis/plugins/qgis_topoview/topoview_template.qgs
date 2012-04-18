<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis projectname="" version="1.8.0-Trunk">
    <title></title>
    <mapcanvas>
        <units>degrees</units>
        <extent>
            <xmin>32.062776</xmin>
            <ymin>-4.396154</ymin>
            <xmax>49.384339</xmax>
            <ymax>6.416827</ymax>
        </extent>
        <projections>0</projections>
        <destinationsrs>
            <spatialrefsys>
                <proj4>+proj=longlat +ellps=WGS84 +towgs84=0,0,0,0,0,0,0 +no_defs</proj4>
                <srsid>3452</srsid>
                <srid>4326</srid>
                <authid>EPSG:4326</authid>
                <description>WGS 84</description>
                <projectionacronym>longlat</projectionacronym>
                <ellipsoidacronym>WGS84</ellipsoidacronym>
                <geographicflag>true</geographicflag>
            </spatialrefsys>
        </destinationsrs>
    </mapcanvas>
    <legend>
        <legendlayer open="false" checked="Qt::Checked" name="face seed" showFeatureCount="0">
            <filegroup open="false" hidden="false">
                <legendlayerfile isInOverview="0" layerid="Query1Layer20110526144457416" visible="1"/>
            </filegroup>
        </legendlayer>
        <legendlayer open="false" checked="Qt::Unchecked" name="next_right_face" showFeatureCount="0">
            <filegroup open="false" hidden="false">
                <legendlayerfile isInOverview="0" layerid="edge_data20110504112636129" visible="0"/>
            </filegroup>
        </legendlayer>
        <legendlayer open="false" checked="Qt::Unchecked" name="next_left_face" showFeatureCount="0">
            <filegroup open="false" hidden="false">
                <legendlayerfile isInOverview="0" layerid="edge_data20110504112431391" visible="0"/>
            </filegroup>
        </legendlayer>
        <legendlayer open="false" checked="Qt::Checked" name="face_right" showFeatureCount="0">
            <filegroup open="false" hidden="false">
                <legendlayerfile isInOverview="0" layerid="edge_data20110504094539733" visible="1"/>
            </filegroup>
        </legendlayer>
        <legendlayer open="false" checked="Qt::Checked" name="face_left" showFeatureCount="0">
            <filegroup open="false" hidden="false">
                <legendlayerfile isInOverview="0" layerid="edge_data20110504094415605" visible="1"/>
            </filegroup>
        </legendlayer>
        <legendlayer open="false" checked="Qt::Checked" name="edge" showFeatureCount="0">
            <filegroup open="false" hidden="false">
                <legendlayerfile isInOverview="0" layerid="edge_data20110504083012449" visible="1"/>
            </filegroup>
        </legendlayer>
        <legendlayer open="false" checked="Qt::Checked" name="node" showFeatureCount="0">
            <filegroup open="false" hidden="false">
                <legendlayerfile isInOverview="0" layerid="node20110504083015129" visible="1"/>
            </filegroup>
        </legendlayer>
    </legend>
    <mapcanvas>
        <units>degrees</units>
        <extent>
            <xmin>0.000000</xmin>
            <ymin>0.000000</ymin>
            <xmax>0.000000</xmax>
            <ymax>0.000000</ymax>
        </extent>
        <projections>0</projections>
        <destinationsrs>
            <spatialrefsys>
                <proj4>+proj=longlat +ellps=WGS84 +towgs84=0,0,0,0,0,0,0 +no_defs</proj4>
                <srsid>3452</srsid>
                <srid>4326</srid>
                <authid>EPSG:4326</authid>
                <description>WGS 84</description>
                <projectionacronym>longlat</projectionacronym>
                <ellipsoidacronym>WGS84</ellipsoidacronym>
                <geographicflag>true</geographicflag>
            </spatialrefsys>
        </destinationsrs>
    </mapcanvas>
    <projectlayers layercount="7">
        <maplayer minimumScale="0" maximumScale="1e+08" minLabelScale="0" maxLabelScale="1e+08" geometry="Point" type="vector" hasScaleBasedVisibilityFlag="0" scaleBasedLabelVisibilityFlag="0">
            <id>Query1Layer20110526144457416</id>
            <datasource>dbname='@@DBNAME@@' key='face_id' table="(SELECT face_id, ST_PointOnSurface(topology.ST_GetFaceGeometry('@@TOPONAME@@', face_id)) from @@TOPONAME@@.face where face_id > 0
)" (st_pointonsurface) sql=</datasource>
            <layername>face seed</layername>
            <srs>
                <spatialrefsys>
                    <proj4>+proj=longlat +ellps=WGS84 +towgs84=0,0,0,0,0,0,0 +no_defs</proj4>
                    <srsid>3452</srsid>
                    <srid>4326</srid>
                    <authid>EPSG:4326</authid>
                    <description>WGS 84</description>
                    <projectionacronym>longlat</projectionacronym>
                    <ellipsoidacronym>WGS84</ellipsoidacronym>
                    <geographicflag>true</geographicflag>
                </spatialrefsys>
            </srs>
            <transparencyLevelInt>255</transparencyLevelInt>
            <provider encoding="UTF-8">postgres</provider>
            <vectorjoins/>
            <singlesymbol>
                <symbol>
                    <lowervalue></lowervalue>
                    <uppervalue></uppervalue>
                    <label></label>
                    <pointsymbol>hard:regular_star</pointsymbol>
                    <pointsize>2.3</pointsize>
                    <pointsizeunits>pixels</pointsizeunits>
                    <rotationclassificationfieldname></rotationclassificationfieldname>
                    <scaleclassificationfieldname></scaleclassificationfieldname>
                    <symbolfieldname></symbolfieldname>
                    <outlinecolor red="0" blue="0" green="0"/>
                    <outlinestyle>SolidLine</outlinestyle>
                    <outlinewidth>0.26</outlinewidth>
                    <fillcolor red="240" blue="23" green="243"/>
                    <fillpattern>SolidPattern</fillpattern>
                    <texturepath></texturepath>
                </symbol>
            </singlesymbol>
            <customproperties>
                <property key="labeling" value="pal"/>
                <property key="labeling/addDirectionSymbol" value="false"/>
                <property key="labeling/bufferColorB" value="255"/>
                <property key="labeling/bufferColorG" value="255"/>
                <property key="labeling/bufferColorR" value="255"/>
                <property key="labeling/bufferSize" value="0"/>
                <property key="labeling/dataDefinedProperty0" value=""/>
                <property key="labeling/dataDefinedProperty1" value=""/>
                <property key="labeling/dataDefinedProperty10" value=""/>
                <property key="labeling/dataDefinedProperty11" value=""/>
                <property key="labeling/dataDefinedProperty12" value=""/>
                <property key="labeling/dataDefinedProperty13" value=""/>
                <property key="labeling/dataDefinedProperty14" value=""/>
                <property key="labeling/dataDefinedProperty2" value=""/>
                <property key="labeling/dataDefinedProperty3" value=""/>
                <property key="labeling/dataDefinedProperty4" value=""/>
                <property key="labeling/dataDefinedProperty5" value=""/>
                <property key="labeling/dataDefinedProperty6" value=""/>
                <property key="labeling/dataDefinedProperty7" value=""/>
                <property key="labeling/dataDefinedProperty8" value=""/>
                <property key="labeling/dataDefinedProperty9" value=""/>
                <property key="labeling/dist" value="0"/>
                <property key="labeling/distInMapUnits" value="true"/>
                <property key="labeling/enabled" value="true"/>
                <property key="labeling/fieldName" value="face_id"/>
                <property key="labeling/fontFamily" value="AlArabiya"/>
                <property key="labeling/fontItalic" value="false"/>
                <property key="labeling/fontSize" value="8"/>
                <property key="labeling/fontSizeInMapUnits" value="false"/>
                <property key="labeling/fontStrikeout" value="false"/>
                <property key="labeling/fontUnderline" value="false"/>
                <property key="labeling/fontWeight" value="50"/>
                <property key="labeling/labelPerPart" value="false"/>
                <property key="labeling/mergeLines" value="false"/>
                <property key="labeling/minFeatureSize" value="0"/>
                <property key="labeling/multiLineLabels" value="false"/>
                <property key="labeling/obstacle" value="true"/>
                <property key="labeling/placement" value="0"/>
                <property key="labeling/placementFlags" value="0"/>
                <property key="labeling/priority" value="5"/>
                <property key="labeling/scaleMax" value="0"/>
                <property key="labeling/scaleMin" value="0"/>
                <property key="labeling/textColorB" value="17"/>
                <property key="labeling/textColorG" value="172"/>
                <property key="labeling/textColorR" value="203"/>
            </customproperties>
            <displayfield>face_id</displayfield>
            <label>0</label>
            <labelattributes>
                <label fieldname="" text="Label"/>
                <family fieldname="" name="Sans"/>
                <size fieldname="" units="pt" value="12"/>
                <bold fieldname="" on="0"/>
                <italic fieldname="" on="0"/>
                <underline fieldname="" on="0"/>
                <strikeout fieldname="" on="0"/>
                <color fieldname="" red="0" blue="0" green="0"/>
                <x fieldname=""/>
                <y fieldname=""/>
                <offset x="0" y="0" units="pt" yfieldname="" xfieldname=""/>
                <angle fieldname="" value="0" auto="0"/>
                <alignment fieldname="" value="center"/>
                <buffercolor fieldname="" red="255" blue="255" green="255"/>
                <buffersize fieldname="" units="pt" value="1"/>
                <bufferenabled fieldname="" on=""/>
                <multilineenabled fieldname="" on=""/>
                <selectedonly on=""/>
            </labelattributes>
            <edittypes>
                <edittype type="0" name="face_id"/>
            </edittypes>
            <editform></editform>
            <editforminit></editforminit>
            <annotationform></annotationform>
            <attributeactions/>
        </maplayer>
        <maplayer minimumScale="0" maximumScale="1e+08" geometry="Line" type="vector" hasScaleBasedVisibilityFlag="0">
            <id>edge_data20110504083012449</id>
            <datasource>dbname='@@DBNAME@@' key='edge_id' table="@@TOPONAME@@"."edge_data" (geom) sql=</datasource>
            <layername>edge</layername>
            <srs>
                <spatialrefsys>
                    <proj4>+proj=longlat +ellps=WGS84 +towgs84=0,0,0,0,0,0,0 +no_defs</proj4>
                    <srsid>3452</srsid>
                    <srid>4326</srid>
                    <authid>EPSG:4326</authid>
                    <description>WGS 84</description>
                    <projectionacronym>longlat</projectionacronym>
                    <ellipsoidacronym>WGS84</ellipsoidacronym>
                    <geographicflag>true</geographicflag>
                </spatialrefsys>
            </srs>
            <transparencyLevelInt>255</transparencyLevelInt>
            <provider encoding="UTF-8">postgres</provider>
            <vectorjoins/>
            <renderer-v2 type="RuleRenderer">
                <rules>
                    <rule scalemaxdenom="0" description="" filter="edge_id % 10 = 0" symbol="0" scalemindenom="0" label=""/>
                    <rule scalemaxdenom="0" description="" filter="edge_id % 10 = 1" symbol="1" scalemindenom="0" label=""/>
                    <rule scalemaxdenom="0" description="" filter="edge_id % 10 = 2" symbol="2" scalemindenom="0" label=""/>
                    <rule scalemaxdenom="0" description="" filter="edge_id % 10 = 3" symbol="3" scalemindenom="0" label=""/>
                    <rule scalemaxdenom="0" description="" filter="edge_id % 10 = 4" symbol="4" scalemindenom="0" label=""/>
                    <rule scalemaxdenom="0" description="" filter="edge_id % 10 = 5" symbol="5" scalemindenom="0" label=""/>
                    <rule scalemaxdenom="0" description="" filter="edge_id % 10 = 6" symbol="6" scalemindenom="0" label=""/>
                    <rule scalemaxdenom="0" description="" filter="edge_id % 10 = 7" symbol="7" scalemindenom="0" label=""/>
                    <rule scalemaxdenom="0" description="" filter="edge_id % 10 = 8" symbol="8" scalemindenom="0" label=""/>
                    <rule scalemaxdenom="0" description="" filter="edge_id % 10 = 9" symbol="9" scalemindenom="0" label=""/>
                </rules>
                <symbols>
                    <symbol outputUnit="MM" alpha="1" type="line" name="0">
                        <layer pass="0" class="LineDecoration" locked="0">
                            <prop k="color" v="0,0,0,255"/>
                            <prop k="width" v="0.6"/>
                        </layer>
                        <layer pass="0" class="SimpleLine" locked="0">
                            <prop k="capstyle" v="square"/>
                            <prop k="color" v="0,0,0,255"/>
                            <prop k="customdash" v="5;2"/>
                            <prop k="joinstyle" v="bevel"/>
                            <prop k="offset" v="0"/>
                            <prop k="penstyle" v="solid"/>
                            <prop k="use_custom_dash" v="0"/>
                            <prop k="width" v="0.5"/>
                        </layer>
                    </symbol>
                    <symbol outputUnit="MM" alpha="1" type="line" name="1">
                        <layer pass="0" class="LineDecoration" locked="0">
                            <prop k="color" v="76,51,152,255"/>
                            <prop k="width" v="0.6"/>
                        </layer>
                        <layer pass="0" class="SimpleLine" locked="0">
                            <prop k="capstyle" v="square"/>
                            <prop k="color" v="76,51,152,255"/>
                            <prop k="customdash" v="5;2"/>
                            <prop k="joinstyle" v="bevel"/>
                            <prop k="offset" v="0"/>
                            <prop k="penstyle" v="solid"/>
                            <prop k="use_custom_dash" v="0"/>
                            <prop k="width" v="0.5"/>
                        </layer>
                    </symbol>
                    <symbol outputUnit="MM" alpha="1" type="line" name="2">
                        <layer pass="0" class="LineDecoration" locked="0">
                            <prop k="color" v="60,150,68,255"/>
                            <prop k="width" v="0.6"/>
                        </layer>
                        <layer pass="0" class="SimpleLine" locked="0">
                            <prop k="capstyle" v="square"/>
                            <prop k="color" v="60,150,68,255"/>
                            <prop k="customdash" v="5;2"/>
                            <prop k="joinstyle" v="bevel"/>
                            <prop k="offset" v="0"/>
                            <prop k="penstyle" v="solid"/>
                            <prop k="use_custom_dash" v="0"/>
                            <prop k="width" v="0.5"/>
                        </layer>
                    </symbol>
                    <symbol outputUnit="MM" alpha="1" type="line" name="3">
                        <layer pass="0" class="LineDecoration" locked="0">
                            <prop k="color" v="166,47,49,255"/>
                            <prop k="width" v="0.6"/>
                        </layer>
                        <layer pass="0" class="SimpleLine" locked="0">
                            <prop k="capstyle" v="square"/>
                            <prop k="color" v="166,47,49,255"/>
                            <prop k="customdash" v="5;2"/>
                            <prop k="joinstyle" v="bevel"/>
                            <prop k="offset" v="0"/>
                            <prop k="penstyle" v="solid"/>
                            <prop k="use_custom_dash" v="0"/>
                            <prop k="width" v="0.5"/>
                        </layer>
                    </symbol>
                    <symbol outputUnit="MM" alpha="1" type="line" name="4">
                        <layer pass="0" class="LineDecoration" locked="0">
                            <prop k="color" v="176,172,55,255"/>
                            <prop k="width" v="0.6"/>
                        </layer>
                        <layer pass="0" class="SimpleLine" locked="0">
                            <prop k="capstyle" v="square"/>
                            <prop k="color" v="176,172,55,255"/>
                            <prop k="customdash" v="5;2"/>
                            <prop k="joinstyle" v="bevel"/>
                            <prop k="offset" v="0"/>
                            <prop k="penstyle" v="solid"/>
                            <prop k="use_custom_dash" v="0"/>
                            <prop k="width" v="0.5"/>
                        </layer>
                    </symbol>
                    <symbol outputUnit="MM" alpha="1" type="line" name="5">
                        <layer pass="0" class="LineDecoration" locked="0">
                            <prop k="color" v="7,79,167,255"/>
                            <prop k="width" v="0.6"/>
                        </layer>
                        <layer pass="0" class="SimpleLine" locked="0">
                            <prop k="capstyle" v="square"/>
                            <prop k="color" v="7,79,167,255"/>
                            <prop k="customdash" v="5;2"/>
                            <prop k="joinstyle" v="bevel"/>
                            <prop k="offset" v="0"/>
                            <prop k="penstyle" v="solid"/>
                            <prop k="use_custom_dash" v="0"/>
                            <prop k="width" v="0.5"/>
                        </layer>
                    </symbol>
                    <symbol outputUnit="MM" alpha="1" type="line" name="6">
                        <layer pass="0" class="LineDecoration" locked="0">
                            <prop k="color" v="203,213,14,255"/>
                            <prop k="width" v="0.6"/>
                        </layer>
                        <layer pass="0" class="SimpleLine" locked="0">
                            <prop k="capstyle" v="square"/>
                            <prop k="color" v="203,213,14,255"/>
                            <prop k="customdash" v="5;2"/>
                            <prop k="joinstyle" v="bevel"/>
                            <prop k="offset" v="0"/>
                            <prop k="penstyle" v="solid"/>
                            <prop k="use_custom_dash" v="0"/>
                            <prop k="width" v="0.5"/>
                        </layer>
                    </symbol>
                    <symbol outputUnit="MM" alpha="1" type="line" name="7">
                        <layer pass="0" class="LineDecoration" locked="0">
                            <prop k="color" v="198,7,157,255"/>
                            <prop k="width" v="0.6"/>
                        </layer>
                        <layer pass="0" class="SimpleLine" locked="0">
                            <prop k="capstyle" v="square"/>
                            <prop k="color" v="198,7,157,255"/>
                            <prop k="customdash" v="5;2"/>
                            <prop k="joinstyle" v="bevel"/>
                            <prop k="offset" v="0"/>
                            <prop k="penstyle" v="solid"/>
                            <prop k="use_custom_dash" v="0"/>
                            <prop k="width" v="0.5"/>
                        </layer>
                    </symbol>
                    <symbol outputUnit="MM" alpha="1" type="line" name="8">
                        <layer pass="0" class="LineDecoration" locked="0">
                            <prop k="color" v="56,211,21,255"/>
                            <prop k="width" v="0.6"/>
                        </layer>
                        <layer pass="0" class="SimpleLine" locked="0">
                            <prop k="capstyle" v="square"/>
                            <prop k="color" v="56,211,21,255"/>
                            <prop k="customdash" v="5;2"/>
                            <prop k="joinstyle" v="bevel"/>
                            <prop k="offset" v="0"/>
                            <prop k="penstyle" v="solid"/>
                            <prop k="use_custom_dash" v="0"/>
                            <prop k="width" v="0.5"/>
                        </layer>
                    </symbol>
                    <symbol outputUnit="MM" alpha="1" type="line" name="9">
                        <layer pass="0" class="LineDecoration" locked="0">
                            <prop k="color" v="12,204,198,255"/>
                            <prop k="width" v="0.6"/>
                        </layer>
                        <layer pass="0" class="SimpleLine" locked="0">
                            <prop k="capstyle" v="square"/>
                            <prop k="color" v="12,204,198,255"/>
                            <prop k="customdash" v="5;2"/>
                            <prop k="joinstyle" v="bevel"/>
                            <prop k="offset" v="0"/>
                            <prop k="penstyle" v="solid"/>
                            <prop k="use_custom_dash" v="0"/>
                            <prop k="width" v="0.5"/>
                        </layer>
                    </symbol>
                    <symbol outputUnit="MM" alpha="1" type="line" name="default">
                        <layer pass="0" class="SimpleLine" locked="0">
                            <prop k="capstyle" v="square"/>
                            <prop k="color" v="13,161,50,255"/>
                            <prop k="customdash" v="5;2"/>
                            <prop k="joinstyle" v="bevel"/>
                            <prop k="offset" v="0"/>
                            <prop k="penstyle" v="solid"/>
                            <prop k="use_custom_dash" v="0"/>
                            <prop k="width" v="0.26"/>
                        </layer>
                    </symbol>
                </symbols>
            </renderer-v2>
            <customproperties>
                <property key="labeling" value="pal"/>
                <property key="labeling/addDirectionSymbol" value="false"/>
                <property key="labeling/bufferColorB" value="255"/>
                <property key="labeling/bufferColorG" value="255"/>
                <property key="labeling/bufferColorR" value="255"/>
                <property key="labeling/bufferSize" value="1"/>
                <property key="labeling/dataDefinedProperty0" value=""/>
                <property key="labeling/dataDefinedProperty1" value=""/>
                <property key="labeling/dataDefinedProperty10" value=""/>
                <property key="labeling/dataDefinedProperty11" value=""/>
                <property key="labeling/dataDefinedProperty12" value=""/>
                <property key="labeling/dataDefinedProperty13" value=""/>
                <property key="labeling/dataDefinedProperty14" value=""/>
                <property key="labeling/dataDefinedProperty2" value=""/>
                <property key="labeling/dataDefinedProperty3" value=""/>
                <property key="labeling/dataDefinedProperty4" value=""/>
                <property key="labeling/dataDefinedProperty5" value=""/>
                <property key="labeling/dataDefinedProperty6" value=""/>
                <property key="labeling/dataDefinedProperty7" value=""/>
                <property key="labeling/dataDefinedProperty8" value=""/>
                <property key="labeling/dataDefinedProperty9" value=""/>
                <property key="labeling/dist" value="0"/>
                <property key="labeling/distInMapUnits" value="false"/>
                <property key="labeling/enabled" value="true"/>
                <property key="labeling/fieldName" value="edge_id"/>
                <property key="labeling/fontFamily" value="AlArabiya"/>
                <property key="labeling/fontItalic" value="false"/>
                <property key="labeling/fontSize" value="8"/>
                <property key="labeling/fontSizeInMapUnits" value="false"/>
                <property key="labeling/fontStrikeout" value="false"/>
                <property key="labeling/fontUnderline" value="false"/>
                <property key="labeling/fontWeight" value="50"/>
                <property key="labeling/labelPerPart" value="false"/>
                <property key="labeling/mergeLines" value="false"/>
                <property key="labeling/minFeatureSize" value="0"/>
                <property key="labeling/multiLineLabels" value="false"/>
                <property key="labeling/obstacle" value="true"/>
                <property key="labeling/placement" value="2"/>
                <property key="labeling/placementFlags" value="9"/>
                <property key="labeling/priority" value="5"/>
                <property key="labeling/scaleMax" value="0"/>
                <property key="labeling/scaleMin" value="0"/>
                <property key="labeling/textColorB" value="0"/>
                <property key="labeling/textColorG" value="0"/>
                <property key="labeling/textColorR" value="0"/>
            </customproperties>
            <displayfield>edge_id</displayfield>
            <label>0</label>
            <labelattributes>
                <label fieldname="" text="Label"/>
                <family fieldname="" name="Sans"/>
                <size fieldname="" units="pt" value="12"/>
                <bold fieldname="" on="0"/>
                <italic fieldname="" on="0"/>
                <underline fieldname="" on="0"/>
                <strikeout fieldname="" on="0"/>
                <color fieldname="" red="0" blue="0" green="0"/>
                <x fieldname=""/>
                <y fieldname=""/>
                <offset x="0" y="0" units="pt" yfieldname="" xfieldname=""/>
                <angle fieldname="" value="0" auto="0"/>
                <alignment fieldname="" value="center"/>
                <buffercolor fieldname="" red="255" blue="255" green="255"/>
                <buffersize fieldname="" units="pt" value="1"/>
                <bufferenabled fieldname="" on=""/>
                <multilineenabled fieldname="" on=""/>
                <selectedonly on=""/>
            </labelattributes>
            <edittypes>
                <edittype type="0" name="abs_next_left_edge"/>
                <edittype type="0" name="abs_next_right_edge"/>
                <edittype type="0" name="edge_id"/>
                <edittype type="0" name="end_node"/>
                <edittype type="0" name="left_face"/>
                <edittype type="0" name="next_left_edge"/>
                <edittype type="0" name="next_right_edge"/>
                <edittype type="0" name="right_face"/>
                <edittype type="0" name="start_node"/>
            </edittypes>
            <editform></editform>
            <editforminit></editforminit>
            <annotationform></annotationform>
            <attributeactions/>
        </maplayer>
        <maplayer minimumScale="0" maximumScale="1e+08" minLabelScale="0" maxLabelScale="1e+08" geometry="Line" type="vector" hasScaleBasedVisibilityFlag="0" scaleBasedLabelVisibilityFlag="0">
            <id>edge_data20110504094415605</id>
            <datasource>dbname='@@DBNAME@@' key='edge_id' table="@@TOPONAME@@"."edge_data" (geom) sql=</datasource>
            <layername>face_left</layername>
            <srs>
                <spatialrefsys>
                    <proj4>+proj=longlat +ellps=WGS84 +towgs84=0,0,0,0,0,0,0 +no_defs</proj4>
                    <srsid>3452</srsid>
                    <srid>4326</srid>
                    <authid>EPSG:4326</authid>
                    <description>WGS 84</description>
                    <projectionacronym>longlat</projectionacronym>
                    <ellipsoidacronym>WGS84</ellipsoidacronym>
                    <geographicflag>true</geographicflag>
                </spatialrefsys>
            </srs>
            <transparencyLevelInt>0</transparencyLevelInt>
            <provider encoding="UTF-8">postgres</provider>
            <vectorjoins/>
            <singlesymbol>
                <symbol>
                    <lowervalue></lowervalue>
                    <uppervalue></uppervalue>
                    <label></label>
                    <pointsymbol>hard:circle</pointsymbol>
                    <pointsize>2</pointsize>
                    <pointsizeunits>pixels</pointsizeunits>
                    <rotationclassificationfieldname></rotationclassificationfieldname>
                    <scaleclassificationfieldname></scaleclassificationfieldname>
                    <symbolfieldname></symbolfieldname>
                    <outlinecolor red="54" blue="10" green="31"/>
                    <outlinestyle>SolidLine</outlinestyle>
                    <outlinewidth>0.26</outlinewidth>
                    <fillcolor red="0" blue="0" green="0"/>
                    <fillpattern>NoBrush</fillpattern>
                    <texturepath></texturepath>
                </symbol>
            </singlesymbol>
            <customproperties>
                <property key="labeling" value="pal"/>
                <property key="labeling/addDirectionSymbol" value="false"/>
                <property key="labeling/bufferColorB" value="255"/>
                <property key="labeling/bufferColorG" value="255"/>
                <property key="labeling/bufferColorR" value="255"/>
                <property key="labeling/bufferSize" value="1"/>
                <property key="labeling/dataDefinedProperty0" value=""/>
                <property key="labeling/dataDefinedProperty1" value=""/>
                <property key="labeling/dataDefinedProperty10" value=""/>
                <property key="labeling/dataDefinedProperty11" value=""/>
                <property key="labeling/dataDefinedProperty12" value=""/>
                <property key="labeling/dataDefinedProperty13" value=""/>
                <property key="labeling/dataDefinedProperty14" value=""/>
                <property key="labeling/dataDefinedProperty2" value=""/>
                <property key="labeling/dataDefinedProperty3" value=""/>
                <property key="labeling/dataDefinedProperty4" value=""/>
                <property key="labeling/dataDefinedProperty5" value=""/>
                <property key="labeling/dataDefinedProperty6" value=""/>
                <property key="labeling/dataDefinedProperty7" value=""/>
                <property key="labeling/dataDefinedProperty8" value=""/>
                <property key="labeling/dataDefinedProperty9" value=""/>
                <property key="labeling/dist" value="3"/>
                <property key="labeling/distInMapUnits" value="false"/>
                <property key="labeling/enabled" value="true"/>
                <property key="labeling/fieldName" value="left_face"/>
                <property key="labeling/fontFamily" value="AlArabiya"/>
                <property key="labeling/fontItalic" value="false"/>
                <property key="labeling/fontSize" value="7"/>
                <property key="labeling/fontSizeInMapUnits" value="false"/>
                <property key="labeling/fontStrikeout" value="false"/>
                <property key="labeling/fontUnderline" value="false"/>
                <property key="labeling/fontWeight" value="50"/>
                <property key="labeling/labelPerPart" value="false"/>
                <property key="labeling/mergeLines" value="false"/>
                <property key="labeling/minFeatureSize" value="0"/>
                <property key="labeling/multiLineLabels" value="false"/>
                <property key="labeling/obstacle" value="true"/>
                <property key="labeling/placement" value="2"/>
                <property key="labeling/placementFlags" value="2"/>
                <property key="labeling/priority" value="5"/>
                <property key="labeling/scaleMax" value="0"/>
                <property key="labeling/scaleMin" value="0"/>
                <property key="labeling/textColorB" value="0"/>
                <property key="labeling/textColorG" value="170"/>
                <property key="labeling/textColorR" value="0"/>
            </customproperties>
            <displayfield>edge_id</displayfield>
            <label>0</label>
            <labelattributes>
                <label fieldname="" text="Label"/>
                <family fieldname="" name="Sans"/>
                <size fieldname="" units="pt" value="12"/>
                <bold fieldname="" on="0"/>
                <italic fieldname="" on="0"/>
                <underline fieldname="" on="0"/>
                <strikeout fieldname="" on="0"/>
                <color fieldname="" red="0" blue="0" green="0"/>
                <x fieldname=""/>
                <y fieldname=""/>
                <offset x="0" y="0" units="pt" yfieldname="" xfieldname=""/>
                <angle fieldname="" value="0" auto="0"/>
                <alignment fieldname="" value="center"/>
                <buffercolor fieldname="" red="255" blue="255" green="255"/>
                <buffersize fieldname="" units="pt" value="1"/>
                <bufferenabled fieldname="" on=""/>
                <multilineenabled fieldname="" on=""/>
                <selectedonly on=""/>
            </labelattributes>
            <edittypes>
                <edittype type="0" name="abs_next_left_edge"/>
                <edittype type="0" name="abs_next_right_edge"/>
                <edittype type="0" name="edge_id"/>
                <edittype type="0" name="end_node"/>
                <edittype type="0" name="left_face"/>
                <edittype type="0" name="next_left_edge"/>
                <edittype type="0" name="next_right_edge"/>
                <edittype type="0" name="right_face"/>
                <edittype type="0" name="start_node"/>
            </edittypes>
            <editform></editform>
            <editforminit></editforminit>
            <annotationform></annotationform>
            <attributeactions/>
        </maplayer>
        <maplayer minimumScale="0" maximumScale="1e+08" minLabelScale="0" maxLabelScale="1e+08" geometry="Line" type="vector" hasScaleBasedVisibilityFlag="0" scaleBasedLabelVisibilityFlag="0">
            <id>edge_data20110504094539733</id>
            <datasource>dbname='@@DBNAME@@' key='edge_id' table="@@TOPONAME@@"."edge_data" (geom) sql=</datasource>
            <layername>face_right</layername>
            <srs>
                <spatialrefsys>
                    <proj4>+proj=longlat +ellps=WGS84 +towgs84=0,0,0,0,0,0,0 +no_defs</proj4>
                    <srsid>3452</srsid>
                    <srid>4326</srid>
                    <authid>EPSG:4326</authid>
                    <description>WGS 84</description>
                    <projectionacronym>longlat</projectionacronym>
                    <ellipsoidacronym>WGS84</ellipsoidacronym>
                    <geographicflag>true</geographicflag>
                </spatialrefsys>
            </srs>
            <transparencyLevelInt>0</transparencyLevelInt>
            <provider encoding="UTF-8">postgres</provider>
            <vectorjoins/>
            <singlesymbol>
                <symbol>
                    <lowervalue></lowervalue>
                    <uppervalue></uppervalue>
                    <label></label>
                    <pointsymbol>hard:circle</pointsymbol>
                    <pointsize>2</pointsize>
                    <pointsizeunits>pixels</pointsizeunits>
                    <rotationclassificationfieldname></rotationclassificationfieldname>
                    <scaleclassificationfieldname></scaleclassificationfieldname>
                    <symbolfieldname></symbolfieldname>
                    <outlinecolor red="195" blue="231" green="102"/>
                    <outlinestyle>SolidLine</outlinestyle>
                    <outlinewidth>0.26</outlinewidth>
                    <fillcolor red="0" blue="0" green="0"/>
                    <fillpattern>NoBrush</fillpattern>
                    <texturepath></texturepath>
                </symbol>
            </singlesymbol>
            <customproperties>
                <property key="labeling" value="pal"/>
                <property key="labeling/addDirectionSymbol" value="false"/>
                <property key="labeling/bufferColorB" value="255"/>
                <property key="labeling/bufferColorG" value="255"/>
                <property key="labeling/bufferColorR" value="255"/>
                <property key="labeling/bufferSize" value="1"/>
                <property key="labeling/dataDefinedProperty0" value=""/>
                <property key="labeling/dataDefinedProperty1" value=""/>
                <property key="labeling/dataDefinedProperty10" value=""/>
                <property key="labeling/dataDefinedProperty11" value=""/>
                <property key="labeling/dataDefinedProperty12" value=""/>
                <property key="labeling/dataDefinedProperty13" value=""/>
                <property key="labeling/dataDefinedProperty14" value=""/>
                <property key="labeling/dataDefinedProperty2" value=""/>
                <property key="labeling/dataDefinedProperty3" value=""/>
                <property key="labeling/dataDefinedProperty4" value=""/>
                <property key="labeling/dataDefinedProperty5" value=""/>
                <property key="labeling/dataDefinedProperty6" value=""/>
                <property key="labeling/dataDefinedProperty7" value=""/>
                <property key="labeling/dataDefinedProperty8" value=""/>
                <property key="labeling/dataDefinedProperty9" value=""/>
                <property key="labeling/dist" value="4"/>
                <property key="labeling/distInMapUnits" value="false"/>
                <property key="labeling/enabled" value="true"/>
                <property key="labeling/fieldName" value="right_face"/>
                <property key="labeling/fontFamily" value="Sans"/>
                <property key="labeling/fontItalic" value="false"/>
                <property key="labeling/fontSize" value="7"/>
                <property key="labeling/fontSizeInMapUnits" value="false"/>
                <property key="labeling/fontStrikeout" value="false"/>
                <property key="labeling/fontUnderline" value="false"/>
                <property key="labeling/fontWeight" value="50"/>
                <property key="labeling/labelPerPart" value="false"/>
                <property key="labeling/mergeLines" value="false"/>
                <property key="labeling/minFeatureSize" value="0"/>
                <property key="labeling/multiLineLabels" value="false"/>
                <property key="labeling/obstacle" value="true"/>
                <property key="labeling/placement" value="2"/>
                <property key="labeling/placementFlags" value="4"/>
                <property key="labeling/priority" value="5"/>
                <property key="labeling/scaleMax" value="0"/>
                <property key="labeling/scaleMin" value="0"/>
                <property key="labeling/textColorB" value="0"/>
                <property key="labeling/textColorG" value="0"/>
                <property key="labeling/textColorR" value="170"/>
            </customproperties>
            <displayfield>edge_id</displayfield>
            <label>0</label>
            <labelattributes>
                <label fieldname="" text="Label"/>
                <family fieldname="" name="Sans"/>
                <size fieldname="" units="pt" value="12"/>
                <bold fieldname="" on="0"/>
                <italic fieldname="" on="0"/>
                <underline fieldname="" on="0"/>
                <strikeout fieldname="" on="0"/>
                <color fieldname="" red="0" blue="0" green="0"/>
                <x fieldname=""/>
                <y fieldname=""/>
                <offset x="0" y="0" units="pt" yfieldname="" xfieldname=""/>
                <angle fieldname="" value="0" auto="0"/>
                <alignment fieldname="" value="center"/>
                <buffercolor fieldname="" red="255" blue="255" green="255"/>
                <buffersize fieldname="" units="pt" value="1"/>
                <bufferenabled fieldname="" on=""/>
                <multilineenabled fieldname="" on=""/>
                <selectedonly on=""/>
            </labelattributes>
            <edittypes>
                <edittype type="0" name="abs_next_left_edge"/>
                <edittype type="0" name="abs_next_right_edge"/>
                <edittype type="0" name="edge_id"/>
                <edittype type="0" name="end_node"/>
                <edittype type="0" name="left_face"/>
                <edittype type="0" name="next_left_edge"/>
                <edittype type="0" name="next_right_edge"/>
                <edittype type="0" name="right_face"/>
                <edittype type="0" name="start_node"/>
            </edittypes>
            <editform></editform>
            <editforminit></editforminit>
            <annotationform></annotationform>
            <attributeactions/>
        </maplayer>
        <maplayer minimumScale="0" maximumScale="1e+08" minLabelScale="0" maxLabelScale="1e+08" geometry="Line" type="vector" hasScaleBasedVisibilityFlag="0" scaleBasedLabelVisibilityFlag="0">
            <id>edge_data20110504112431391</id>
            <datasource>dbname='@@DBNAME@@' key='edge_id' table="@@TOPONAME@@"."edge_data" (geom) sql=</datasource>
            <layername>next_left_face</layername>
            <srs>
                <spatialrefsys>
                    <proj4>+proj=longlat +ellps=WGS84 +towgs84=0,0,0,0,0,0,0 +no_defs</proj4>
                    <srsid>3452</srsid>
                    <srid>4326</srid>
                    <authid>EPSG:4326</authid>
                    <description>WGS 84</description>
                    <projectionacronym>longlat</projectionacronym>
                    <ellipsoidacronym>WGS84</ellipsoidacronym>
                    <geographicflag>true</geographicflag>
                </spatialrefsys>
            </srs>
            <transparencyLevelInt>0</transparencyLevelInt>
            <provider encoding="UTF-8">postgres</provider>
            <vectorjoins/>
            <singlesymbol>
                <symbol>
                    <lowervalue></lowervalue>
                    <uppervalue></uppervalue>
                    <label></label>
                    <pointsymbol>hard:circle</pointsymbol>
                    <pointsize>2</pointsize>
                    <pointsizeunits>pixels</pointsizeunits>
                    <rotationclassificationfieldname></rotationclassificationfieldname>
                    <scaleclassificationfieldname></scaleclassificationfieldname>
                    <symbolfieldname></symbolfieldname>
                    <outlinecolor red="133" blue="203" green="231"/>
                    <outlinestyle>SolidLine</outlinestyle>
                    <outlinewidth>0.26</outlinewidth>
                    <fillcolor red="0" blue="0" green="0"/>
                    <fillpattern>NoBrush</fillpattern>
                    <texturepath></texturepath>
                </symbol>
            </singlesymbol>
            <customproperties>
                <property key="labeling" value="pal"/>
                <property key="labeling/addDirectionSymbol" value="false"/>
                <property key="labeling/bufferColorB" value="255"/>
                <property key="labeling/bufferColorG" value="255"/>
                <property key="labeling/bufferColorR" value="255"/>
                <property key="labeling/bufferSize" value="1"/>
                <property key="labeling/dataDefinedProperty0" value=""/>
                <property key="labeling/dataDefinedProperty1" value=""/>
                <property key="labeling/dataDefinedProperty10" value=""/>
                <property key="labeling/dataDefinedProperty11" value=""/>
                <property key="labeling/dataDefinedProperty12" value=""/>
                <property key="labeling/dataDefinedProperty13" value=""/>
                <property key="labeling/dataDefinedProperty14" value=""/>
                <property key="labeling/dataDefinedProperty2" value=""/>
                <property key="labeling/dataDefinedProperty3" value=""/>
                <property key="labeling/dataDefinedProperty4" value=""/>
                <property key="labeling/dataDefinedProperty5" value=""/>
                <property key="labeling/dataDefinedProperty6" value=""/>
                <property key="labeling/dataDefinedProperty7" value=""/>
                <property key="labeling/dataDefinedProperty8" value=""/>
                <property key="labeling/dataDefinedProperty9" value=""/>
                <property key="labeling/dist" value="0"/>
                <property key="labeling/distInMapUnits" value="false"/>
                <property key="labeling/enabled" value="true"/>
                <property key="labeling/fieldName" value="next_left_edge"/>
                <property key="labeling/fontFamily" value="Sans"/>
                <property key="labeling/fontItalic" value="false"/>
                <property key="labeling/fontSize" value="7"/>
                <property key="labeling/fontSizeInMapUnits" value="false"/>
                <property key="labeling/fontStrikeout" value="false"/>
                <property key="labeling/fontUnderline" value="false"/>
                <property key="labeling/fontWeight" value="50"/>
                <property key="labeling/labelPerPart" value="false"/>
                <property key="labeling/mergeLines" value="false"/>
                <property key="labeling/minFeatureSize" value="0"/>
                <property key="labeling/multiLineLabels" value="false"/>
                <property key="labeling/obstacle" value="true"/>
                <property key="labeling/placement" value="2"/>
                <property key="labeling/placementFlags" value="9"/>
                <property key="labeling/priority" value="5"/>
                <property key="labeling/scaleMax" value="0"/>
                <property key="labeling/scaleMin" value="0"/>
                <property key="labeling/textColorB" value="0"/>
                <property key="labeling/textColorG" value="170"/>
                <property key="labeling/textColorR" value="0"/>
            </customproperties>
            <displayfield>edge_id</displayfield>
            <label>0</label>
            <labelattributes>
                <label fieldname="" text="Label"/>
                <family fieldname="" name="Sans"/>
                <size fieldname="" units="pt" value="12"/>
                <bold fieldname="" on="0"/>
                <italic fieldname="" on="0"/>
                <underline fieldname="" on="0"/>
                <strikeout fieldname="" on="0"/>
                <color fieldname="" red="0" blue="0" green="0"/>
                <x fieldname=""/>
                <y fieldname=""/>
                <offset x="0" y="0" units="pt" yfieldname="" xfieldname=""/>
                <angle fieldname="" value="0" auto="0"/>
                <alignment fieldname="" value="center"/>
                <buffercolor fieldname="" red="255" blue="255" green="255"/>
                <buffersize fieldname="" units="pt" value="1"/>
                <bufferenabled fieldname="" on=""/>
                <multilineenabled fieldname="" on=""/>
                <selectedonly on=""/>
            </labelattributes>
            <edittypes>
                <edittype type="0" name="abs_next_left_edge"/>
                <edittype type="0" name="abs_next_right_edge"/>
                <edittype type="0" name="edge_id"/>
                <edittype type="0" name="end_node"/>
                <edittype type="0" name="left_face"/>
                <edittype type="0" name="next_left_edge"/>
                <edittype type="0" name="next_right_edge"/>
                <edittype type="0" name="right_face"/>
                <edittype type="0" name="start_node"/>
            </edittypes>
            <editform></editform>
            <editforminit></editforminit>
            <annotationform></annotationform>
            <attributeactions/>
        </maplayer>
        <maplayer minimumScale="0" maximumScale="1e+08" minLabelScale="0" maxLabelScale="1e+08" geometry="Line" type="vector" hasScaleBasedVisibilityFlag="0" scaleBasedLabelVisibilityFlag="0">
            <id>edge_data20110504112636129</id>
            <datasource>dbname='@@DBNAME@@' key='edge_id' table="@@TOPONAME@@"."edge_data" (geom) sql=</datasource>
            <layername>next_right_face</layername>
            <srs>
                <spatialrefsys>
                    <proj4>+proj=longlat +ellps=WGS84 +towgs84=0,0,0,0,0,0,0 +no_defs</proj4>
                    <srsid>3452</srsid>
                    <srid>4326</srid>
                    <authid>EPSG:4326</authid>
                    <description>WGS 84</description>
                    <projectionacronym>longlat</projectionacronym>
                    <ellipsoidacronym>WGS84</ellipsoidacronym>
                    <geographicflag>true</geographicflag>
                </spatialrefsys>
            </srs>
            <transparencyLevelInt>0</transparencyLevelInt>
            <provider encoding="UTF-8">postgres</provider>
            <vectorjoins/>
            <singlesymbol>
                <symbol>
                    <lowervalue></lowervalue>
                    <uppervalue></uppervalue>
                    <label></label>
                    <pointsymbol>hard:circle</pointsymbol>
                    <pointsize>2</pointsize>
                    <pointsizeunits>pixels</pointsizeunits>
                    <rotationclassificationfieldname></rotationclassificationfieldname>
                    <scaleclassificationfieldname></scaleclassificationfieldname>
                    <symbolfieldname></symbolfieldname>
                    <outlinecolor red="28" blue="179" green="160"/>
                    <outlinestyle>SolidLine</outlinestyle>
                    <outlinewidth>0.26</outlinewidth>
                    <fillcolor red="0" blue="0" green="0"/>
                    <fillpattern>NoBrush</fillpattern>
                    <texturepath></texturepath>
                </symbol>
            </singlesymbol>
            <customproperties>
                <property key="labeling" value="pal"/>
                <property key="labeling/addDirectionSymbol" value="false"/>
                <property key="labeling/bufferColorB" value="255"/>
                <property key="labeling/bufferColorG" value="255"/>
                <property key="labeling/bufferColorR" value="255"/>
                <property key="labeling/bufferSize" value="1"/>
                <property key="labeling/dataDefinedProperty0" value=""/>
                <property key="labeling/dataDefinedProperty1" value=""/>
                <property key="labeling/dataDefinedProperty10" value=""/>
                <property key="labeling/dataDefinedProperty11" value=""/>
                <property key="labeling/dataDefinedProperty12" value=""/>
                <property key="labeling/dataDefinedProperty13" value=""/>
                <property key="labeling/dataDefinedProperty14" value=""/>
                <property key="labeling/dataDefinedProperty2" value=""/>
                <property key="labeling/dataDefinedProperty3" value=""/>
                <property key="labeling/dataDefinedProperty4" value=""/>
                <property key="labeling/dataDefinedProperty5" value=""/>
                <property key="labeling/dataDefinedProperty6" value=""/>
                <property key="labeling/dataDefinedProperty7" value=""/>
                <property key="labeling/dataDefinedProperty8" value=""/>
                <property key="labeling/dataDefinedProperty9" value=""/>
                <property key="labeling/dist" value="0"/>
                <property key="labeling/distInMapUnits" value="false"/>
                <property key="labeling/enabled" value="true"/>
                <property key="labeling/fieldName" value="next_right_edge"/>
                <property key="labeling/fontFamily" value="Sans"/>
                <property key="labeling/fontItalic" value="false"/>
                <property key="labeling/fontSize" value="6"/>
                <property key="labeling/fontSizeInMapUnits" value="false"/>
                <property key="labeling/fontStrikeout" value="false"/>
                <property key="labeling/fontUnderline" value="false"/>
                <property key="labeling/fontWeight" value="50"/>
                <property key="labeling/labelPerPart" value="false"/>
                <property key="labeling/mergeLines" value="false"/>
                <property key="labeling/minFeatureSize" value="0"/>
                <property key="labeling/multiLineLabels" value="false"/>
                <property key="labeling/obstacle" value="true"/>
                <property key="labeling/placement" value="2"/>
                <property key="labeling/placementFlags" value="9"/>
                <property key="labeling/priority" value="5"/>
                <property key="labeling/scaleMax" value="0"/>
                <property key="labeling/scaleMin" value="0"/>
                <property key="labeling/textColorB" value="0"/>
                <property key="labeling/textColorG" value="85"/>
                <property key="labeling/textColorR" value="170"/>
            </customproperties>
            <displayfield>edge_id</displayfield>
            <label>0</label>
            <labelattributes>
                <label fieldname="" text="Label"/>
                <family fieldname="" name="Sans"/>
                <size fieldname="" units="pt" value="12"/>
                <bold fieldname="" on="0"/>
                <italic fieldname="" on="0"/>
                <underline fieldname="" on="0"/>
                <strikeout fieldname="" on="0"/>
                <color fieldname="" red="0" blue="0" green="0"/>
                <x fieldname=""/>
                <y fieldname=""/>
                <offset x="0" y="0" units="pt" yfieldname="" xfieldname=""/>
                <angle fieldname="" value="0" auto="0"/>
                <alignment fieldname="" value="center"/>
                <buffercolor fieldname="" red="255" blue="255" green="255"/>
                <buffersize fieldname="" units="pt" value="1"/>
                <bufferenabled fieldname="" on=""/>
                <multilineenabled fieldname="" on=""/>
                <selectedonly on=""/>
            </labelattributes>
            <edittypes>
                <edittype type="0" name="abs_next_left_edge"/>
                <edittype type="0" name="abs_next_right_edge"/>
                <edittype type="0" name="edge_id"/>
                <edittype type="0" name="end_node"/>
                <edittype type="0" name="left_face"/>
                <edittype type="0" name="next_left_edge"/>
                <edittype type="0" name="next_right_edge"/>
                <edittype type="0" name="right_face"/>
                <edittype type="0" name="start_node"/>
            </edittypes>
            <editform></editform>
            <editforminit></editforminit>
            <annotationform></annotationform>
            <attributeactions/>
        </maplayer>
        <maplayer minimumScale="0" maximumScale="1e+08" minLabelScale="0" maxLabelScale="1e+08" geometry="Point" type="vector" hasScaleBasedVisibilityFlag="0" scaleBasedLabelVisibilityFlag="0">
            <id>node20110504083015129</id>
            <datasource>dbname='@@DBNAME@@' key='node_id' table="@@TOPONAME@@"."node" (geom) sql=</datasource>
            <layername>node</layername>
            <srs>
                <spatialrefsys>
                    <proj4>+proj=longlat +ellps=WGS84 +towgs84=0,0,0,0,0,0,0 +no_defs</proj4>
                    <srsid>3452</srsid>
                    <srid>4326</srid>
                    <authid>EPSG:4326</authid>
                    <description>WGS 84</description>
                    <projectionacronym>longlat</projectionacronym>
                    <ellipsoidacronym>WGS84</ellipsoidacronym>
                    <geographicflag>true</geographicflag>
                </spatialrefsys>
            </srs>
            <transparencyLevelInt>255</transparencyLevelInt>
            <provider encoding="UTF-8">postgres</provider>
            <vectorjoins/>
            <singlesymbol>
                <symbol>
                    <lowervalue></lowervalue>
                    <uppervalue></uppervalue>
                    <label></label>
                    <pointsymbol>hard:circle</pointsymbol>
                    <pointsize>4</pointsize>
                    <pointsizeunits>pixels</pointsizeunits>
                    <rotationclassificationfieldname></rotationclassificationfieldname>
                    <scaleclassificationfieldname></scaleclassificationfieldname>
                    <symbolfieldname></symbolfieldname>
                    <outlinecolor red="0" blue="0" green="0"/>
                    <outlinestyle>SolidLine</outlinestyle>
                    <outlinewidth>0.26</outlinewidth>
                    <fillcolor red="196" blue="176" green="201"/>
                    <fillpattern>SolidPattern</fillpattern>
                    <texturepath></texturepath>
                </symbol>
            </singlesymbol>
            <customproperties>
                <property key="labeling" value="pal"/>
                <property key="labeling/addDirectionSymbol" value="false"/>
                <property key="labeling/bufferColorB" value="255"/>
                <property key="labeling/bufferColorG" value="255"/>
                <property key="labeling/bufferColorR" value="255"/>
                <property key="labeling/bufferSize" value="1"/>
                <property key="labeling/dataDefinedProperty0" value=""/>
                <property key="labeling/dataDefinedProperty1" value=""/>
                <property key="labeling/dataDefinedProperty10" value=""/>
                <property key="labeling/dataDefinedProperty11" value=""/>
                <property key="labeling/dataDefinedProperty12" value=""/>
                <property key="labeling/dataDefinedProperty13" value=""/>
                <property key="labeling/dataDefinedProperty14" value=""/>
                <property key="labeling/dataDefinedProperty2" value=""/>
                <property key="labeling/dataDefinedProperty3" value=""/>
                <property key="labeling/dataDefinedProperty4" value=""/>
                <property key="labeling/dataDefinedProperty5" value=""/>
                <property key="labeling/dataDefinedProperty6" value=""/>
                <property key="labeling/dataDefinedProperty7" value=""/>
                <property key="labeling/dataDefinedProperty8" value=""/>
                <property key="labeling/dataDefinedProperty9" value=""/>
                <property key="labeling/dist" value="0"/>
                <property key="labeling/distInMapUnits" value="false"/>
                <property key="labeling/enabled" value="true"/>
                <property key="labeling/fieldName" value="node_id"/>
                <property key="labeling/fontFamily" value="Sans"/>
                <property key="labeling/fontItalic" value="false"/>
                <property key="labeling/fontSize" value="8"/>
                <property key="labeling/fontSizeInMapUnits" value="false"/>
                <property key="labeling/fontStrikeout" value="false"/>
                <property key="labeling/fontUnderline" value="false"/>
                <property key="labeling/fontWeight" value="50"/>
                <property key="labeling/labelPerPart" value="false"/>
                <property key="labeling/mergeLines" value="false"/>
                <property key="labeling/minFeatureSize" value="0"/>
                <property key="labeling/multiLineLabels" value="false"/>
                <property key="labeling/obstacle" value="true"/>
                <property key="labeling/placement" value="1"/>
                <property key="labeling/placementFlags" value="0"/>
                <property key="labeling/priority" value="5"/>
                <property key="labeling/scaleMax" value="0"/>
                <property key="labeling/scaleMin" value="0"/>
                <property key="labeling/textColorB" value="217"/>
                <property key="labeling/textColorG" value="41"/>
                <property key="labeling/textColorR" value="14"/>
            </customproperties>
            <displayfield>node_id</displayfield>
            <label>0</label>
            <labelattributes>
                <label fieldname="" text="Label"/>
                <family fieldname="" name="Sans"/>
                <size fieldname="" units="pt" value="12"/>
                <bold fieldname="" on="0"/>
                <italic fieldname="" on="0"/>
                <underline fieldname="" on="0"/>
                <strikeout fieldname="" on="0"/>
                <color fieldname="" red="0" blue="0" green="0"/>
                <x fieldname=""/>
                <y fieldname=""/>
                <offset x="0" y="0" units="pt" yfieldname="" xfieldname=""/>
                <angle fieldname="" value="0" auto="0"/>
                <alignment fieldname="" value="center"/>
                <buffercolor fieldname="" red="255" blue="255" green="255"/>
                <buffersize fieldname="" units="pt" value="1"/>
                <bufferenabled fieldname="" on=""/>
                <multilineenabled fieldname="" on=""/>
                <selectedonly on=""/>
            </labelattributes>
            <edittypes>
                <edittype type="0" name="containing_face"/>
                <edittype type="0" name="node_id"/>
            </edittypes>
            <editform></editform>
            <editforminit></editforminit>
            <annotationform></annotationform>
            <attributeactions/>
        </maplayer>
    </projectlayers>
    <properties>
        <SpatialRefSys>
            <ProjectCrs type="QString">EPSG:4326</ProjectCrs>
        </SpatialRefSys>
        <Gui>
            <SelectionColorBluePart type="int">0</SelectionColorBluePart>
            <CanvasColorGreenPart type="int">255</CanvasColorGreenPart>
            <CanvasColorRedPart type="int">255</CanvasColorRedPart>
            <SelectionColorRedPart type="int">255</SelectionColorRedPart>
            <SelectionColorAlphaPart type="int">255</SelectionColorAlphaPart>
            <SelectionColorGreenPart type="int">255</SelectionColorGreenPart>
            <CanvasColorBluePart type="int">255</CanvasColorBluePart>
        </Gui>
        <PositionPrecision>
            <DecimalPlaces type="int">2</DecimalPlaces>
            <Automatic type="bool">true</Automatic>
        </PositionPrecision>
    </properties>
</qgis>
