<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis styleCategories="AllStyleCategories" minScale="180000" simplifyMaxScale="1" readOnly="0" simplifyDrawingTol="1" labelsEnabled="0" simplifyAlgorithm="0" version="3.15.0-Master" hasScaleBasedVisibilityFlag="1" simplifyLocal="1" maxScale="0" simplifyDrawingHints="1">
  <flags>
    <Identifiable>1</Identifiable>
    <Removable>1</Removable>
    <Searchable>1</Searchable>
  </flags>
  <temporal startExpression="" accumulate="0" enabled="0" durationField="" durationUnit="min" fixedDuration="0" endField="" endExpression="" startField="" mode="0">
    <fixedRange>
      <start></start>
      <end></end>
    </fixedRange>
  </temporal>
  <renderer-v2 symbollevels="0" enableorderby="0" forceraster="0" type="singleSymbol">
    <symbols>
      <symbol force_rhr="0" alpha="1" clip_to_extent="1" type="fill" name="0">
        <layer class="SimpleFill" enabled="1" locked="0" pass="0">
          <prop k="border_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="color" v="183,72,75,255"/>
          <prop k="joinstyle" v="bevel"/>
          <prop k="offset" v="0,0"/>
          <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="offset_unit" v="MM"/>
          <prop k="outline_color" v="35,35,35,255"/>
          <prop k="outline_style" v="solid"/>
          <prop k="outline_width" v="0.26"/>
          <prop k="outline_width_unit" v="MM"/>
          <prop k="style" v="solid"/>
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
  <SingleCategoryDiagramRenderer attributeLegend="1" diagramType="Histogram">
    <DiagramCategory backgroundAlpha="255" spacingUnit="MM" height="15" showAxis="0" direction="1" scaleBasedVisibility="0" labelPlacementMethod="XHeight" enabled="0" minScaleDenominator="0" maxScaleDenominator="1e+08" penColor="#000000" spacingUnitScale="3x:0,0,0,0,0,0" sizeType="MM" width="15" backgroundColor="#ffffff" spacing="0" minimumSize="0" penWidth="0" penAlpha="255" barWidth="5" lineSizeType="MM" opacity="1" sizeScale="3x:0,0,0,0,0,0" lineSizeScale="3x:0,0,0,0,0,0" scaleDependency="Area" diagramOrientation="Up" rotationOffset="270">
      <fontProperties style="Regular" description="Noto Sans,10,-1,5,50,0,0,0,0,0,Regular"/>
      <attribute color="#000000" label="" field=""/>
      <axisSymbol>
        <symbol force_rhr="0" alpha="1" clip_to_extent="1" type="line" name="">
          <layer class="SimpleLine" enabled="1" locked="0" pass="0">
            <prop k="capstyle" v="square"/>
            <prop k="customdash" v="5;2"/>
            <prop k="customdash_map_unit_scale" v="3x:0,0,0,0,0,0"/>
            <prop k="customdash_unit" v="MM"/>
            <prop k="draw_inside_polygon" v="0"/>
            <prop k="joinstyle" v="bevel"/>
            <prop k="line_color" v="35,35,35,255"/>
            <prop k="line_style" v="solid"/>
            <prop k="line_width" v="0.26"/>
            <prop k="line_width_unit" v="MM"/>
            <prop k="offset" v="0"/>
            <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>
            <prop k="offset_unit" v="MM"/>
            <prop k="ring_filter" v="0"/>
            <prop k="use_custom_dash" v="0"/>
            <prop k="width_map_unit_scale" v="3x:0,0,0,0,0,0"/>
            <data_defined_properties>
              <Option type="Map">
                <Option value="" type="QString" name="name"/>
                <Option name="properties"/>
                <Option value="collection" type="QString" name="type"/>
              </Option>
            </data_defined_properties>
          </layer>
        </symbol>
      </axisSymbol>
    </DiagramCategory>
  </SingleCategoryDiagramRenderer>
  <DiagramLayerSettings obstacle="0" dist="0" showAll="1" zIndex="0" placement="1" linePlacementFlags="18" priority="0">
    <properties>
      <Option type="Map">
        <Option value="" type="QString" name="name"/>
        <Option name="properties"/>
        <Option value="collection" type="QString" name="type"/>
      </Option>
    </properties>
  </DiagramLayerSettings>
  <geometryOptions geometryPrecision="0" removeDuplicateNodes="0">
    <activeChecks/>
    <checkConfiguration type="Map">
      <Option type="Map" name="QgsGeometryGapCheck">
        <Option value="0" type="double" name="allowedGapsBuffer"/>
        <Option value="false" type="bool" name="allowedGapsEnabled"/>
        <Option value="" type="QString" name="allowedGapsLayer"/>
      </Option>
    </checkConfiguration>
  </geometryOptions>
  <referencedLayers/>
  <referencingLayers/>
  <fieldConfiguration>
    <field name="fid">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="id">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="typ">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="name">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="ortsrat">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="id_long">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
  </fieldConfiguration>
  <aliases>
    <alias index="0" field="fid" name=""/>
    <alias index="1" field="id" name=""/>
    <alias index="2" field="typ" name=""/>
    <alias index="3" field="name" name=""/>
    <alias index="4" field="ortsrat" name=""/>
    <alias index="5" field="id_long" name=""/>
  </aliases>
  <excludeAttributesWMS/>
  <excludeAttributesWFS/>
  <defaults>
    <default expression="" applyOnUpdate="0" field="fid"/>
    <default expression="" applyOnUpdate="0" field="id"/>
    <default expression="" applyOnUpdate="0" field="typ"/>
    <default expression="" applyOnUpdate="0" field="name"/>
    <default expression="" applyOnUpdate="0" field="ortsrat"/>
    <default expression="" applyOnUpdate="0" field="id_long"/>
  </defaults>
  <constraints>
    <constraint notnull_strength="1" exp_strength="0" constraints="3" field="fid" unique_strength="1"/>
    <constraint notnull_strength="0" exp_strength="0" constraints="0" field="id" unique_strength="0"/>
    <constraint notnull_strength="0" exp_strength="0" constraints="0" field="typ" unique_strength="0"/>
    <constraint notnull_strength="1" exp_strength="0" constraints="1" field="name" unique_strength="0"/>
    <constraint notnull_strength="0" exp_strength="0" constraints="0" field="ortsrat" unique_strength="0"/>
    <constraint notnull_strength="0" exp_strength="0" constraints="0" field="id_long" unique_strength="0"/>
  </constraints>
  <constraintExpressions>
    <constraint desc="" exp="" field="fid"/>
    <constraint desc="" exp="" field="id"/>
    <constraint desc="" exp="" field="typ"/>
    <constraint desc="" exp="" field="name"/>
    <constraint desc="" exp="" field="ortsrat"/>
    <constraint desc="" exp="" field="id_long"/>
  </constraintExpressions>
  <expressionfields/>
  <attributeactions>
    <defaultAction value="{00000000-0000-0000-0000-000000000000}" key="Canvas"/>
  </attributeactions>
  <attributetableconfig actionWidgetStyle="dropDown" sortOrder="0" sortExpression="">
    <columns>
      <column hidden="0" width="-1" type="field" name="fid"/>
      <column hidden="0" width="-1" type="field" name="gid"/>
      <column hidden="0" width="-1" type="field" name="datum"/>
      <column hidden="0" width="-1" type="field" name="bearbeiter"/>
      <column hidden="0" width="-1" type="field" name="veranstaltung"/>
      <column hidden="0" width="-1" type="field" name="beschriftung"/>
      <column hidden="0" width="-1" type="field" name="name"/>
      <column hidden="0" width="-1" type="field" name="flaechentyp"/>
      <column hidden="0" width="-1" type="field" name="farbe"/>
      <column hidden="0" width="-1" type="field" name="schraff_width"/>
      <column hidden="0" width="-1" type="field" name="schraff_width_prt"/>
      <column hidden="0" width="-1" type="field" name="schraff_size"/>
      <column hidden="0" width="-1" type="field" name="schraff_size_prt"/>
      <column hidden="0" width="-1" type="field" name="schraff_winkel"/>
      <column hidden="0" width="-1" type="field" name="umrissfarbe"/>
      <column hidden="0" width="-1" type="field" name="umrisstyp"/>
      <column hidden="0" width="-1" type="field" name="umrissstaerke"/>
      <column hidden="0" width="-1" type="field" name="umrissstaerke_prt"/>
      <column hidden="0" width="-1" type="field" name="umfang"/>
      <column hidden="0" width="-1" type="field" name="flaeche"/>
      <column hidden="0" width="-1" type="field" name="bemerkung"/>
      <column hidden="0" width="-1" type="field" name="last_change"/>
      <column hidden="1" width="-1" type="actions"/>
    </columns>
  </attributetableconfig>
  <conditionalstyles>
    <rowstyles/>
    <fieldstyles/>
  </conditionalstyles>
  <storedexpressions/>
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
    <field editable="1" name="bearbeiter"/>
    <field editable="1" name="bemerkung"/>
    <field editable="1" name="beschriftung"/>
    <field editable="1" name="datum"/>
    <field editable="1" name="farbe"/>
    <field editable="1" name="fid"/>
    <field editable="1" name="flaeche"/>
    <field editable="1" name="flaechentyp"/>
    <field editable="1" name="gid"/>
    <field editable="1" name="last_change"/>
    <field editable="1" name="name"/>
    <field editable="1" name="schraff_size"/>
    <field editable="1" name="schraff_size_prt"/>
    <field editable="1" name="schraff_width"/>
    <field editable="1" name="schraff_width_prt"/>
    <field editable="1" name="schraff_winkel"/>
    <field editable="1" name="umfang"/>
    <field editable="1" name="umrissfarbe"/>
    <field editable="1" name="umrissstaerke"/>
    <field editable="1" name="umrissstaerke_prt"/>
    <field editable="1" name="umrisstyp"/>
    <field editable="1" name="veranstaltung"/>
  </editable>
  <labelOnTop>
    <field labelOnTop="0" name="bearbeiter"/>
    <field labelOnTop="0" name="bemerkung"/>
    <field labelOnTop="0" name="beschriftung"/>
    <field labelOnTop="0" name="datum"/>
    <field labelOnTop="0" name="farbe"/>
    <field labelOnTop="0" name="fid"/>
    <field labelOnTop="0" name="flaeche"/>
    <field labelOnTop="0" name="flaechentyp"/>
    <field labelOnTop="0" name="gid"/>
    <field labelOnTop="0" name="last_change"/>
    <field labelOnTop="0" name="name"/>
    <field labelOnTop="0" name="schraff_size"/>
    <field labelOnTop="0" name="schraff_size_prt"/>
    <field labelOnTop="0" name="schraff_width"/>
    <field labelOnTop="0" name="schraff_width_prt"/>
    <field labelOnTop="0" name="schraff_winkel"/>
    <field labelOnTop="0" name="umfang"/>
    <field labelOnTop="0" name="umrissfarbe"/>
    <field labelOnTop="0" name="umrissstaerke"/>
    <field labelOnTop="0" name="umrissstaerke_prt"/>
    <field labelOnTop="0" name="umrisstyp"/>
    <field labelOnTop="0" name="veranstaltung"/>
  </labelOnTop>
  <dataDefinedFieldProperties/>
  <widgets/>
  <previewExpression>fid</previewExpression>
  <mapTip></mapTip>
  <layerGeometryType>2</layerGeometryType>
</qgis>
