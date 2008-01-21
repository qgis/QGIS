<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis version="0.9.2-Ganymede" >
  <maplayer minScale="1" maxScale="1e+08" scaleBasedVisibilityFlag="0" geometry="Polygon" type="vector" >
    <id>maprender_testdata20080120093911087</id>
    <datasource>/tmp/maprender_testdata.shp</datasource>
    <layername>maprender_testdata</layername>
    <srs>
      <spatialrefsys>
        <proj4>+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs</proj4>
        <srsid>2585</srsid>
        <srid>4326</srid>
        <epsg>4326</epsg>
        <description>WGS 84</description>
        <projectionacronym>longlat</projectionacronym>
        <ellipsoidacronym>WGS84</ellipsoidacronym>
        <geographicflag>true</geographicflag>
      </spatialrefsys>
    </srs>
    <transparencyLevelInt>255</transparencyLevelInt>
    <provider>ogr</provider>
    <encoding>System</encoding>
    <classificationattribute>Value</classificationattribute>
    <displayfield>Value</displayfield>
    <label>0</label>
    <attributeactions/>
    <continuoussymbol>
      <classificationfield>0</classificationfield>
      <polygonoutline>1</polygonoutline>
      <lowestsymbol>
        <symbol>
          <lowervalue>-180.000000</lowervalue>
          <uppervalue></uppervalue>
          <label></label>
          <pointsymbol>hard:circle</pointsymbol>
          <pointsize>10</pointsize>
          <rotationclassificationfield>-1</rotationclassificationfield>
          <scaleclassificationfield>-1</scaleclassificationfield>
          <outlinecolor red="0" blue="0" green="0" />
          <outlinestyle>SolidLine</outlinestyle>
          <outlinewidth>0</outlinewidth>
          <fillcolor red="221" blue="77" green="218" />
          <fillpattern>SolidPattern</fillpattern>
          <texturepath></texturepath>
        </symbol>
      </lowestsymbol>
      <highestsymbol>
        <symbol>
          <lowervalue>180.000000</lowervalue>
          <uppervalue></uppervalue>
          <label></label>
          <pointsymbol>hard:circle</pointsymbol>
          <pointsize>10</pointsize>
          <rotationclassificationfield>-1</rotationclassificationfield>
          <scaleclassificationfield>-1</scaleclassificationfield>
          <outlinecolor red="0" blue="0" green="0" />
          <outlinestyle>SolidLine</outlinestyle>
          <outlinewidth>0</outlinewidth>
          <fillcolor red="187" blue="20" green="27" />
          <fillpattern>SolidPattern</fillpattern>
          <texturepath></texturepath>
        </symbol>
      </highestsymbol>
    </continuoussymbol>
    <labelattributes>
      <label field="" text="Rótulo" />
      <family field="" name="Lucida Grande" />
      <size field="" units="pt" value="12" />
      <bold field="" on="0" />
      <italic field="" on="0" />
      <underline field="" on="0" />
      <color field="" red="0" blue="0" green="0" />
      <x field="" />
      <y field="" />
      <offset x="0" y="0" yfield="-1" xfield="-1" units="pt" />
      <angle field="" value="0" />
      <alignment field="-1" value="center" />
      <buffercolor field="" red="255" blue="255" green="255" />
      <buffersize field="" units="pt" value="1" />
      <bufferenabled field="" on="" />
    </labelattributes>
  </maplayer>
</qgis>
