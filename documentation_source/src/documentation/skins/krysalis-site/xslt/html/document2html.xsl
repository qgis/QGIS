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
      <xsl:if test="normalize-space(header/title)!=''">
        <table class="title">
          <tr> 
            <td valign="middle"> 
              <h1>
                <xsl:value-of select="header/title"/>
              </h1>
            </td>
            <!--
            <xsl:call-template name="printerfriendlylink"/>
            <xsl:call-template name="xmllink"/>
            -->
            <xsl:call-template name="pdflink"/>
          </tr>
        </table>
      </xsl:if>
      <xsl:if test="normalize-space(header/subtitle)!=''">
        <h3>
          <xsl:value-of select="header/subtitle"/>
        </h3>
      </xsl:if>

      <xsl:apply-templates select="body"/>

      <xsl:if test="header/authors">
        <p align="right">
          <font size="-2">
            <xsl:for-each select="header/authors/person">
              <xsl:choose>
                <xsl:when test="position()=1">by&#160;</xsl:when>
                <xsl:otherwise>,&#160;</xsl:otherwise>
              </xsl:choose>
              <xsl:value-of select="@name"/>
            </xsl:for-each>
          </font>
        </p>
      </xsl:if>

    </div>
  </xsl:template>

  <xsl:template match="body">

    <xsl:if test="section and not($notoc='true')">
      <toc>
        <xsl:for-each select="section">
          <tocc>
            <toca>
              <xsl:attribute name="href">
                <xsl:text>#</xsl:text><xsl:call-template name="generate-id"/>
              </xsl:attribute>
              <xsl:value-of select="title"/>
            </toca>
            <xsl:if test="section">
              <toc2>
                <xsl:for-each select="section">
                  <tocc>
                    <toca>
                      <xsl:attribute name="href">
                        <xsl:text>#</xsl:text><xsl:call-template name="generate-id"/>
                      </xsl:attribute>
                      <xsl:value-of select="title"/>
                    </toca>
                  </tocc>
                </xsl:for-each>
              </toc2>
            </xsl:if>
          </tocc>
        </xsl:for-each>
      </toc>
    </xsl:if>
    <xsl:apply-templates/>
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
    <a name="{generate-id()}"/>
    <xsl:apply-templates select="@id"/>

    <xsl:variable name = "level" select = "count(ancestor::section)+1" />

    <xsl:choose>
      <xsl:when test="$level=1">

        <table cellpadding="0" cellspacing="0" border="0" width="100%">
          <tbody>
            <tr>
              <td width="9" height="10"></td>
              <td><h3><xsl:value-of select="title"/></h3></td>
              <td></td>
            </tr>
            <tr>
              <td class="bottom-left-thick"></td>
              <td bgcolor="#a5b6c6"></td>
              <td class="bottom-right-thick"></td>
            </tr>

          </tbody>            
        </table>

        <div class="section"><xsl:apply-templates/></div>  

      </xsl:when>
      <xsl:when test="$level=2">

        <table cellpadding="0" cellspacing="0" border="0">
          <tbody>
            <tr>
              <td width="9" height="10"></td>
              <td><h4><xsl:value-of select="title"/></h4></td>
              <td></td>
            </tr>
            <tr>
              <td class="bottom-left"></td>
              <td bgcolor="#a5b6c6"></td>
              <td class="bottom-right"></td>
            </tr>

          </tbody>            
        </table>
        <xsl:apply-templates select="*[not(self::title)]"/>
      </xsl:when>
      <!-- If a faq, answer sections will be level 3 (1=Q/A, 2=part) -->
      <xsl:when test="$level=3 and $notoc='true'">
        <h4 class="faq"><xsl:value-of select="title"/></h4>
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

  <xsl:template match="fixme | note | warning">
    <xsl:apply-imports/>
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

</xsl:stylesheet>
