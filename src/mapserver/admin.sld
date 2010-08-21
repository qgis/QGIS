<StyledLayerDescriptor xmlns="http://www.opengis.net/sld">
<UserLayer>
<Name>wfslayer</Name>
<RemoteOWS>
<Service>WFS</Service>
<OnlineResource href="http://karlinapp.ethz.ch/cgi-bin/mapserv?map=/home/marco/mapfiles/mapfile1.map&amp;" type="states" />
</RemoteOWS>
<UserStyle>
<Name>userstyle</Name>
<FeatureTypeStyle>
<Rule>
<PolygonSymbolizer>
<Stroke><sld:CssParameter sld:name="stroke">#0000ff</sld:CssParameter>
<sld:CssParameter sld:name="stroke-width">3</sld:CssParameter>
</Stroke>
<Fill>
<sld:CssParameter sld:name="fill">#0000ff</sld:CssParameter>
</Fill>
</PolygonSymbolizer>
</Rule>
</FeatureTypeStyle>
</UserStyle>
</UserLayer>
</StyledLayerDescriptor>
