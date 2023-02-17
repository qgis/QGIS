<?xml version="1.0" encoding="UTF-8"?>
<StyledLayerDescriptor xmlns="http://www.opengis.net/sld" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:ogc="http://www.opengis.net/ogc" xsi:schemaLocation="http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" version="1.1.0" xmlns:se="http://www.opengis.net/se" xmlns:xlink="http://www.w3.org/1999/xlink">
    <NamedLayer>
    <se:Name>db_point</se:Name>
        <UserStyle>
            <se:Name>db_point_style</se:Name>
            <se:FeatureTypeStyle>
                <se:Rule>
                    <se:Name>Single symbol</se:Name>
                    <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
                        <ogc:PropertyIsEqualTo>
                            <ogc:PropertyName>gid</ogc:PropertyName>
                            <ogc:Literal>1</ogc:Literal>
                        </ogc:PropertyIsEqualTo>
                    </ogc:Filter>
                    <se:PointSymbolizer uom="http://www.opengeospatial.org/se/units/metre">
                        <se:Graphic>
                            <se:Mark>
                                <se:WellKnownName>square</se:WellKnownName>
                                <se:Fill>
                                    <se:SvgParameter name="fill">5e86a1</se:SvgParameter>
                                </se:Fill>
                                <se:Stroke>
                                    <se:SvgParameter name="stroke">000000</se:SvgParameter>
                                </se:Stroke>
                            </se:Mark>
                            <se:Size>1000000</se:Size>
                        </se:Graphic>
                    </se:PointSymbolizer>
                </se:Rule>
            </se:FeatureTypeStyle>
        </UserStyle>
    </NamedLayer>
</StyledLayerDescriptor>