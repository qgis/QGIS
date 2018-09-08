<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis maxScale="0" simplifyDrawingHints="0" simplifyMaxScale="1" version="3.3.0-Master" simplifyAlgorithm="0" simplifyDrawingTol="1" readOnly="0" simplifyLocal="1" searchable="1" hasScaleBasedVisibilityFlag="0" minScale="1e+08" labelsEnabled="0">
  <renderer-v2 enableorderby="0" forceraster="0" type="singleSymbol" symbollevels="0">
    <symbols>
      <symbol alpha="1" clip_to_extent="1" name="0" type="marker">
        <layer locked="0" pass="0" class="SimpleMarker" enabled="1">
          <prop k="angle" v="0"/>
          <prop k="color" v="17,81,210,255"/>
          <prop k="horizontal_anchor_point" v="1"/>
          <prop k="joinstyle" v="bevel"/>
          <prop k="name" v="circle"/>
          <prop k="offset" v="0,0"/>
          <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="offset_unit" v="MM"/>
          <prop k="outline_color" v="35,35,35,255"/>
          <prop k="outline_style" v="no"/>
          <prop k="outline_width" v="0"/>
          <prop k="outline_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="outline_width_unit" v="MM"/>
          <prop k="scale_method" v="diameter"/>
          <prop k="size" v="10"/>
          <prop k="size_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="size_unit" v="MM"/>
          <prop k="vertical_anchor_point" v="1"/>
          <data_defined_properties>
            <Option type="Map">
              <Option name="name" type="QString" value=""/>
              <Option name="properties" type="Map">
                <Option name="size" type="Map">
                  <Option name="active" type="bool" value="false"/>
                  <Option name="type" type="int" value="1"/>
                  <Option name="val" type="QString" value=""/>
                </Option>
              </Option>
              <Option name="type" type="QString" value="collection"/>
            </Option>
          </data_defined_properties>
        </layer>
        <layer locked="0" pass="0" class="SimpleMarker" enabled="1">
          <prop k="angle" v="0"/>
          <prop k="color" v="255,149,0,255"/>
          <prop k="horizontal_anchor_point" v="1"/>
          <prop k="joinstyle" v="bevel"/>
          <prop k="name" v="circle"/>
          <prop k="offset" v="0,0"/>
          <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="offset_unit" v="MM"/>
          <prop k="outline_color" v="35,35,35,255"/>
          <prop k="outline_style" v="no"/>
          <prop k="outline_width" v="0"/>
          <prop k="outline_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="outline_width_unit" v="MM"/>
          <prop k="scale_method" v="diameter"/>
          <prop k="size" v="8"/>
          <prop k="size_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="size_unit" v="MM"/>
          <prop k="vertical_anchor_point" v="1"/>
          <data_defined_properties>
            <Option type="Map">
              <Option name="name" type="QString" value=""/>
              <Option name="properties" type="Map">
                <Option name="fillColor" type="Map">
                  <Option name="active" type="bool" value="true"/>
                  <Option name="expression" type="QString" value="color_hsv(((abs( second(now()) - 30) * 7) +300) % 360, 92, 82)"/>
                  <Option name="type" type="int" value="3"/>
                </Option>
                <Option name="size" type="Map">
                  <Option name="active" type="bool" value="false"/>
                  <Option name="type" type="int" value="1"/>
                  <Option name="val" type="QString" value=""/>
                </Option>
              </Option>
              <Option name="type" type="QString" value="collection"/>
            </Option>
          </data_defined_properties>
        </layer>
        <layer locked="0" pass="0" class="SimpleMarker" enabled="1">
          <prop k="angle" v="0"/>
          <prop k="color" v="255,0,0,255"/>
          <prop k="horizontal_anchor_point" v="1"/>
          <prop k="joinstyle" v="bevel"/>
          <prop k="name" v="cross"/>
          <prop k="offset" v="0,0"/>
          <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="offset_unit" v="MM"/>
          <prop k="outline_color" v="17,81,210,255"/>
          <prop k="outline_style" v="solid"/>
          <prop k="outline_width" v="2"/>
          <prop k="outline_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="outline_width_unit" v="MM"/>
          <prop k="scale_method" v="diameter"/>
          <prop k="size" v="5"/>
          <prop k="size_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="size_unit" v="MM"/>
          <prop k="vertical_anchor_point" v="1"/>
          <data_defined_properties>
            <Option type="Map">
              <Option name="name" type="QString" value=""/>
              <Option name="properties" type="Map">
                <Option name="angle" type="Map">
                  <Option name="active" type="bool" value="true"/>
                  <Option name="expression" type="QString" value="(60 - second(now())) * 6"/>
                  <Option name="type" type="int" value="3"/>
                </Option>
              </Option>
              <Option name="type" type="QString" value="collection"/>
            </Option>
          </data_defined_properties>
        </layer>
      </symbol>
    </symbols>
    <rotation/>
    <sizescale/>
    <effect type="effectStack" enabled="1">
      <effect type="dropShadow">
        <prop k="blend_mode" v="13"/>
        <prop k="blur_level" v="3"/>
        <prop k="color" v="0,0,0,255"/>
        <prop k="draw_mode" v="2"/>
        <prop k="enabled" v="1"/>
        <prop k="offset_angle" v="135"/>
        <prop k="offset_distance" v="2"/>
        <prop k="offset_unit" v="MM"/>
        <prop k="offset_unit_scale" v="3x:0,0,0,0,0,0"/>
        <prop k="opacity" v="0.5"/>
      </effect>
      <effect type="outerGlow">
        <prop k="blend_mode" v="0"/>
        <prop k="blur_level" v="3"/>
        <prop k="color1" v="0,0,255,255"/>
        <prop k="color2" v="0,255,0,255"/>
        <prop k="color_type" v="0"/>
        <prop k="discrete" v="0"/>
        <prop k="draw_mode" v="2"/>
        <prop k="enabled" v="0"/>
        <prop k="opacity" v="0.5"/>
        <prop k="rampType" v="gradient"/>
        <prop k="single_color" v="255,255,255,255"/>
        <prop k="spread" v="2"/>
        <prop k="spread_unit" v="MM"/>
        <prop k="spread_unit_scale" v="3x:0,0,0,0,0,0"/>
      </effect>
      <effect type="drawSource">
        <prop k="blend_mode" v="0"/>
        <prop k="draw_mode" v="2"/>
        <prop k="enabled" v="1"/>
        <prop k="opacity" v="1"/>
      </effect>
      <effect type="innerShadow">
        <prop k="blend_mode" v="13"/>
        <prop k="blur_level" v="10"/>
        <prop k="color" v="0,0,0,255"/>
        <prop k="draw_mode" v="2"/>
        <prop k="enabled" v="0"/>
        <prop k="offset_angle" v="135"/>
        <prop k="offset_distance" v="2"/>
        <prop k="offset_unit" v="MM"/>
        <prop k="offset_unit_scale" v="3x:0,0,0,0,0,0"/>
        <prop k="opacity" v="1"/>
      </effect>
      <effect type="innerGlow">
        <prop k="blend_mode" v="0"/>
        <prop k="blur_level" v="3"/>
        <prop k="color1" v="0,0,255,255"/>
        <prop k="color2" v="0,255,0,255"/>
        <prop k="color_type" v="0"/>
        <prop k="discrete" v="0"/>
        <prop k="draw_mode" v="2"/>
        <prop k="enabled" v="0"/>
        <prop k="opacity" v="0.5"/>
        <prop k="rampType" v="gradient"/>
        <prop k="single_color" v="255,255,255,255"/>
        <prop k="spread" v="2"/>
        <prop k="spread_unit" v="MM"/>
        <prop k="spread_unit_scale" v="3x:0,0,0,0,0,0"/>
      </effect>
    </effect>
  </renderer-v2>
  <labeling type="simple">
    <settings>
      <text-style fontCapitals="0" isExpression="1" previewBkgrdColor="#ffffff" fontWeight="50" fontSize="10" textOpacity="1" textColor="0,0,0,255" multilineHeight="1" fontStrikeout="0" fontWordSpacing="0" namedStyle="Normal" fontUnderline="0" useSubstitutions="0" blendMode="0" fontLetterSpacing="0" fontSizeUnit="Point" fontSizeMapUnitScale="3x:0,0,0,0,0,0" fieldName="((abs(second(now())-30) * 7) +300) % 360" fontItalic="0" fontFamily="Sans Serif">
        <text-buffer bufferSize="1" bufferSizeMapUnitScale="3x:0,0,0,0,0,0" bufferBlendMode="0" bufferColor="255,255,255,255" bufferNoFill="1" bufferOpacity="1" bufferDraw="0" bufferSizeUnits="MM" bufferJoinStyle="128"/>
        <background shapeType="0" shapeJoinStyle="64" shapeOffsetY="0" shapeSVGFile="" shapeSizeMapUnitScale="3x:0,0,0,0,0,0" shapeSizeType="0" shapeBorderWidth="0" shapeOpacity="1" shapeRotation="0" shapeRadiiMapUnitScale="3x:0,0,0,0,0,0" shapeSizeUnit="MM" shapeOffsetMapUnitScale="3x:0,0,0,0,0,0" shapeDraw="0" shapeOffsetUnit="MM" shapeBorderColor="128,128,128,255" shapeRadiiY="0" shapeRadiiUnit="MM" shapeBorderWidthMapUnitScale="3x:0,0,0,0,0,0" shapeSizeY="0" shapeRotationType="0" shapeBlendMode="0" shapeOffsetX="0" shapeBorderWidthUnit="MM" shapeFillColor="255,255,255,255" shapeSizeX="0" shapeRadiiX="0"/>
        <shadow shadowDraw="0" shadowColor="0,0,0,255" shadowOffsetAngle="135" shadowRadiusAlphaOnly="0" shadowOpacity="0.7" shadowOffsetMapUnitScale="3x:0,0,0,0,0,0" shadowBlendMode="6" shadowOffsetDist="1" shadowOffsetUnit="MM" shadowOffsetGlobal="1" shadowRadiusUnit="MM" shadowRadiusMapUnitScale="3x:0,0,0,0,0,0" shadowScale="100" shadowUnder="0" shadowRadius="1.5"/>
        <substitutions/>
      </text-style>
      <text-format leftDirectionSymbol="&lt;" placeDirectionSymbol="0" rightDirectionSymbol=">" reverseDirectionSymbol="0" formatNumbers="0" decimals="3" plussign="0" wrapChar="" multilineAlign="3" addDirectionSymbol="0"/>
      <placement labelOffsetMapUnitScale="3x:0,0,0,0,0,0" maxCurvedCharAngleOut="-25" centroidWhole="0" repeatDistanceUnits="MM" distUnits="MM" centroidInside="0" rotationAngle="0" maxCurvedCharAngleIn="25" predefinedPositionOrder="TR,TL,BR,BL,R,L,TSR,BSR" offsetUnits="MM" xOffset="0" preserveRotation="1" dist="0" yOffset="0" distMapUnitScale="3x:0,0,0,0,0,0" offsetType="0" priority="5" fitInPolygonOnly="0" placementFlags="10" placement="0" quadOffset="4" repeatDistance="0" repeatDistanceMapUnitScale="3x:0,0,0,0,0,0"/>
      <rendering zIndex="0" scaleVisibility="0" scaleMin="0" upsidedownLabels="0" drawLabels="1" labelPerPart="0" fontLimitPixelSize="0" limitNumLabels="0" fontMinPixelSize="3" minFeatureSize="0" scaleMax="0" obstacle="1" fontMaxPixelSize="10000" displayAll="0" maxNumLabels="2000" mergeLines="0" obstacleType="0" obstacleFactor="1"/>
      <dd_properties>
        <Option type="Map">
          <Option name="name" type="QString" value=""/>
          <Option name="properties"/>
          <Option name="type" type="QString" value="collection"/>
        </Option>
      </dd_properties>
    </settings>
  </labeling>
  <customproperties>
    <property key="embeddedWidgets/count" value="0"/>
    <property key="variableNames"/>
    <property key="variableValues"/>
  </customproperties>
  <blendMode>0</blendMode>
  <featureBlendMode>0</featureBlendMode>
  <layerOpacity>1</layerOpacity>
  <SingleCategoryDiagramRenderer attributeLegend="1" diagramType="Histogram">
    <DiagramCategory lineSizeType="MM" sizeType="MM" enabled="0" penAlpha="255" penColor="#000000" minimumSize="0" diagramOrientation="Up" backgroundAlpha="255" rotationOffset="270" width="15" labelPlacementMethod="XHeight" scaleBasedVisibility="0" lineSizeScale="3x:0,0,0,0,0,0" minScaleDenominator="0" sizeScale="3x:0,0,0,0,0,0" backgroundColor="#ffffff" barWidth="5" maxScaleDenominator="1e+08" penWidth="0" opacity="1" height="15" scaleDependency="Area">
      <fontProperties style="" description=".SF NS Text,13,-1,5,50,0,0,0,0,0"/>
      <attribute field="" label="" color="#000000"/>
    </DiagramCategory>
  </SingleCategoryDiagramRenderer>
  <DiagramLayerSettings placement="0" showAll="1" linePlacementFlags="18" priority="0" zIndex="0" dist="0" obstacle="0">
    <properties>
      <Option type="Map">
        <Option name="name" type="QString" value=""/>
        <Option name="properties"/>
        <Option name="type" type="QString" value="collection"/>
      </Option>
    </properties>
  </DiagramLayerSettings>
  <fieldConfiguration>
    <field name="Name">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="Committer">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="First Commit Message">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="First Commit Date">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="GIT Nickname">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
  </fieldConfiguration>
  <geometryOptions removeDuplicateNodes="0" geometryPrecision="0"/>
  <aliases>
    <alias name="" index="0" field="Name"/>
    <alias name="" index="1" field="Committer"/>
    <alias name="" index="2" field="First Commit Message"/>
    <alias name="" index="3" field="First Commit Date"/>
    <alias name="" index="4" field="GIT Nickname"/>
  </aliases>
  <excludeAttributesWMS/>
  <excludeAttributesWFS/>
  <defaults>
    <default expression="" field="Name" applyOnUpdate="0"/>
    <default expression="" field="Committer" applyOnUpdate="0"/>
    <default expression="" field="First Commit Message" applyOnUpdate="0"/>
    <default expression="" field="First Commit Date" applyOnUpdate="0"/>
    <default expression="" field="GIT Nickname" applyOnUpdate="0"/>
  </defaults>
  <constraints>
    <constraint notnull_strength="0" exp_strength="0" unique_strength="0" field="Name" constraints="0"/>
    <constraint notnull_strength="0" exp_strength="0" unique_strength="0" field="Committer" constraints="0"/>
    <constraint notnull_strength="0" exp_strength="0" unique_strength="0" field="First Commit Message" constraints="0"/>
    <constraint notnull_strength="0" exp_strength="0" unique_strength="0" field="First Commit Date" constraints="0"/>
    <constraint notnull_strength="0" exp_strength="0" unique_strength="0" field="GIT Nickname" constraints="0"/>
  </constraints>
  <constraintExpressions>
    <constraint desc="" exp="" field="Name"/>
    <constraint desc="" exp="" field="Committer"/>
    <constraint desc="" exp="" field="First Commit Message"/>
    <constraint desc="" exp="" field="First Commit Date"/>
    <constraint desc="" exp="" field="GIT Nickname"/>
  </constraintExpressions>
  <attributeactions>
    <defaultAction key="Canvas" value="{00000000-0000-0000-0000-000000000000}"/>
  </attributeactions>
  <attributetableconfig sortExpression="" sortOrder="0" actionWidgetStyle="dropDown">
    <columns>
      <column hidden="1" width="-1" type="actions"/>
      <column hidden="0" name="Name" width="-1" type="field"/>
      <column hidden="0" name="Committer" width="-1" type="field"/>
      <column hidden="0" name="First Commit Message" width="-1" type="field"/>
      <column hidden="0" name="First Commit Date" width="-1" type="field"/>
      <column hidden="0" name="GIT Nickname" width="-1" type="field"/>
    </columns>
  </attributetableconfig>
  <editform tolerant="1"></editform>
  <editforminit/>
  <editforminitcodesource>0</editforminitcodesource>
  <editforminitfilepath></editforminitfilepath>
  <editforminitcode><![CDATA[# -*- coding: utf-8 -*-
"""
QGIS forms can have a Python function that is called when the form is
opened.

Use this function to add extra logic to your forms.

Enter the name of the function in the "Python Init function"
field.
An example follows:
"""
from qgis.PyQt.QtWidgets import QWidget

def my_form_open(dialog, layer, feature):
	geom = feature.geometry()
	control = dialog.findChild(QWidget, "MyLineEdit")
]]></editforminitcode>
  <featformsuppress>0</featformsuppress>
  <editorlayout>generatedlayout</editorlayout>
  <editable>
    <field editable="1" name="Committer"/>
    <field editable="1" name="First Commit Date"/>
    <field editable="1" name="First Commit Message"/>
    <field editable="1" name="GIT Nickname"/>
    <field editable="1" name="Name"/>
    <field editable="1" name="date_nice"/>
    <field editable="1" name="day_int"/>
    <field editable="1" name="hackfest_number"/>
    <field editable="1" name="month"/>
    <field editable="1" name="month_int"/>
    <field editable="1" name="notes"/>
    <field editable="1" name="place"/>
    <field editable="1" name="year"/>
  </editable>
  <labelOnTop>
    <field labelOnTop="0" name="Committer"/>
    <field labelOnTop="0" name="First Commit Date"/>
    <field labelOnTop="0" name="First Commit Message"/>
    <field labelOnTop="0" name="GIT Nickname"/>
    <field labelOnTop="0" name="Name"/>
    <field labelOnTop="0" name="date_nice"/>
    <field labelOnTop="0" name="day_int"/>
    <field labelOnTop="0" name="hackfest_number"/>
    <field labelOnTop="0" name="month"/>
    <field labelOnTop="0" name="month_int"/>
    <field labelOnTop="0" name="notes"/>
    <field labelOnTop="0" name="place"/>
    <field labelOnTop="0" name="year"/>
  </labelOnTop>
  <widgets/>
  <conditionalstyles>
    <rowstyles/>
    <fieldstyles/>
  </conditionalstyles>
  <expressionfields/>
  <previewExpression>year</previewExpression>
  <mapTip></mapTip>
  <layerGeometryType>0</layerGeometryType>
</qgis>
