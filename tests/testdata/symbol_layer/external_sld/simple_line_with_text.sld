<?xml version="1.0" encoding="UTF-8"?>
<StyledLayerDescriptor xmlns="http://www.opengis.net/sld" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:ogc="http://www.opengis.net/ogc" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" version="1.1.0" xmlns:se="http://www.opengis.net/se">
 <NamedLayer>
  <se:Name>simple_line</se:Name>
  <UserStyle>
   <se:Name>simple_line</se:Name>
   <se:FeatureTypeStyle>
    <se:Rule>
     <se:Name>Single symbol</se:Name>
     <se:LineSymbolizer>
      <se:Stroke>
       <se:SvgParameter name="stroke">#003eba</se:SvgParameter>
       <se:SvgParameter name="stroke-opacity">0.098</se:SvgParameter>
       <se:SvgParameter name="stroke-width">2</se:SvgParameter>
       <se:SvgParameter name="stroke-linejoin">bevel</se:SvgParameter>
       <se:SvgParameter name="stroke-linecap">square</se:SvgParameter>
      </se:Stroke>
     </se:LineSymbolizer>
    </se:Rule>
    <se:Rule>
     <se:TextSymbolizer>
      <se:Label>
       <ogc:PropertyName>name</ogc:PropertyName>
      </se:Label>
      <se:Font>
        <se:SvgParameter name="font-family">QGIS Vera Sans</se:SvgParameter>
        <se:SvgParameter name="font-size">18</se:SvgParameter>
        <se:SvgParameter name="font-weight">bold</se:SvgParameter>
      </se:Font>
      <se:LabelPlacement>
       <se:PointPlacement>
        <se:AnchorPoint>
          <se:AnchorPointX>1</se:AnchorPointX>
          <se:AnchorPointY>0</se:AnchorPointY>
        </se:AnchorPoint>
       </se:PointPlacement>
      </se:LabelPlacement>
      <se:Fill>
       <se:SvgParameter name="fill">#FF0000</se:SvgParameter>
      </se:Fill>
      <se:VendorOption name="maxDisplacement">1</se:VendorOption>
     </se:TextSymbolizer>
    </se:Rule>
   </se:FeatureTypeStyle>
  </UserStyle>
 </NamedLayer>
</StyledLayerDescriptor>
