<?xml version="1.0" ?>
<sld:StyledLayerDescriptor version="1.0.0" xmlns="http://www.opengis.net/sld" xmlns:gml="http://www.opengis.net/gml" xmlns:ogc="http://www.opengis.net/ogc" xmlns:sld="http://www.opengis.net/sld">
	<sld:NamedLayer>
		<sld:Name>testSimpleMarkerRotation</sld:Name>
		<sld:UserStyle>
			<sld:Name>testSimpleMarkerRotation</sld:Name>
			<sld:FeatureTypeStyle>
				<sld:Name>name</sld:Name>
				<sld:Rule>
					<sld:Name>Single symbol</sld:Name>
					<sld:PointSymbolizer>
						<sld:Graphic>
							<sld:Mark>
								<sld:WellKnownName>star</sld:WellKnownName>
								<sld:Fill>
									<sld:CssParameter name="fill">#ff0000</sld:CssParameter>
								</sld:Fill>
								<sld:Stroke>
									<sld:CssParameter name="stroke">#00ff00</sld:CssParameter>
								</sld:Stroke>
							</sld:Mark>
							<sld:Size>36</sld:Size>
							<sld:Rotation>50.0</sld:Rotation>
						</sld:Graphic>
					</sld:PointSymbolizer>
				</sld:Rule>
			</sld:FeatureTypeStyle>
		</sld:UserStyle>
	</sld:NamedLayer>
</sld:StyledLayerDescriptor>
