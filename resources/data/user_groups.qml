<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis styleCategories="Symbology|MapTips" version="3.15.0-Master">
  <renderer-v2 type="RuleRenderer" symbollevels="0" enableorderby="0" forceraster="0">
    <rules key="{7c035b8b-7431-4fee-87be-6dc156a20233}">
      <rule filter="ug_name is not NULL" label="User group" key="{49603121-7528-4de3-885b-45a422234c3b}" symbol="0"/>
      <rule filter="ELSE" label="No user group" key="{b7e9bc39-65ae-47ca-af82-6a3d3b0c49be}" symbol="1"/>
    </rules>
    <symbols>
      <symbol type="fill" name="0" alpha="1" clip_to_extent="1" force_rhr="0">
        <layer class="GradientFill" pass="0" enabled="1" locked="0">
          <prop v="0" k="angle"/>
          <prop v="145,175,35,255" k="color"/>
          <prop v="247,252,245,255" k="color1"/>
          <prop v="0,68,27,255" k="color2"/>
          <prop v="0" k="color_type"/>
          <prop v="0" k="coordinate_mode"/>
          <prop v="0" k="discrete"/>
          <prop v="93,152,48,255" k="gradient_color2"/>
          <prop v="0,0" k="offset"/>
          <prop v="3x:0,0,0,0,0,0" k="offset_map_unit_scale"/>
          <prop v="MM" k="offset_unit"/>
          <prop v="gradient" k="rampType"/>
          <prop v="0,0" k="reference_point1"/>
          <prop v="1" k="reference_point1_iscentroid"/>
          <prop v="1,1" k="reference_point2"/>
          <prop v="0" k="reference_point2_iscentroid"/>
          <prop v="0" k="spread"/>
          <prop v="0.13;229,245,224,255:0.26;199,233,192,255:0.39;161,217,155,255:0.52;116,196,118,255:0.65;65,171,93,255:0.78;35,139,69,255:0.9;0,109,44,255" k="stops"/>
          <prop v="1" k="type"/>
          <data_defined_properties>
            <Option type="Map">
              <Option type="QString" name="name" value=""/>
              <Option name="properties"/>
              <Option type="QString" name="type" value="collection"/>
            </Option>
          </data_defined_properties>
        </layer>
        <layer class="SimpleFill" pass="0" enabled="1" locked="0">
          <prop v="3x:0,0,0,0,0,0" k="border_width_map_unit_scale"/>
          <prop v="77,175,74,255" k="color"/>
          <prop v="bevel" k="joinstyle"/>
          <prop v="0,0" k="offset"/>
          <prop v="3x:0,0,0,0,0,0" k="offset_map_unit_scale"/>
          <prop v="MM" k="offset_unit"/>
          <prop v="56,128,54,255" k="outline_color"/>
          <prop v="solid" k="outline_style"/>
          <prop v="0.26" k="outline_width"/>
          <prop v="MM" k="outline_width_unit"/>
          <prop v="no" k="style"/>
          <data_defined_properties>
            <Option type="Map">
              <Option type="QString" name="name" value=""/>
              <Option name="properties"/>
              <Option type="QString" name="type" value="collection"/>
            </Option>
          </data_defined_properties>
        </layer>
      </symbol>
      <symbol type="fill" name="1" alpha="1" clip_to_extent="1" force_rhr="0">
        <layer class="SimpleFill" pass="0" enabled="1" locked="0">
          <prop v="3x:0,0,0,0,0,0" k="border_width_map_unit_scale"/>
          <prop v="224,220,202,154" k="color"/>
          <prop v="bevel" k="joinstyle"/>
          <prop v="0,0" k="offset"/>
          <prop v="3x:0,0,0,0,0,0" k="offset_map_unit_scale"/>
          <prop v="MM" k="offset_unit"/>
          <prop v="119,116,104,154" k="outline_color"/>
          <prop v="solid" k="outline_style"/>
          <prop v="0.26" k="outline_width"/>
          <prop v="MM" k="outline_width_unit"/>
          <prop v="solid" k="style"/>
          <data_defined_properties>
            <Option type="Map">
              <Option type="QString" name="name" value=""/>
              <Option name="properties"/>
              <Option type="QString" name="type" value="collection"/>
            </Option>
          </data_defined_properties>
        </layer>
      </symbol>
    </symbols>
  </renderer-v2>
  <blendMode>0</blendMode>
  <featureBlendMode>0</featureBlendMode>
  <mapTip>&lt;b style="color:#505050">[% "ug_name" %]&lt;/b>&lt;br>
&lt;img style="height:70px" src="[% ug_logo_url %]" alt="(No user group in [% NAME %])">&lt;br>
&lt;a href="[% ug_website %]">[% "ug_website" %]&lt;/a>&lt;br>
&lt;i style="color:gray">[% CASE WHEN "ug_year" IS NOT NULL THEN '(Registered in '|| "ug_year" ||')' ELSE '' END %]&lt;/i></mapTip>
  <layerGeometryType>2</layerGeometryType>
</qgis>
