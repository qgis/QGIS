<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis version="0.9.2-Ganymede" >
  <maplayer minScale="1" maxScale="1e+08" scaleBasedVisibilityFlag="0" geometry="Point" type="vector" >
    <id>points20080103150949100</id>
    <datasource>/Users/timlinux/dev/cpp/qgis_qml/tests/testdata/points.shp</datasource>
    <layername>points</layername>
    <srs>
      <spatialrefsys>
        <proj4>+proj=longlat +ellps=WGS84 +no_defs</proj4>
        <srsid>1449</srsid>
        <srid>4031</srid>
        <epsg>4031</epsg>
        <description>Unknown datum based upon the GEM 10C ellipsoid</description>
        <projectionacronym>longlat</projectionacronym>
        <ellipsoidacronym>WGS84</ellipsoidacronym>
        <geographicflag>true</geographicflag>
      </spatialrefsys>
    </srs>
    <transparencyLevelInt>255</transparencyLevelInt>
    <provider>ogr</provider>
    <encoding>System</encoding>
    <classificationattribute>Value</classificationattribute>
    <displayfield>Name</displayfield>
    <label>0</label>
    <attributeactions/>
    <continuoussymbol>
      <classificationfield>1</classificationfield>
      <polygonoutline>1</polygonoutline>
      <lowestsymbol>
        <symbol>
          <lowervalue>12.000000</lowervalue>
          <uppervalue></uppervalue>
          <label></label>
          <pointsymbol>hard:circle</pointsymbol>
          <pointsize>6</pointsize>
          <outlinecolor red="255" blue="204" green="216" />
          <outlinestyle>SolidLine</outlinestyle>
          <outlinewidth>2</outlinewidth>
          <fillcolor red="0" blue="0" green="0" />
          <fillpattern>NoBrush</fillpattern>
          <texturepath></texturepath>
        </symbol>
      </lowestsymbol>
      <highestsymbol>
        <symbol>
          <lowervalue>233.000000</lowervalue>
          <uppervalue></uppervalue>
          <label></label>
          <pointsymbol>hard:circle</pointsymbol>
          <pointsize>6</pointsize>
          <outlinecolor red="196" blue="0" green="56" />
          <outlinestyle>SolidLine</outlinestyle>
          <outlinewidth>2</outlinewidth>
          <fillcolor red="0" blue="0" green="0" />
          <fillpattern>NoBrush</fillpattern>
          <texturepath></texturepath>
        </symbol>
      </highestsymbol>
    </continuoussymbol>
    <labelattributes>
      <label field="" text="Label" />
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
