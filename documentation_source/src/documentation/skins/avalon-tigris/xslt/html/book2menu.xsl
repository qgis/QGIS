<?xml version="1.0"?>
<!--
book2menu.xsl generates the HTML menu.  See the imported book2menu.xsl for
details.
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import href="../../../common/xslt/html/book2menu.xsl"/>

  <xsl:template match="book">
    <div class="menuBar">
      <xsl:apply-templates select="menu"/>
    </div>
  </xsl:template>

  <xsl:template match="menu">
    <div class="menu">
      <span class="menuLabel"><xsl:value-of select="@label"/></span>
      <xsl:call-template name="base-menu"/>
    </div>
  </xsl:template>

  <xsl:template match="menu-item[@type='hidden']"/>

  <xsl:template match="menu-item">
    <div class="menuItem">
      <xsl:apply-imports/>
    </div>
  </xsl:template>

  <xsl:template name="selected">
    <span class="menuSelected">
      <xsl:value-of select="@label"/>
    </span>
  </xsl:template>

  <xsl:template name="print-external">
    <font color="#ffcc00">
      <span class="externalSelected">
        <xsl:apply-imports/>
      </span>
    </font>
  </xsl:template>


</xsl:stylesheet>
