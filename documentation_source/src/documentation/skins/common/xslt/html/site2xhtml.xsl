<?xml version="1.0"?>
<!--
site2xhtml.xsl is the final stage in HTML page production.  It merges HTML from
document2html.xsl, tab2menu.xsl and book2menu.xsl, and adds the site header,
footer, searchbar, css etc.  As input, it takes XML of the form:

<site>
  <div class="menu">
    ...
  </div>
  <div class="tab">
    ...
  </div>
  <div class="content">
    ...
  </div>
</site>

$Id$
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- Default skinconf.xml in the skins/ directory -->
  <xsl:param name="config-file" select="'../../../../skinconf.xml'"/>
  <xsl:variable name="config" select="document($config-file)/skinconfig"/>
  <xsl:param name="path"/>

  <xsl:include href="dotdots.xsl"/>
  <xsl:include href="pathutils.xsl"/>
  <xsl:include href="renderlogo.xsl"/>

  <!-- Path (..'s) to the root directory -->
  <xsl:variable name="root">
    <xsl:call-template name="dotdots">
      <xsl:with-param name="path" select="$path"/>
    </xsl:call-template>
  </xsl:variable>

  <!-- Source filename (eg 'foo.xml') of current page -->
  <xsl:variable name="filename">
    <xsl:call-template name="filename">
      <xsl:with-param name="path" select="$path"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="skin-img-dir" select="concat(string($root), 'skin/images')"/>
  <xsl:variable name="spacer" select="concat($root, 'skin/images/spacer.gif')"/>

  <xsl:template match="site">
    <html>
      <head>
        <title><xsl:value-of select="div[@class='content']/table/tr/td/h1"/></title>
      </head>
      <body>
        <xsl:if test="$config/group-url">
          <xsl:call-template name="renderlogo">
            <xsl:with-param name="name" select="$config/group-name"/>
            <xsl:with-param name="url" select="$config/group-url"/>
            <xsl:with-param name="logo" select="$config/group-logo"/>
            <xsl:with-param name="root" select="$root"/>
            <xsl:with-param name="description" select="$config/group-description"/>
          </xsl:call-template>
        </xsl:if>
        <xsl:call-template name="renderlogo">
          <xsl:with-param name="name" select="$config/project-name"/>
          <xsl:with-param name="url" select="$config/project-url"/>
          <xsl:with-param name="logo" select="$config/project-logo"/>
          <xsl:with-param name="root" select="$root"/>
          <xsl:with-param name="description" select="$config/project-description"/>
        </xsl:call-template>
        <xsl:comment>================= start Tabs ==================</xsl:comment>
        <xsl:apply-templates select="div[@class='tab']"/>
        <xsl:comment>================= end Tabs ==================</xsl:comment>
        <xsl:comment>================= start Menu items ==================</xsl:comment>
        <xsl:apply-templates select="div[@class='menu']"/>
        <xsl:comment>================= end Menu items ==================</xsl:comment>
        <xsl:comment>================= start Content==================</xsl:comment>
        <xsl:apply-templates select="div[@class='content']"/>
        <xsl:comment>================= end Content==================</xsl:comment>

        <xsl:comment>================= start Footer ==================</xsl:comment>
        Copyright &#169; <xsl:value-of select="$config/year"/>&#160;<xsl:value-of
          select="$config/vendor"/> All rights reserved.
       /*  <script language="JavaScript" type="text/javascript"><![CDATA[<!--
          document.write(" - "+"Last Published: " + document.lastModified);
          //  -->]]></script> */
        <xsl:if test="$config/host-logo and not($config/host-logo = '')">
          <a href="{$config/host-url}">
            <xsl:call-template name="renderlogo">
              <xsl:with-param name="name" select="$config/host-name"/>
              <xsl:with-param name="url" select="$config/host-url"/>
              <xsl:with-param name="logo" select="$config/host-logo"/>
              <xsl:with-param name="root" select="$root"/>
            </xsl:call-template>
          </a>
        </xsl:if>
        <xsl:if test="$filename = 'index.html' and $config/credits">
          <xsl:for-each select="$config/credits/credit[not(@role='pdf')]">
            <xsl:call-template name="renderlogo">
              <xsl:with-param name="name" select="name"/>
              <xsl:with-param name="url" select="url"/>
              <xsl:with-param name="logo" select="image"/>
              <xsl:with-param name="root" select="$root"/>
              <xsl:with-param name="width" select="width"/>
              <xsl:with-param name="height" select="height"/>
            </xsl:call-template>
          </xsl:for-each>
        </xsl:if>
        <a href="http://validator.w3.org/check/referer"><img class="skin" border="0"
            src="http://www.w3.org/Icons/valid-html401"
            alt="Valid HTML 4.01!" height="31" width="88"/></a>
      </body>
    </html>
  </xsl:template>

  <!-- Add links to any standards-compliance logos -->
  <xsl:template name="compliancy-logos">
    <xsl:if test="$config/disable-compliance-links = 'false'">
      <a href="http://validator.w3.org/check/referer"><img class="logoImage" 
          src="{$skin-img-dir}/valid-html401.png"
          alt="Valid HTML 4.01!" height="31" width="88"/></a>
          
      <a href="http://jigsaw.w3.org/css-validator/"><img class="logoImage" 
          src="{$skin-img-dir}/vcss.png" 
          alt="Valid CSS!" height="31" width="88"/></a>
    </xsl:if>
  </xsl:template>


  <xsl:template match="node()|@*" priority="-1">
    <xsl:copy>
      <xsl:apply-templates select="@*"/>
      <xsl:apply-templates/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
