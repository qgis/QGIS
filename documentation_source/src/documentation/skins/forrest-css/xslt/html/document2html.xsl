<?xml version="1.0"?>
<!--
This stylesheet contains the majority of templates for converting documentv11
to HTML.  It renders XML as HTML in this form:

  <div class="content">
   ...
  </div>

..which site2xhtml.xsl then combines with HTML from the index (book2menu.xsl)
and tabs (tab2menu.xsl) to generate the final HTML.

$Id$
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import href="../../../common/xslt/html/document2html.xsl"/>

  <xsl:template match="document">

    <div class="content">
      <xsl:call-template name="pdflink"/>
      <xsl:if test="normalize-space(header/title)!=''">
              <h1><xsl:value-of select="header/title"/></h1>
      </xsl:if>
      
      <xsl:if test="normalize-space(header/subtitle)!=''">
        <h3>
          <xsl:value-of select="header/subtitle"/>
        </h3>
      </xsl:if>

      <xsl:apply-templates select="body"/>

      <xsl:if test="header/authors">
        <div class="author">
            <xsl:for-each select="header/authors/person">
              <xsl:choose>
                <xsl:when test="position()=1">by </xsl:when>
                <xsl:otherwise>, </xsl:otherwise>
              </xsl:choose>
              <xsl:value-of select="@name"/>
            </xsl:for-each>
        </div>
      </xsl:if>

    </div>
  </xsl:template>

  <xsl:template match="body">
    <xsl:if test="$max-depth&gt;0 and not($notoc='true')" >
      <xsl:call-template name="minitoc">
        <xsl:with-param name="tocroot" select="."/>
        <xsl:with-param name="depth">1</xsl:with-param>
      </xsl:call-template>
    </xsl:if>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template name="toclink">
    <xsl:variable name="tocitem" select="normalize-space(title)"/>
    <xsl:if test="string-length($tocitem)>0">
      <li>
      <a>
        <xsl:attribute name="href">
          <xsl:text>#</xsl:text><xsl:call-template name="generate-id"/>
        </xsl:attribute>
        <xsl:value-of select="$tocitem"/>
      </a>
      </li>
    </xsl:if>
  </xsl:template>
  


  <xsl:template name="minitoc">  
    <xsl:param name="tocroot"/>
    <xsl:param name="depth"/>     
    <ul>
      <xsl:for-each select="$tocroot/section">
        <xsl:call-template name="toclink"/>
        <xsl:if test="$depth&lt;$max-depth">
          <xsl:call-template name="minitoc">
            <xsl:with-param name="tocroot" select="."/>
            <xsl:with-param name="depth" select="$depth + 1"/>          
          </xsl:call-template>
        </xsl:if>      
      </xsl:for-each>
    </ul>
  </xsl:template>


  <xsl:template name="generate-id">
    <xsl:choose>
      <xsl:when test="@id">
        <xsl:value-of select="@id"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="generate-id(.)"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@id">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="section">
    <xsl:apply-templates select="@id"/>

    <xsl:variable name = "level" select = "count(ancestor::section)+1" />

    <xsl:choose>
      <xsl:when test="$level=1">
        <h2><xsl:value-of select="title"/></h2>
        <xsl:apply-templates/>  
      </xsl:when>
      <xsl:when test="$level=2">
        <h3><xsl:value-of select="title"/></h3>
        <div class="section"><xsl:apply-templates select="*[not(self::title)]"/></div>
      </xsl:when>
      <!-- If a faq, answer sections will be level 3 (1=Q/A, 2=part) -->
      <xsl:when test="$level=3 and $notoc='true'">
        <h4><xsl:value-of select="title"/></h4>
        <div align="right"><a href="#{@id}-menu">^</a></div>
        <div style="margin-left: 15px">
          <xsl:apply-templates select="*[not(self::title)]"/>
        </div>
      </xsl:when>
      <xsl:when test="$level=3">
        <h4><xsl:value-of select="title"/></h4>
        <xsl:apply-templates select="*[not(self::title)]"/>

      </xsl:when>

      <xsl:otherwise>
        <h5><xsl:value-of select="title"/></h5>
        <xsl:apply-templates select="*[not(self::title)]"/>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>  

  <xsl:template match="note | warning | fixme">
    <xsl:apply-templates select="@id"/>
    <div class="frame {local-name()}">
      <div class="label">
        <xsl:choose>
          <xsl:when test="local-name() = 'note'">Note</xsl:when>
          <xsl:when test="local-name() = 'warning'">Warning</xsl:when>
          <xsl:otherwise>Fixme (
               <xsl:value-of select="@author"/>

               )</xsl:otherwise>
        </xsl:choose>
      </div>
      <div class="framecontent">
        <xsl:apply-templates/>
      </div>
    </div>
  </xsl:template>

  <xsl:template match="link">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="jump">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="fork">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="p[@xml:space='preserve']">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="source">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="anchor">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="icon">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="code">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="figure">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="table">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="title">
    <!-- do not show title elements, they are already in other places-->
  </xsl:template>
  
  <!-- Generates the PDF link -->
  <xsl:template name="pdflink">
  	<a href="{$filename-noext}.pdf" id="printable"><img src="{$skin-img-dir}/pdfdoc.gif"
      alt="PDF"/>PDF version</a>
  </xsl:template>
  

</xsl:stylesheet>
