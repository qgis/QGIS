<?xml version="1.0"?>
<!--
This stylesheet generates 'tabs' at the top left of the screen.
See the imported tab2menu.xsl for details.
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import href="../../../common/xslt/html/tab2menu.xsl"/>

  <xsl:template name="pre-separator">
  </xsl:template>

  <xsl:template name="post-separator">
  </xsl:template>

  <xsl:template name="separator">
    <xsl:text> | </xsl:text>
  </xsl:template>

  <xsl:template name="selected" mode="print">
    <span class="selectedTab">
      <xsl:call-template name="base-selected"/>
    </span>
  </xsl:template>

  <xsl:template name="not-selected" mode="print">
    <span class="unselectedTab">
    <!-- Called from 'not-selected' -->
     <a>
      <xsl:attribute name="href">
        <xsl:call-template name="calculate-tab-href">
          <xsl:with-param name="tab" select="."/>
          <xsl:with-param name="path" select="$path"/>
        </xsl:call-template>
      </xsl:attribute>
      <xsl:value-of select="@label"/>
     </a>
    </span>
  </xsl:template>

</xsl:stylesheet>
