<?xml version="1.0"?>
<!--
book2menu.xsl generates the HTML menu.  See the imported book2menu.xsl for
details.
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import href="../../../common/xslt/html/book2menu.xsl"/>

  <xsl:template match="book">
    <div class="menu">
      <ul>
        <xsl:apply-templates select="menu"/>
      </ul>
    </div>
  </xsl:template>

  <xsl:template match="menu">
    <li>
      <font color="#CFDCED"><xsl:value-of select="@label"/></font>
      <ul>
        <xsl:apply-templates/>
      </ul>
    </li>
  </xsl:template>

  <xsl:template match="menu-item[@type='hidden']"/>

  <xsl:template match="menu-item">
    <li>
      <xsl:apply-imports/>
    </li>
  </xsl:template>

  <xsl:template name="selected">
    <span class="sel">
      <font color="#ffcc00">
        <xsl:value-of select="@label"/>
      </font>
    </span>
  </xsl:template>

  <xsl:template name="print-external">
    <font color="#ffcc00">
      <xsl:apply-imports/>
    </font>
  </xsl:template>

</xsl:stylesheet>
