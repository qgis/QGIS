<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis maxScale="0" simplifyLocal="1" hasScaleBasedVisibilityFlag="0" simplifyDrawingHints="0" minScale="1e+8" simplifyMaxScale="1" labelsEnabled="0" readOnly="0" simplifyDrawingTol="1" simplifyAlgorithm="0" version="3.3.0-Master">
  <renderer-v2 enableorderby="0" symbollevels="0" type="singleSymbol" forceraster="0">
    <symbols>
      <symbol type="marker" name="0" clip_to_extent="1" alpha="1">
        <layer enabled="1" class="SimpleMarker" pass="0" locked="0">
          <prop k="angle" v="0"/>
          <prop k="color" v="55,126,184,255"/>
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
              <Option type="QString" value="" name="name"/>
              <Option type="Map" name="properties">
                <Option type="Map" name="size">
                  <Option type="bool" value="true" name="active"/>
                  <Option type="QString" value="rand(10, 20)" name="expression"/>
                  <Option type="int" value="3" name="type"/>
                </Option>
              </Option>
              <Option type="QString" value="collection" name="type"/>
            </Option>
          </data_defined_properties>
        </layer>
        <layer enabled="1" class="SimpleMarker" pass="0" locked="0">
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
              <Option type="QString" value="" name="name"/>
              <Option name="properties"/>
              <Option type="QString" value="collection" name="type"/>
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
  <SingleCategoryDiagramRenderer attributeLegend="1" diagramType="Histogram">
    <DiagramCategory penAlpha="255" scaleDependency="Area" maxScaleDenominator="1e+8" penWidth="0" enabled="0" sizeType="MM" backgroundAlpha="255" penColor="#000000" scaleBasedVisibility="0" labelPlacementMethod="XHeight" lineSizeType="MM" minScaleDenominator="0" sizeScale="3x:0,0,0,0,0,0" barWidth="5" lineSizeScale="3x:0,0,0,0,0,0" rotationOffset="270" height="15" width="15" backgroundColor="#ffffff" diagramOrientation="Up" opacity="1" minimumSize="0">
      <fontProperties description=".SF NS Text,13,-1,5,50,0,0,0,0,0" style=""/>
      <attribute color="#000000" label="" field=""/>
    </DiagramCategory>
  </SingleCategoryDiagramRenderer>
  <DiagramLayerSettings linePlacementFlags="18" showAll="1" obstacle="0" dist="0" zIndex="0" placement="0" priority="0">
    <properties>
      <Option type="Map">
        <Option type="QString" value="" name="name"/>
        <Option name="properties"/>
        <Option type="QString" value="collection" name="type"/>
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
    <alias field="Name" name="" index="0"/>
    <alias field="Committer" name="" index="1"/>
    <alias field="First Commit Message" name="" index="2"/>
    <alias field="First Commit Date" name="" index="3"/>
    <alias field="GIT Nickname" name="" index="4"/>
  </aliases>
  <excludeAttributesWMS/>
  <excludeAttributesWFS/>
  <defaults>
    <default applyOnUpdate="0" expression="" field="Name"/>
    <default applyOnUpdate="0" expression="" field="Committer"/>
    <default applyOnUpdate="0" expression="" field="First Commit Message"/>
    <default applyOnUpdate="0" expression="" field="First Commit Date"/>
    <default applyOnUpdate="0" expression="" field="GIT Nickname"/>
  </defaults>
  <constraints>
    <constraint unique_strength="0" constraints="0" field="Name" notnull_strength="0" exp_strength="0"/>
    <constraint unique_strength="0" constraints="0" field="Committer" notnull_strength="0" exp_strength="0"/>
    <constraint unique_strength="0" constraints="0" field="First Commit Message" notnull_strength="0" exp_strength="0"/>
    <constraint unique_strength="0" constraints="0" field="First Commit Date" notnull_strength="0" exp_strength="0"/>
    <constraint unique_strength="0" constraints="0" field="GIT Nickname" notnull_strength="0" exp_strength="0"/>
  </constraints>
  <constraintExpressions>
    <constraint desc="" field="Name" exp=""/>
    <constraint desc="" field="Committer" exp=""/>
    <constraint desc="" field="First Commit Message" exp=""/>
    <constraint desc="" field="First Commit Date" exp=""/>
    <constraint desc="" field="GIT Nickname" exp=""/>
  </constraintExpressions>
  <attributeactions>
    <defaultAction value="{00000000-0000-0000-0000-000000000000}" key="Canvas"/>
  </attributeactions>
  <attributetableconfig sortOrder="0" actionWidgetStyle="dropDown" sortExpression="">
    <columns>
      <column type="actions" width="-1" hidden="1"/>
      <column type="field" width="-1" hidden="0" name="Name"/>
      <column type="field" width="-1" hidden="0" name="Committer"/>
      <column type="field" width="-1" hidden="0" name="First Commit Message"/>
      <column type="field" width="-1" hidden="0" name="First Commit Date"/>
      <column type="field" width="-1" hidden="0" name="GIT Nickname"/>
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
