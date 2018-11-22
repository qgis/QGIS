<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis hasScaleBasedVisibilityFlag="0" simplifyDrawingHints="0" version="2.99.0-Master" simplifyAlgorithm="0" readOnly="0" simplifyDrawingTol="1" simplifyMaxScale="1" maxScale="0" mincale="0" simplifyLocal="1">
  <renderer-v2 forceraster="0" type="singleSymbol" enableorderby="0" symbollevels="0">
    <symbols>
      <symbol name="0" type="marker" alpha="1" clip_to_extent="1">
        <layer locked="0" class="SimpleMarker" pass="0" enabled="1">
          <prop k="angle" v="0"/>
          <prop k="color" v="142,37,144,255"/>
          <prop k="horizontal_anchor_point" v="1"/>
          <prop k="joinstyle" v="bevel"/>
          <prop k="name" v="circle"/>
          <prop k="offset" v="0,0"/>
          <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="offset_unit" v="MM"/>
          <prop k="outline_color" v="0,0,0,255"/>
          <prop k="outline_style" v="solid"/>
          <prop k="outline_width" v="0"/>
          <prop k="outline_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="outline_width_unit" v="MM"/>
          <prop k="scale_method" v="diameter"/>
          <prop k="size" v="2"/>
          <prop k="size_map_unit_scale" v="3x:0,0,0,0,0,0"/>
          <prop k="size_unit" v="MM"/>
          <prop k="vertical_anchor_point" v="1"/>
          <data_defined_properties>
            <Option type="Map">
              <Option name="name" type="QString" value=""/>
              <Option name="properties"/>
              <Option name="type" type="QString" value="collection"/>
            </Option>
          </data_defined_properties>
        </layer>
      </symbol>
    </symbols>
    <rotation/>
    <sizescale/>
  </renderer-v2>
  <labeling type="rule-based">
    <rules key="{ef315774-cd0d-440b-93bf-4aa3d9e01db7}">
      <rule scalemindenom="100000" description="High population" filter="POP_MAX > 1000000" key="{5ce49f58-c8f2-4492-8930-262bae0be058}" scalemaxdenom="10000000">
        <settings>
          <text-style fontSize="10" namedStyle="Regular" fontSizeUnit="Point" fontCapitals="0" fontFamily="Noto Sans" fontSizeMapUnitScale="3x:0,0,0,0,0,0" isExpression="0" blendMode="0" fontWordSpacing="0" previewBkgrdColor="#ffffff" textOpacity="1" textColor="0,0,0,255" fieldName="NAME" fontItalic="0" fontLetterSpacing="0" fontWeight="50" useSubstitutions="0" fontUnderline="0" multilineHeight="1" fontStrikeout="0">
            <text-buffer bufferDraw="0" bufferJoinStyle="128" bufferBlendMode="0" bufferOpacity="1" bufferSizeMapUnitScale="3x:0,0,0,0,0,0" bufferColor="255,255,255,255" bufferNoFill="1" bufferSizeUnits="MM" bufferSize="1"/>
            <background shapeSizeY="0" shapeSizeMapUnitScale="3x:0,0,0,0,0,0" shapeSizeType="0" shapeSizeX="0" shapeOffsetUnit="MM" shapeRadiiMapUnitScale="3x:0,0,0,0,0,0" shapeRotationType="0" shapeRadiiY="0" shapeOffsetMapUnitScale="3x:0,0,0,0,0,0" shapeType="0" shapeBorderColor="128,128,128,255" shapeOffsetY="0" shapeFillColor="255,255,255,255" shapeBorderWidth="0" shapeOffsetX="0" shapeBlendMode="0" shapeSizeUnit="MM" shapeBorderWidthUnit="MM" shapeBorderWidthMapUnitScale="3x:0,0,0,0,0,0" shapeRadiiX="0" shapeOpacity="1" shapeSVGFile="" shapeRadiiUnit="MM" shapeRotation="0" shapeDraw="0" shapeJoinStyle="64"/>
            <shadow shadowOffsetMapUnitScale="3x:0,0,0,0,0,0" shadowOffsetAngle="135" shadowRadiusAlphaOnly="0" shadowScale="100" shadowOffsetGlobal="1" shadowBlendMode="6" shadowOffsetDist="1" shadowOffsetUnit="MM" shadowRadius="0" shadowColor="0,0,0,255" shadowOpacity="0" shadowRadiusUnit="MM" shadowUnder="0" shadowDraw="0" shadowRadiusMapUnitScale="3x:0,0,0,0,0,0"/>
            <substitutions/>
          </text-style>
          <text-format decimals="3" leftDirectionSymbol="&lt;" rightDirectionSymbol=">" addDirectionSymbol="0" reverseDirectionSymbol="0" wrapChar="" formatNumbers="0" plussign="0" multilineAlign="3" placeDirectionSymbol="0"/>
          <placement predefinedPositionOrder="TR,TL,BR,BL,R,L,TSR,BSR" distMapUnitScale="3x:0,0,0,0,0,0" fitInPolygonOnly="0" centroidWhole="0" maxCurvedCharAngleIn="25" centroidInside="0" maxCurvedCharAngleOut="-25" placementFlags="10" repeatDistanceUnits="MM" priority="5" labelOffsetMapUnitScale="3x:0,0,0,0,0,0" offsetUnits="MM" placement="0" repeatDistance="0" distUnits="MM" preserveRotation="1" quadOffset="4" xOffset="0" yOffset="0" offsetType="0" repeatDistanceMapUnitScale="3x:0,0,0,0,0,0" rotationAngle="0" dist="0"/>
          <rendering obstacleType="0" fontLimitPixelSize="0" displayAll="0" minFeatureSize="0" scaleMin="0" limitNumLabels="0" mergeLines="0" scaleVisibility="1" fontMinPixelSize="3" scaleMax="15000000" zIndex="0" drawLabels="1" obstacleFactor="1" maxNumLabels="2000" fontMaxPixelSize="10000" obstacle="1" upsidedownLabels="0" labelPerPart="0"/>
          <dd_properties>
            <Option type="Map">
              <Option name="name" type="QString" value=""/>
              <Option name="properties"/>
              <Option name="type" type="QString" value="collection"/>
            </Option>
          </dd_properties>
        </settings>
      </rule>
      <rule filter="POP_MAX &lt; 1000000" key="{b7e569f5-809c-4376-be66-f55ba754accc}">
        <settings>
          <text-style fontSize="8" namedStyle="Italic" fontSizeUnit="Point" fontCapitals="0" fontFamily="Noto Sans" fontSizeMapUnitScale="3x:0,0,0,0,0,0" isExpression="0" blendMode="0" fontWordSpacing="0" previewBkgrdColor="#ffffff" textOpacity="1" textColor="0,0,0,255" fieldName="NAME" fontItalic="1" fontLetterSpacing="0" fontWeight="50" useSubstitutions="0" fontUnderline="0" multilineHeight="1" fontStrikeout="0">
            <text-buffer bufferDraw="0" bufferJoinStyle="128" bufferBlendMode="0" bufferOpacity="1" bufferSizeMapUnitScale="3x:0,0,0,0,0,0" bufferColor="255,255,255,255" bufferNoFill="1" bufferSizeUnits="MM" bufferSize="1"/>
            <background shapeSizeY="0" shapeSizeMapUnitScale="3x:0,0,0,0,0,0" shapeSizeType="0" shapeSizeX="0" shapeOffsetUnit="MM" shapeRadiiMapUnitScale="3x:0,0,0,0,0,0" shapeRotationType="0" shapeRadiiY="0" shapeOffsetMapUnitScale="3x:0,0,0,0,0,0" shapeType="0" shapeBorderColor="128,128,128,255" shapeOffsetY="0" shapeFillColor="255,255,255,255" shapeBorderWidth="0" shapeOffsetX="0" shapeBlendMode="0" shapeSizeUnit="MM" shapeBorderWidthUnit="MM" shapeBorderWidthMapUnitScale="3x:0,0,0,0,0,0" shapeRadiiX="0" shapeOpacity="1" shapeSVGFile="" shapeRadiiUnit="MM" shapeRotation="0" shapeDraw="0" shapeJoinStyle="64"/>
            <shadow shadowOffsetMapUnitScale="3x:0,0,0,0,0,0" shadowOffsetAngle="135" shadowRadiusAlphaOnly="0" shadowScale="100" shadowOffsetGlobal="1" shadowBlendMode="6" shadowOffsetDist="1" shadowOffsetUnit="MM" shadowRadius="1,5" shadowColor="0,0,0,255" shadowOpacity="0,7" shadowRadiusUnit="MM" shadowUnder="0" shadowDraw="0" shadowRadiusMapUnitScale="3x:0,0,0,0,0,0"/>
            <substitutions/>
          </text-style>
          <text-format decimals="3" leftDirectionSymbol="&lt;" rightDirectionSymbol=">" addDirectionSymbol="0" reverseDirectionSymbol="0" wrapChar="" formatNumbers="0" plussign="0" multilineAlign="3" placeDirectionSymbol="0"/>
          <placement predefinedPositionOrder="TR,TL,BR,BL,R,L,TSR,BSR" distMapUnitScale="3x:0,0,0,0,0,0" fitInPolygonOnly="0" centroidWhole="0" maxCurvedCharAngleIn="25" centroidInside="0" maxCurvedCharAngleOut="-25" placementFlags="10" repeatDistanceUnits="MM" priority="5" labelOffsetMapUnitScale="3x:0,0,0,0,0,0" offsetUnits="MM" placement="0" repeatDistance="0" distUnits="MM" preserveRotation="1" quadOffset="4" xOffset="0" yOffset="0" offsetType="0" repeatDistanceMapUnitScale="3x:0,0,0,0,0,0" rotationAngle="0" dist="0"/>
          <rendering obstacleType="0" fontLimitPixelSize="0" displayAll="0" minFeatureSize="0" scaleMin="0" limitNumLabels="0" mergeLines="0" scaleVisibility="0" fontMinPixelSize="3" scaleMax="0" zIndex="0" drawLabels="1" obstacleFactor="1" maxNumLabels="2000" fontMaxPixelSize="10000" obstacle="1" upsidedownLabels="0" labelPerPart="0"/>
          <dd_properties>
            <Option type="Map">
              <Option name="name" type="QString" value=""/>
              <Option name="properties"/>
              <Option name="type" type="QString" value="collection"/>
            </Option>
          </dd_properties>
        </settings>
      </rule>
    </rules>
  </labeling>
  <customproperties>
    <property key="dualview/previewExpressions" value="NAME"/>
    <property key="embeddedWidgets/count" value="0"/>
    <property key="variableNames"/>
    <property key="variableValues"/>
  </customproperties>
  <blendMode>0</blendMode>
  <featureBlendMode>0</featureBlendMode>
  <layerOpacity>1</layerOpacity>
  <SingleCategoryDiagramRenderer attributeLegend="1" diagramType="Histogram">
    <DiagramCategory penWidth="0" scaleBasedVisibility="0" sizeScale="3x:0,0,0,0,0,0" height="15" backgroundAlpha="255" barWidth="5" rotationOffset="270" width="15" opacity="1" scaleDependency="Area" minimumSize="0" labelPlacementMethod="XHeight" minScaleDenominator="0" enabled="0" sizeType="MM" maxScaleDenominator="1e+08" lineSizeScale="3x:0,0,0,0,0,0" lineSizeType="MM" penAlpha="255" backgroundColor="#ffffff" penColor="#000000" diagramOrientation="Up">
      <fontProperties description="Noto Sans,9,-1,5,50,0,0,0,0,0" style=""/>
      <attribute field="" label="" color="#000000"/>
    </DiagramCategory>
  </SingleCategoryDiagramRenderer>
  <DiagramLayerSettings obstacle="0" showAll="1" zIndex="0" linePlacementFlags="18" priority="0" dist="0" placement="0">
    <properties>
      <Option type="Map">
        <Option name="name" type="QString" value=""/>
        <Option name="properties"/>
        <Option name="type" type="QString" value="collection"/>
      </Option>
    </properties>
  </DiagramLayerSettings>
  <fieldConfiguration>
    <field name="SCALERANK">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="NATSCALE">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="LABELRANK">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="FEATURECLA">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="NAME">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="NAMEPAR">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="NAMEALT">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="DIFFASCII">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="NAMEASCII">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="ADM0CAP">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="CAPALT">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="CAPIN">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="WORLDCITY">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MEGACITY">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="SOV0NAME">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="SOV_A3">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="ADM0NAME">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="ADM0_A3">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="ADM1NAME">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="ISO_A2">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="NOTE">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="LATITUDE">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="LONGITUDE">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="CHANGED">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="NAMEDIFF">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="DIFFNOTE">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP_MAX">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP_MIN">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP_OTHER">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="RANK_MAX">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="RANK_MIN">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="GEONAMEID">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MEGANAME">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="LS_NAME">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="LS_MATCH">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="CHECKME">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_POP10">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_POP20">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_POP50">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_POP300">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_POP310">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_NATSCA">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MIN_AREAKM">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_AREAKM">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MIN_AREAMI">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_AREAMI">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MIN_PERKM">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_PERKM">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MIN_PERMI">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_PERMI">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MIN_BBXMIN">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_BBXMIN">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MIN_BBXMAX">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_BBXMAX">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MIN_BBYMIN">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_BBYMIN">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MIN_BBYMAX">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MAX_BBYMAX">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MEAN_BBXC">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="MEAN_BBYC">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="COMPARE">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="GN_ASCII">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="FEATURE_CL">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="FEATURE_CO">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="ADMIN1_COD">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="GN_POP">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="ELEVATION">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="GTOPO30">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="TIMEZONE">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="GEONAMESNO">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="UN_FID">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="UN_ADM0">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="UN_LAT">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="UN_LONG">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP1950">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP1955">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP1960">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP1965">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP1970">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP1975">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP1980">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP1985">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP1990">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP1995">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP2000">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP2005">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP2010">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP2015">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP2020">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP2025">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="POP2050">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="CITYALT">
      <editWidget type="TextEdit">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
  </fieldConfiguration>
  <aliases>
    <alias name="" field="SCALERANK" index="0"/>
    <alias name="" field="NATSCALE" index="1"/>
    <alias name="" field="LABELRANK" index="2"/>
    <alias name="" field="FEATURECLA" index="3"/>
    <alias name="" field="NAME" index="4"/>
    <alias name="" field="NAMEPAR" index="5"/>
    <alias name="" field="NAMEALT" index="6"/>
    <alias name="" field="DIFFASCII" index="7"/>
    <alias name="" field="NAMEASCII" index="8"/>
    <alias name="" field="ADM0CAP" index="9"/>
    <alias name="" field="CAPALT" index="10"/>
    <alias name="" field="CAPIN" index="11"/>
    <alias name="" field="WORLDCITY" index="12"/>
    <alias name="" field="MEGACITY" index="13"/>
    <alias name="" field="SOV0NAME" index="14"/>
    <alias name="" field="SOV_A3" index="15"/>
    <alias name="" field="ADM0NAME" index="16"/>
    <alias name="" field="ADM0_A3" index="17"/>
    <alias name="" field="ADM1NAME" index="18"/>
    <alias name="" field="ISO_A2" index="19"/>
    <alias name="" field="NOTE" index="20"/>
    <alias name="" field="LATITUDE" index="21"/>
    <alias name="" field="LONGITUDE" index="22"/>
    <alias name="" field="CHANGED" index="23"/>
    <alias name="" field="NAMEDIFF" index="24"/>
    <alias name="" field="DIFFNOTE" index="25"/>
    <alias name="" field="POP_MAX" index="26"/>
    <alias name="" field="POP_MIN" index="27"/>
    <alias name="" field="POP_OTHER" index="28"/>
    <alias name="" field="RANK_MAX" index="29"/>
    <alias name="" field="RANK_MIN" index="30"/>
    <alias name="" field="GEONAMEID" index="31"/>
    <alias name="" field="MEGANAME" index="32"/>
    <alias name="" field="LS_NAME" index="33"/>
    <alias name="" field="LS_MATCH" index="34"/>
    <alias name="" field="CHECKME" index="35"/>
    <alias name="" field="MAX_POP10" index="36"/>
    <alias name="" field="MAX_POP20" index="37"/>
    <alias name="" field="MAX_POP50" index="38"/>
    <alias name="" field="MAX_POP300" index="39"/>
    <alias name="" field="MAX_POP310" index="40"/>
    <alias name="" field="MAX_NATSCA" index="41"/>
    <alias name="" field="MIN_AREAKM" index="42"/>
    <alias name="" field="MAX_AREAKM" index="43"/>
    <alias name="" field="MIN_AREAMI" index="44"/>
    <alias name="" field="MAX_AREAMI" index="45"/>
    <alias name="" field="MIN_PERKM" index="46"/>
    <alias name="" field="MAX_PERKM" index="47"/>
    <alias name="" field="MIN_PERMI" index="48"/>
    <alias name="" field="MAX_PERMI" index="49"/>
    <alias name="" field="MIN_BBXMIN" index="50"/>
    <alias name="" field="MAX_BBXMIN" index="51"/>
    <alias name="" field="MIN_BBXMAX" index="52"/>
    <alias name="" field="MAX_BBXMAX" index="53"/>
    <alias name="" field="MIN_BBYMIN" index="54"/>
    <alias name="" field="MAX_BBYMIN" index="55"/>
    <alias name="" field="MIN_BBYMAX" index="56"/>
    <alias name="" field="MAX_BBYMAX" index="57"/>
    <alias name="" field="MEAN_BBXC" index="58"/>
    <alias name="" field="MEAN_BBYC" index="59"/>
    <alias name="" field="COMPARE" index="60"/>
    <alias name="" field="GN_ASCII" index="61"/>
    <alias name="" field="FEATURE_CL" index="62"/>
    <alias name="" field="FEATURE_CO" index="63"/>
    <alias name="" field="ADMIN1_COD" index="64"/>
    <alias name="" field="GN_POP" index="65"/>
    <alias name="" field="ELEVATION" index="66"/>
    <alias name="" field="GTOPO30" index="67"/>
    <alias name="" field="TIMEZONE" index="68"/>
    <alias name="" field="GEONAMESNO" index="69"/>
    <alias name="" field="UN_FID" index="70"/>
    <alias name="" field="UN_ADM0" index="71"/>
    <alias name="" field="UN_LAT" index="72"/>
    <alias name="" field="UN_LONG" index="73"/>
    <alias name="" field="POP1950" index="74"/>
    <alias name="" field="POP1955" index="75"/>
    <alias name="" field="POP1960" index="76"/>
    <alias name="" field="POP1965" index="77"/>
    <alias name="" field="POP1970" index="78"/>
    <alias name="" field="POP1975" index="79"/>
    <alias name="" field="POP1980" index="80"/>
    <alias name="" field="POP1985" index="81"/>
    <alias name="" field="POP1990" index="82"/>
    <alias name="" field="POP1995" index="83"/>
    <alias name="" field="POP2000" index="84"/>
    <alias name="" field="POP2005" index="85"/>
    <alias name="" field="POP2010" index="86"/>
    <alias name="" field="POP2015" index="87"/>
    <alias name="" field="POP2020" index="88"/>
    <alias name="" field="POP2025" index="89"/>
    <alias name="" field="POP2050" index="90"/>
    <alias name="" field="CITYALT" index="91"/>
  </aliases>
  <excludeAttributesWMS/>
  <excludeAttributesWFS/>
  <defaults>
    <default field="SCALERANK" expression=""/>
    <default field="NATSCALE" expression=""/>
    <default field="LABELRANK" expression=""/>
    <default field="FEATURECLA" expression=""/>
    <default field="NAME" expression=""/>
    <default field="NAMEPAR" expression=""/>
    <default field="NAMEALT" expression=""/>
    <default field="DIFFASCII" expression=""/>
    <default field="NAMEASCII" expression=""/>
    <default field="ADM0CAP" expression=""/>
    <default field="CAPALT" expression=""/>
    <default field="CAPIN" expression=""/>
    <default field="WORLDCITY" expression=""/>
    <default field="MEGACITY" expression=""/>
    <default field="SOV0NAME" expression=""/>
    <default field="SOV_A3" expression=""/>
    <default field="ADM0NAME" expression=""/>
    <default field="ADM0_A3" expression=""/>
    <default field="ADM1NAME" expression=""/>
    <default field="ISO_A2" expression=""/>
    <default field="NOTE" expression=""/>
    <default field="LATITUDE" expression=""/>
    <default field="LONGITUDE" expression=""/>
    <default field="CHANGED" expression=""/>
    <default field="NAMEDIFF" expression=""/>
    <default field="DIFFNOTE" expression=""/>
    <default field="POP_MAX" expression=""/>
    <default field="POP_MIN" expression=""/>
    <default field="POP_OTHER" expression=""/>
    <default field="RANK_MAX" expression=""/>
    <default field="RANK_MIN" expression=""/>
    <default field="GEONAMEID" expression=""/>
    <default field="MEGANAME" expression=""/>
    <default field="LS_NAME" expression=""/>
    <default field="LS_MATCH" expression=""/>
    <default field="CHECKME" expression=""/>
    <default field="MAX_POP10" expression=""/>
    <default field="MAX_POP20" expression=""/>
    <default field="MAX_POP50" expression=""/>
    <default field="MAX_POP300" expression=""/>
    <default field="MAX_POP310" expression=""/>
    <default field="MAX_NATSCA" expression=""/>
    <default field="MIN_AREAKM" expression=""/>
    <default field="MAX_AREAKM" expression=""/>
    <default field="MIN_AREAMI" expression=""/>
    <default field="MAX_AREAMI" expression=""/>
    <default field="MIN_PERKM" expression=""/>
    <default field="MAX_PERKM" expression=""/>
    <default field="MIN_PERMI" expression=""/>
    <default field="MAX_PERMI" expression=""/>
    <default field="MIN_BBXMIN" expression=""/>
    <default field="MAX_BBXMIN" expression=""/>
    <default field="MIN_BBXMAX" expression=""/>
    <default field="MAX_BBXMAX" expression=""/>
    <default field="MIN_BBYMIN" expression=""/>
    <default field="MAX_BBYMIN" expression=""/>
    <default field="MIN_BBYMAX" expression=""/>
    <default field="MAX_BBYMAX" expression=""/>
    <default field="MEAN_BBXC" expression=""/>
    <default field="MEAN_BBYC" expression=""/>
    <default field="COMPARE" expression=""/>
    <default field="GN_ASCII" expression=""/>
    <default field="FEATURE_CL" expression=""/>
    <default field="FEATURE_CO" expression=""/>
    <default field="ADMIN1_COD" expression=""/>
    <default field="GN_POP" expression=""/>
    <default field="ELEVATION" expression=""/>
    <default field="GTOPO30" expression=""/>
    <default field="TIMEZONE" expression=""/>
    <default field="GEONAMESNO" expression=""/>
    <default field="UN_FID" expression=""/>
    <default field="UN_ADM0" expression=""/>
    <default field="UN_LAT" expression=""/>
    <default field="UN_LONG" expression=""/>
    <default field="POP1950" expression=""/>
    <default field="POP1955" expression=""/>
    <default field="POP1960" expression=""/>
    <default field="POP1965" expression=""/>
    <default field="POP1970" expression=""/>
    <default field="POP1975" expression=""/>
    <default field="POP1980" expression=""/>
    <default field="POP1985" expression=""/>
    <default field="POP1990" expression=""/>
    <default field="POP1995" expression=""/>
    <default field="POP2000" expression=""/>
    <default field="POP2005" expression=""/>
    <default field="POP2010" expression=""/>
    <default field="POP2015" expression=""/>
    <default field="POP2020" expression=""/>
    <default field="POP2025" expression=""/>
    <default field="POP2050" expression=""/>
    <default field="CITYALT" expression=""/>
  </defaults>
  <constraints>
    <constraint field="SCALERANK" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="NATSCALE" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="LABELRANK" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="FEATURECLA" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="NAME" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="NAMEPAR" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="NAMEALT" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="DIFFASCII" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="NAMEASCII" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="ADM0CAP" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="CAPALT" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="CAPIN" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="WORLDCITY" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MEGACITY" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="SOV0NAME" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="SOV_A3" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="ADM0NAME" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="ADM0_A3" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="ADM1NAME" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="ISO_A2" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="NOTE" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="LATITUDE" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="LONGITUDE" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="CHANGED" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="NAMEDIFF" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="DIFFNOTE" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP_MAX" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP_MIN" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP_OTHER" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="RANK_MAX" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="RANK_MIN" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="GEONAMEID" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MEGANAME" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="LS_NAME" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="LS_MATCH" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="CHECKME" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_POP10" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_POP20" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_POP50" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_POP300" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_POP310" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_NATSCA" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MIN_AREAKM" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_AREAKM" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MIN_AREAMI" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_AREAMI" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MIN_PERKM" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_PERKM" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MIN_PERMI" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_PERMI" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MIN_BBXMIN" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_BBXMIN" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MIN_BBXMAX" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_BBXMAX" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MIN_BBYMIN" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_BBYMIN" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MIN_BBYMAX" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MAX_BBYMAX" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MEAN_BBXC" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="MEAN_BBYC" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="COMPARE" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="GN_ASCII" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="FEATURE_CL" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="FEATURE_CO" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="ADMIN1_COD" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="GN_POP" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="ELEVATION" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="GTOPO30" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="TIMEZONE" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="GEONAMESNO" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="UN_FID" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="UN_ADM0" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="UN_LAT" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="UN_LONG" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP1950" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP1955" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP1960" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP1965" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP1970" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP1975" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP1980" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP1985" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP1990" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP1995" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP2000" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP2005" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP2010" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP2015" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP2020" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP2025" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="POP2050" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
    <constraint field="CITYALT" constraints="0" exp_strength="0" unique_strength="0" notnull_strength="0"/>
  </constraints>
  <constraintExpressions>
    <constraint field="SCALERANK" exp="" desc=""/>
    <constraint field="NATSCALE" exp="" desc=""/>
    <constraint field="LABELRANK" exp="" desc=""/>
    <constraint field="FEATURECLA" exp="" desc=""/>
    <constraint field="NAME" exp="" desc=""/>
    <constraint field="NAMEPAR" exp="" desc=""/>
    <constraint field="NAMEALT" exp="" desc=""/>
    <constraint field="DIFFASCII" exp="" desc=""/>
    <constraint field="NAMEASCII" exp="" desc=""/>
    <constraint field="ADM0CAP" exp="" desc=""/>
    <constraint field="CAPALT" exp="" desc=""/>
    <constraint field="CAPIN" exp="" desc=""/>
    <constraint field="WORLDCITY" exp="" desc=""/>
    <constraint field="MEGACITY" exp="" desc=""/>
    <constraint field="SOV0NAME" exp="" desc=""/>
    <constraint field="SOV_A3" exp="" desc=""/>
    <constraint field="ADM0NAME" exp="" desc=""/>
    <constraint field="ADM0_A3" exp="" desc=""/>
    <constraint field="ADM1NAME" exp="" desc=""/>
    <constraint field="ISO_A2" exp="" desc=""/>
    <constraint field="NOTE" exp="" desc=""/>
    <constraint field="LATITUDE" exp="" desc=""/>
    <constraint field="LONGITUDE" exp="" desc=""/>
    <constraint field="CHANGED" exp="" desc=""/>
    <constraint field="NAMEDIFF" exp="" desc=""/>
    <constraint field="DIFFNOTE" exp="" desc=""/>
    <constraint field="POP_MAX" exp="" desc=""/>
    <constraint field="POP_MIN" exp="" desc=""/>
    <constraint field="POP_OTHER" exp="" desc=""/>
    <constraint field="RANK_MAX" exp="" desc=""/>
    <constraint field="RANK_MIN" exp="" desc=""/>
    <constraint field="GEONAMEID" exp="" desc=""/>
    <constraint field="MEGANAME" exp="" desc=""/>
    <constraint field="LS_NAME" exp="" desc=""/>
    <constraint field="LS_MATCH" exp="" desc=""/>
    <constraint field="CHECKME" exp="" desc=""/>
    <constraint field="MAX_POP10" exp="" desc=""/>
    <constraint field="MAX_POP20" exp="" desc=""/>
    <constraint field="MAX_POP50" exp="" desc=""/>
    <constraint field="MAX_POP300" exp="" desc=""/>
    <constraint field="MAX_POP310" exp="" desc=""/>
    <constraint field="MAX_NATSCA" exp="" desc=""/>
    <constraint field="MIN_AREAKM" exp="" desc=""/>
    <constraint field="MAX_AREAKM" exp="" desc=""/>
    <constraint field="MIN_AREAMI" exp="" desc=""/>
    <constraint field="MAX_AREAMI" exp="" desc=""/>
    <constraint field="MIN_PERKM" exp="" desc=""/>
    <constraint field="MAX_PERKM" exp="" desc=""/>
    <constraint field="MIN_PERMI" exp="" desc=""/>
    <constraint field="MAX_PERMI" exp="" desc=""/>
    <constraint field="MIN_BBXMIN" exp="" desc=""/>
    <constraint field="MAX_BBXMIN" exp="" desc=""/>
    <constraint field="MIN_BBXMAX" exp="" desc=""/>
    <constraint field="MAX_BBXMAX" exp="" desc=""/>
    <constraint field="MIN_BBYMIN" exp="" desc=""/>
    <constraint field="MAX_BBYMIN" exp="" desc=""/>
    <constraint field="MIN_BBYMAX" exp="" desc=""/>
    <constraint field="MAX_BBYMAX" exp="" desc=""/>
    <constraint field="MEAN_BBXC" exp="" desc=""/>
    <constraint field="MEAN_BBYC" exp="" desc=""/>
    <constraint field="COMPARE" exp="" desc=""/>
    <constraint field="GN_ASCII" exp="" desc=""/>
    <constraint field="FEATURE_CL" exp="" desc=""/>
    <constraint field="FEATURE_CO" exp="" desc=""/>
    <constraint field="ADMIN1_COD" exp="" desc=""/>
    <constraint field="GN_POP" exp="" desc=""/>
    <constraint field="ELEVATION" exp="" desc=""/>
    <constraint field="GTOPO30" exp="" desc=""/>
    <constraint field="TIMEZONE" exp="" desc=""/>
    <constraint field="GEONAMESNO" exp="" desc=""/>
    <constraint field="UN_FID" exp="" desc=""/>
    <constraint field="UN_ADM0" exp="" desc=""/>
    <constraint field="UN_LAT" exp="" desc=""/>
    <constraint field="UN_LONG" exp="" desc=""/>
    <constraint field="POP1950" exp="" desc=""/>
    <constraint field="POP1955" exp="" desc=""/>
    <constraint field="POP1960" exp="" desc=""/>
    <constraint field="POP1965" exp="" desc=""/>
    <constraint field="POP1970" exp="" desc=""/>
    <constraint field="POP1975" exp="" desc=""/>
    <constraint field="POP1980" exp="" desc=""/>
    <constraint field="POP1985" exp="" desc=""/>
    <constraint field="POP1990" exp="" desc=""/>
    <constraint field="POP1995" exp="" desc=""/>
    <constraint field="POP2000" exp="" desc=""/>
    <constraint field="POP2005" exp="" desc=""/>
    <constraint field="POP2010" exp="" desc=""/>
    <constraint field="POP2015" exp="" desc=""/>
    <constraint field="POP2020" exp="" desc=""/>
    <constraint field="POP2025" exp="" desc=""/>
    <constraint field="POP2050" exp="" desc=""/>
    <constraint field="CITYALT" exp="" desc=""/>
  </constraintExpressions>
  <attributeactions>
    <defaultAction key="Canvas" value="{00000000-0000-0000-0000-000000000000}"/>
  </attributeactions>
  <attributetableconfig sortOrder="0" actionWidgetStyle="dropDown" sortExpression="&quot;SCALERANK&quot;">
    <columns>
      <column name="SCALERANK" hidden="0" type="field" width="-1"/>
      <column name="NATSCALE" hidden="0" type="field" width="-1"/>
      <column name="LABELRANK" hidden="0" type="field" width="-1"/>
      <column name="FEATURECLA" hidden="0" type="field" width="-1"/>
      <column name="NAME" hidden="0" type="field" width="-1"/>
      <column name="NAMEPAR" hidden="0" type="field" width="-1"/>
      <column name="NAMEALT" hidden="0" type="field" width="-1"/>
      <column name="DIFFASCII" hidden="0" type="field" width="-1"/>
      <column name="NAMEASCII" hidden="0" type="field" width="-1"/>
      <column name="ADM0CAP" hidden="0" type="field" width="-1"/>
      <column name="CAPALT" hidden="0" type="field" width="-1"/>
      <column name="CAPIN" hidden="0" type="field" width="-1"/>
      <column name="WORLDCITY" hidden="0" type="field" width="-1"/>
      <column name="MEGACITY" hidden="0" type="field" width="-1"/>
      <column name="SOV0NAME" hidden="0" type="field" width="-1"/>
      <column name="SOV_A3" hidden="0" type="field" width="-1"/>
      <column name="ADM0NAME" hidden="0" type="field" width="-1"/>
      <column name="ADM0_A3" hidden="0" type="field" width="-1"/>
      <column name="ADM1NAME" hidden="0" type="field" width="-1"/>
      <column name="ISO_A2" hidden="0" type="field" width="-1"/>
      <column name="NOTE" hidden="0" type="field" width="-1"/>
      <column name="LATITUDE" hidden="0" type="field" width="-1"/>
      <column name="LONGITUDE" hidden="0" type="field" width="-1"/>
      <column name="CHANGED" hidden="0" type="field" width="-1"/>
      <column name="NAMEDIFF" hidden="0" type="field" width="-1"/>
      <column name="DIFFNOTE" hidden="0" type="field" width="-1"/>
      <column name="POP_MAX" hidden="0" type="field" width="-1"/>
      <column name="POP_MIN" hidden="0" type="field" width="-1"/>
      <column name="POP_OTHER" hidden="0" type="field" width="-1"/>
      <column name="RANK_MAX" hidden="0" type="field" width="-1"/>
      <column name="RANK_MIN" hidden="0" type="field" width="-1"/>
      <column name="GEONAMEID" hidden="0" type="field" width="-1"/>
      <column name="MEGANAME" hidden="0" type="field" width="-1"/>
      <column name="LS_NAME" hidden="0" type="field" width="-1"/>
      <column name="LS_MATCH" hidden="0" type="field" width="-1"/>
      <column name="CHECKME" hidden="0" type="field" width="-1"/>
      <column name="MAX_POP10" hidden="0" type="field" width="-1"/>
      <column name="MAX_POP20" hidden="0" type="field" width="-1"/>
      <column name="MAX_POP50" hidden="0" type="field" width="-1"/>
      <column name="MAX_POP300" hidden="0" type="field" width="-1"/>
      <column name="MAX_POP310" hidden="0" type="field" width="-1"/>
      <column name="MAX_NATSCA" hidden="0" type="field" width="-1"/>
      <column name="MIN_AREAKM" hidden="0" type="field" width="-1"/>
      <column name="MAX_AREAKM" hidden="0" type="field" width="-1"/>
      <column name="MIN_AREAMI" hidden="0" type="field" width="-1"/>
      <column name="MAX_AREAMI" hidden="0" type="field" width="-1"/>
      <column name="MIN_PERKM" hidden="0" type="field" width="-1"/>
      <column name="MAX_PERKM" hidden="0" type="field" width="-1"/>
      <column name="MIN_PERMI" hidden="0" type="field" width="-1"/>
      <column name="MAX_PERMI" hidden="0" type="field" width="-1"/>
      <column name="MIN_BBXMIN" hidden="0" type="field" width="-1"/>
      <column name="MAX_BBXMIN" hidden="0" type="field" width="-1"/>
      <column name="MIN_BBXMAX" hidden="0" type="field" width="-1"/>
      <column name="MAX_BBXMAX" hidden="0" type="field" width="-1"/>
      <column name="MIN_BBYMIN" hidden="0" type="field" width="-1"/>
      <column name="MAX_BBYMIN" hidden="0" type="field" width="-1"/>
      <column name="MIN_BBYMAX" hidden="0" type="field" width="-1"/>
      <column name="MAX_BBYMAX" hidden="0" type="field" width="-1"/>
      <column name="MEAN_BBXC" hidden="0" type="field" width="-1"/>
      <column name="MEAN_BBYC" hidden="0" type="field" width="-1"/>
      <column name="COMPARE" hidden="0" type="field" width="-1"/>
      <column name="GN_ASCII" hidden="0" type="field" width="-1"/>
      <column name="FEATURE_CL" hidden="0" type="field" width="-1"/>
      <column name="FEATURE_CO" hidden="0" type="field" width="-1"/>
      <column name="ADMIN1_COD" hidden="0" type="field" width="-1"/>
      <column name="GN_POP" hidden="0" type="field" width="-1"/>
      <column name="ELEVATION" hidden="0" type="field" width="-1"/>
      <column name="GTOPO30" hidden="0" type="field" width="-1"/>
      <column name="TIMEZONE" hidden="0" type="field" width="-1"/>
      <column name="GEONAMESNO" hidden="0" type="field" width="-1"/>
      <column name="UN_FID" hidden="0" type="field" width="-1"/>
      <column name="UN_ADM0" hidden="0" type="field" width="-1"/>
      <column name="UN_LAT" hidden="0" type="field" width="-1"/>
      <column name="UN_LONG" hidden="0" type="field" width="-1"/>
      <column name="POP1950" hidden="0" type="field" width="-1"/>
      <column name="POP1955" hidden="0" type="field" width="-1"/>
      <column name="POP1960" hidden="0" type="field" width="-1"/>
      <column name="POP1965" hidden="0" type="field" width="-1"/>
      <column name="POP1970" hidden="0" type="field" width="-1"/>
      <column name="POP1975" hidden="0" type="field" width="-1"/>
      <column name="POP1980" hidden="0" type="field" width="-1"/>
      <column name="POP1985" hidden="0" type="field" width="-1"/>
      <column name="POP1990" hidden="0" type="field" width="-1"/>
      <column name="POP1995" hidden="0" type="field" width="-1"/>
      <column name="POP2000" hidden="0" type="field" width="-1"/>
      <column name="POP2005" hidden="0" type="field" width="-1"/>
      <column name="POP2010" hidden="0" type="field" width="-1"/>
      <column name="POP2015" hidden="0" type="field" width="-1"/>
      <column name="POP2020" hidden="0" type="field" width="-1"/>
      <column name="POP2025" hidden="0" type="field" width="-1"/>
      <column name="POP2050" hidden="0" type="field" width="-1"/>
      <column name="CITYALT" hidden="0" type="field" width="-1"/>
      <column hidden="1" type="actions" width="-1"/>
    </columns>
  </attributetableconfig>
  <editform></editform>
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
  <widgets/>
  <conditionalstyles>
    <rowstyles/>
    <fieldstyles/>
  </conditionalstyles>
  <expressionfields/>
  <previewExpression>NAME</previewExpression>
  <mapTip></mapTip>
  <layerGeometryType>0</layerGeometryType>
</qgis>
