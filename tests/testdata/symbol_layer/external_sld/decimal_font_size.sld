<?xml version="1.0" encoding="UTF-8"?>
<StyledLayerDescriptor xmlns="http://www.opengis.net/sld" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:ogc="http://www.opengis.net/ogc" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" version="1.1.0" xmlns:se="http://www.opengis.net/se">
 <NamedLayer>
  <se:Name>font_decimal</se:Name>
  <UserStyle>
   <se:Name>font_decimal</se:Name>
   <se:FeatureTypeStyle>
    <se:Rule>
     <se:Name>Single symbol</se:Name>
     <se:PointSymbolizer>
      <se:Graphic>
       <se:Mark>
        <se:WellKnownName>circle</se:WellKnownName>
        <se:Fill>
          <se:SvgParameter name="fill">#000000</se:SvgParameter>
        </se:Fill>
       </se:Mark>
      </se:Graphic>
     </se:PointSymbolizer>
    </se:Rule>
    <se:Rule>
     <se:TextSymbolizer>
      <se:Label>
        <ogc:PropertyName>fldtxt</ogc:PropertyName>
      </se:Label>
      <se:Font>
        <se:SvgParameter name="font-size">13.5</se:SvgParameter>
      </se:Font>
      <se:Fill>
       <se:SvgParameter name="fill">#000000</se:SvgParameter>
      </se:Fill>
     </se:TextSymbolizer>
    </se:Rule>
   </se:FeatureTypeStyle>
  </UserStyle>
 </NamedLayer>
</StyledLayerDescriptor>
