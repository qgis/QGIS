<?xml version="1.0"?>
<!--
book2menu.xsl generates the HTML menu. It outputs XML/HTML of the form:
  <div class="menu">
     ...
  </div>
which is then merged with other HTML by site2xhtml.xsl

$Id$
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- ================================================================ -->
  <!-- These templates SHOULD be overridden                             -->
  <!-- ================================================================ -->

  <xsl:template name="selected">
    <xsl:value-of select="@label"/>
  </xsl:template>

  <xsl:template name="unselected">
    <a href="{@href}">
      <xsl:if test="@description">
        <xsl:attribute name="title">
          <xsl:value-of select="@description"/>
        </xsl:attribute>
      </xsl:if>
      <xsl:value-of select="@label"/>
    </a>
  </xsl:template>

  <xsl:template name="print-external">
    <!-- Use apply-imports when overriding -->
    <xsl:value-of select="@label"/>
  </xsl:template>


  <!-- ================================================================ -->
  <!-- These templates CAN be overridden                                -->
  <!-- ================================================================ -->

  <!-- Eg, if tab href is 'index.html#foo', this will be called when index.html
  is selected -->
  <xsl:template name="selected-anchor">
    <!-- By default, render as unselected so that it is clickable (takes user
    to the anchor) -->
    <xsl:call-template name="unselected"/>
  </xsl:template>

  <xsl:template name="unselected-anchor">
    <xsl:call-template name="unselected"/>
  </xsl:template>


  <xsl:template match="book">
    <xsl:apply-templates select="menu"/>
  </xsl:template>


  <xsl:template match="menu">
    <div class="menu">
      <xsl:call-template name="base-menu"/>
    </div>
  </xsl:template>

  <xsl:template match="menu-item">
    <!-- Use apply-imports when overriding -->

    <xsl:variable name="href-nofrag">
      <xsl:call-template name="path-nofrag">
        <xsl:with-param name="path" select="@href"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="node-path">
      <xsl:call-template name="normalize">
        <xsl:with-param name="path" select="concat($dirname, $href-nofrag)"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:choose>
      <!-- Compare with extensions stripped -->
      <xsl:when test="$node-path = $path-nofrag">
        <xsl:choose>
          <xsl:when test="contains(@href, '#')">
            <xsl:call-template name="selected-anchor"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="selected"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:otherwise>
        <xsl:choose>
          <xsl:when test="contains(@href, '#')">
            <xsl:call-template name="unselected-anchor"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="unselected"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- ================================================================ -->
  <!-- These templates SHOULD NOT be overridden                         -->
  <!-- ================================================================ -->


  <xsl:param name="path"/>

  <xsl:include href="pathutils.xsl"/>

  <xsl:variable name="filename">
    <xsl:call-template name="filename">
      <xsl:with-param name="path" select="$path"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="path-nofrag">
    <xsl:call-template name="path-nofrag">
      <xsl:with-param name="path" select="$path"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="path-nofrag">
    <xsl:call-template name="path-nofrag">
      <xsl:with-param name="path" select="$path"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="dirname">
    <xsl:call-template name="dirname">
      <xsl:with-param name="path" select="$path"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:template match="external">
    <li>
      <xsl:choose>
        <xsl:when test="starts-with(@href, $path-nofrag)">
          <span class="externalSelected">
            <xsl:call-template name="print-external"/>
          </span>
        </xsl:when>
        <xsl:otherwise>
          <a href="{@href}" target="_blank"><xsl:value-of select="@label"/></a>
        </xsl:otherwise>
      </xsl:choose>
    </li>
  </xsl:template>

  <xsl:template match="menu-item[@type='hidden']"/>

  <xsl:template match="external[@type='hidden']"/>

  <xsl:template name="base-menu">
    <xsl:apply-templates/>
  </xsl:template>

</xsl:stylesheet>
