<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:fo="http://www.w3.org/1999/XSL/Format"
                xmlns:fox="http://xml.apache.org/fop/extensions"
                version="1.0">

<xsl:template match="document" mode="outline">
      <xsl:apply-templates select="body/section" mode="outline"/>
</xsl:template>

<xsl:template match="section" mode="outline">
  <fox:outline>
    <xsl:attribute name="internal-destination">
      <xsl:choose>
        <xsl:when test="normalize-space(@id)!=''">
          <xsl:value-of select="@id"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="generate-id()"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
    <fox:label>
      <xsl:number format="1.1.1.1.1.1.1" count="section" level="multiple"/>
      <xsl:text> </xsl:text>
      <xsl:value-of select="title"/>

    </fox:label>
    <xsl:apply-templates select="section" mode="outline"/>
  </fox:outline>
</xsl:template>

</xsl:stylesheet>
