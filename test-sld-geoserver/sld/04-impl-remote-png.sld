<?xml version="1.0" encoding="UTF-8"?>
<StyledLayerDescriptor xmlns="http://www.opengis.net/sld" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.1.0" xmlns:ogc="http://www.opengis.net/ogc" xmlns:xlink="http://www.w3.org/1999/xlink" xsi:schemaLocation="http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" xmlns:se="http://www.opengis.net/se">
  <NamedLayer>
    <se:Name>TREES_LAYER</se:Name>
    <UserStyle>
      <se:Name>TREES_LAYER</se:Name>
      <se:FeatureTypeStyle>
        <se:Rule>
          <se:Name>Trees</se:Name>
          <se:Description>
            <se:Title>Trees</se:Title>
          </se:Description>
          <se:PointSymbolizer>
            <se:Graphic>
              <se:ExternalGraphic>
                <se:OnlineResource xlink:type="simple" xlink:href="https://picsum.photos/id/237/200/300"/>
                <se:Format>image/png</se:Format>
              </se:ExternalGraphic>
            </se:Graphic>
          </se:PointSymbolizer>
        </se:Rule>
      </se:FeatureTypeStyle>
    </UserStyle>
  </NamedLayer>
</StyledLayerDescriptor>