<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis minScale="1e+8" simplifyAlgorithm="0" simplifyDrawingTol="1" simplifyLocal="1" simplifyDrawingHints="0" readOnly="0" version="3.3.0-Master" hasScaleBasedVisibilityFlag="0" simplifyMaxScale="1" maxScale="0" labelsEnabled="0">
  <renderer-v2 forceraster="0" symbollevels="0" type="singleSymbol" enableorderby="0">
    <symbols>
      <symbol alpha="1" type="marker" name="0" clip_to_extent="1">
        <layer locked="0" pass="0" enabled="1" class="SimpleMarker">
          <prop k="angle" v="0"/>
          <prop k="color" v="222,12,61,255"/>
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
              <Option value="" type="QString" name="name"/>
              <Option type="Map" name="properties">
                <Option type="Map" name="size">
                  <Option value="true" type="bool" name="active"/>
                  <Option value="rand(10, 20)" type="QString" name="expression"/>
                  <Option value="3" type="int" name="type"/>
                </Option>
              </Option>
              <Option value="collection" type="QString" name="type"/>
            </Option>
          </data_defined_properties>
        </layer>
        <layer locked="0" pass="0" enabled="1" class="SimpleMarker">
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
          <prop k="size" v="6"/>
          <prop k="size_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="size_unit" v="MM"/>
          <prop k="vertical_anchor_point" v="1"/>
          <data_defined_properties>
            <Option type="Map">
              <Option value="" type="QString" name="name"/>
              <Option name="properties"/>
              <Option value="collection" type="QString" name="type"/>
            </Option>
          </data_defined_properties>
        </layer>
      </symbol>
    </symbols>
    <rotation/>
    <sizescale/>
  </renderer-v2>
  <customproperties>
    <property value="0" key="embeddedWidgets/count"/>
    <property key="variableNames"/>
    <property key="variableValues"/>
  </customproperties>
  <blendMode>0</blendMode>
  <featureBlendMode>0</featureBlendMode>
  <layerOpacity>1</layerOpacity>
  <SingleCategoryDiagramRenderer diagramType="Histogram" attributeLegend="1">
    <DiagramCategory enabled="0" minScaleDenominator="0" penWidth="0" sizeType="MM" lineSizeType="MM" height="15" diagramOrientation="Up" scaleBasedVisibility="0" minimumSize="0" lineSizeScale="3x:0,0,0,0,0,0" scaleDependency="Area" backgroundAlpha="255" opacity="1" width="15" penColor="#000000" barWidth="5" maxScaleDenominator="1e+8" labelPlacementMethod="XHeight" penAlpha="255" sizeScale="3x:0,0,0,0,0,0" rotationOffset="270" backgroundColor="#ffffff">
      <fontProperties description=".SF NS Text,13,-1,5,50,0,0,0,0,0" style=""/>
      <attribute label="" field="" color="#000000"/>
    </DiagramCategory>
  </SingleCategoryDiagramRenderer>
  <DiagramLayerSettings obstacle="0" priority="0" placement="0" dist="0" linePlacementFlags="18" zIndex="0" showAll="1">
    <properties>
      <Option type="Map">
        <Option value="" type="QString" name="name"/>
        <Option name="properties"/>
        <Option value="collection" type="QString" name="type"/>
      </Option>
    </properties>
  </DiagramLayerSettings>
  <fieldConfiguration>
    <field name="year">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="month">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="hackfest_number">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="place">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="notes">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="month_int">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="day_int">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="date_nice">
      <editWidget type="DateTime">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
  </fieldConfiguration>
  <geometryOptions geometryPrecision="0" removeDuplicateNodes="0"/>
  <aliases>
    <alias field="year" index="0" name=""/>
    <alias field="month" index="1" name=""/>
    <alias field="hackfest_number" index="2" name=""/>
    <alias field="place" index="3" name=""/>
    <alias field="notes" index="4" name=""/>
    <alias field="month_int" index="5" name=""/>
    <alias field="day_int" index="6" name=""/>
    <alias field="date_nice" index="7" name=""/>
  </aliases>
  <excludeAttributesWMS/>
  <excludeAttributesWFS/>
  <defaults>
    <default field="year" expression="" applyOnUpdate="0"/>
    <default field="month" expression="" applyOnUpdate="0"/>
    <default field="hackfest_number" expression="" applyOnUpdate="0"/>
    <default field="place" expression="" applyOnUpdate="0"/>
    <default field="notes" expression="" applyOnUpdate="0"/>
    <default field="month_int" expression="" applyOnUpdate="0"/>
    <default field="day_int" expression="" applyOnUpdate="0"/>
    <default field="date_nice" expression="" applyOnUpdate="0"/>
  </defaults>
  <constraints>
    <constraint exp_strength="0" field="year" constraints="0" notnull_strength="0" unique_strength="0"/>
    <constraint exp_strength="0" field="month" constraints="0" notnull_strength="0" unique_strength="0"/>
    <constraint exp_strength="0" field="hackfest_number" constraints="0" notnull_strength="0" unique_strength="0"/>
    <constraint exp_strength="0" field="place" constraints="0" notnull_strength="0" unique_strength="0"/>
    <constraint exp_strength="0" field="notes" constraints="0" notnull_strength="0" unique_strength="0"/>
    <constraint exp_strength="0" field="month_int" constraints="0" notnull_strength="0" unique_strength="0"/>
    <constraint exp_strength="0" field="day_int" constraints="0" notnull_strength="0" unique_strength="0"/>
    <constraint exp_strength="0" field="date_nice" constraints="0" notnull_strength="0" unique_strength="0"/>
  </constraints>
  <constraintExpressions>
    <constraint field="year" exp="" desc=""/>
    <constraint field="month" exp="" desc=""/>
    <constraint field="hackfest_number" exp="" desc=""/>
    <constraint field="place" exp="" desc=""/>
    <constraint field="notes" exp="" desc=""/>
    <constraint field="month_int" exp="" desc=""/>
    <constraint field="day_int" exp="" desc=""/>
    <constraint field="date_nice" exp="" desc=""/>
  </constraintExpressions>
  <attributeactions>
    <defaultAction value="{00000000-0000-0000-0000-000000000000}" key="Canvas"/>
  </attributeactions>
  <attributetableconfig sortExpression="" sortOrder="0" actionWidgetStyle="dropDown">
    <columns>
      <column width="-1" type="field" name="year" hidden="0"/>
      <column width="-1" type="field" name="month" hidden="0"/>
      <column width="-1" type="field" name="hackfest_number" hidden="0"/>
      <column width="-1" type="field" name="place" hidden="0"/>
      <column width="-1" type="field" name="notes" hidden="0"/>
      <column width="-1" type="field" name="month_int" hidden="0"/>
      <column width="-1" type="field" name="day_int" hidden="0"/>
      <column width="-1" type="field" name="date_nice" hidden="0"/>
      <column width="-1" type="actions" hidden="1"/>
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
    <field name="date_nice" labelOnTop="0"/>
    <field name="day_int" labelOnTop="0"/>
    <field name="hackfest_number" labelOnTop="0"/>
    <field name="month" labelOnTop="0"/>
    <field name="month_int" labelOnTop="0"/>
    <field name="notes" labelOnTop="0"/>
    <field name="place" labelOnTop="0"/>
    <field name="year" labelOnTop="0"/>
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
