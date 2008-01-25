<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis projectname="" version="0.9.2-Ganymede" >
        <maplayer minScale="1" maxScale="1e+08" scaleBasedVisibilityFlag="0" type="raster" >
            <id>landsat20080124145540028</id>
            <datasource>landsat.tif</datasource>
            <layername>landsat</layername>
            <srs>
                <spatialrefsys>
                    <proj4>+proj=utm +zone=33 +ellps=WGS84 +datum=WGS84 +units=m +no_defs</proj4>
                    <srsid>2267</srsid>
                    <srid>32633</srid>
                    <epsg>32633</epsg>
                    <description>WGS 84 / UTM zone 33N</description>
                    <projectionacronym>utm</projectionacronym>
                    <ellipsoidacronym>WGS84</ellipsoidacronym>
                    <geographicflag>true</geographicflag>
                </spatialrefsys>
            </srs>
            <transparencyLevelInt>255</transparencyLevelInt>
            <provider></provider>
            <rasterproperties>
                <mDebugOverlayFlag boolean="false" />
                <drawingStyle>MULTI_BAND_COLOR</drawingStyle>
                <mColorShadingAlgorithm>UNDEFINED_SHADING_ALGORITHM</mColorShadingAlgorithm>
                <mInvertPixelsFlag boolean="false" />
                <mRedBandName>8 : Undefined</mRedBandName>
                <mGreenBandName>7 : Undefined</mGreenBandName>
                <mBlueBandName>5 : Undefined</mBlueBandName>
                <mGrayBandName>Not Set</mGrayBandName>
                <mStandardDeviations>0</mStandardDeviations>
                <mContrastEnhancementAlgorithm>STRETCH_TO_MINMAX</mContrastEnhancementAlgorithm>
                <contrastEnhancementMinMaxValues>
                    <minMaxEntry>
                        <min>0</min>
                        <max>137</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>0</min>
                        <max>161</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>0</min>
                        <max>181</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>0</min>
                        <max>151</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>76</min>
                        <max>108</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>0</min>
                        <max>255</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>130</min>
                        <max>194</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>170</min>
                        <max>246</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>0</min>
                        <max>255</max>
                    </minMaxEntry>
                </contrastEnhancementMinMaxValues>
                <mNoDataValue mValidNoDataValue="false" >-9999.000000</mNoDataValue>
            </rasterproperties>
        </maplayer>
        <maplayer minScale="1" maxScale="1e+08" scaleBasedVisibilityFlag="0" type="raster" >
            <id>landsat_clip20080124154848331</id>
            <datasource>/Users/timlinux/gisdata/LandsatTM/landsat_clip.tif</datasource>
            <layername>landsat_clip</layername>
            <srs>
                <spatialrefsys>
                    <proj4>+proj=utm +zone=33 +ellps=WGS84 +datum=WGS84 +units=m +no_defs</proj4>
                    <srsid>2267</srsid>
                    <srid>32633</srid>
                    <epsg>32633</epsg>
                    <description>WGS 84 / UTM zone 33N</description>
                    <projectionacronym>utm</projectionacronym>
                    <ellipsoidacronym>WGS84</ellipsoidacronym>
                    <geographicflag>false</geographicflag>
                </spatialrefsys>
            </srs>
            <transparencyLevelInt>255</transparencyLevelInt>
            <provider></provider>
            <rasterproperties>
                <mDebugOverlayFlag boolean="false" />
                <drawingStyle>MULTI_BAND_COLOR</drawingStyle>
                <mColorShadingAlgorithm>UNDEFINED_SHADING_ALGORITHM</mColorShadingAlgorithm>
                <mInvertPixelsFlag boolean="false" />
                <mRedBandName>8 : Undefined</mRedBandName>
                <mGreenBandName>7 : Undefined</mGreenBandName>
                <mBlueBandName>5 : Undefined</mBlueBandName>
                <mGrayBandName>Not Set</mGrayBandName>
                <mStandardDeviations>0</mStandardDeviations>
                <mContrastEnhancementAlgorithm>STRETCH_TO_MINMAX</mContrastEnhancementAlgorithm>
                <contrastEnhancementMinMaxValues>
                    <minMaxEntry>
                        <min>122</min>
                        <max>130</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>134</min>
                        <max>147</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>88</min>
                        <max>141</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>0</min>
                        <max>255</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>76</min>
                        <max>108</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>0</min>
                        <max>255</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>130</min>
                        <max>194</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>170</min>
                        <max>246</max>
                    </minMaxEntry>
                    <minMaxEntry>
                        <min>0</min>
                        <max>255</max>
                    </minMaxEntry>
                </contrastEnhancementMinMaxValues>
                <mNoDataValue mValidNoDataValue="false" >-9999.000000</mNoDataValue>
            </rasterproperties>
        </maplayer>
</qgis>
