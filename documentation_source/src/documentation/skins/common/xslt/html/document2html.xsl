<?xml version="1.0"?>
<!--
This stylesheet contains the majority of templates for converting documentv11
to HTML.  It renders XML as HTML in this form:

  <div class="content">
   ...
  </div>

..which site2xhtml.xsl then combines with HTML from the index (book2menu.xsl)
and tabs (tab2menu.xsl) to generate the final HTML.

Section handling
  - <a name/> anchors are added if the id attribute is specified

$Id$
-->
<xsl:stylesheet version="1.0" 
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- the skinconf file -->
  <xsl:param name="config-file" select="'../../../../skinconf.xml'"/>
  <xsl:variable name="config" select="document($config-file)/skinconfig"/>
  
  <!-- If true, a PDF link for this page will not be generated -->
  <xsl:variable name="disable-pdf-link" select="$config/disable-pdf-link"/>
  <!-- If true, a "print" link for this page will not be generated -->
  <xsl:variable name="disable-print-link" select="$config/disable-print-link"/>
  <!-- If true, an XML link for this page will not be generated -->
  <xsl:variable name="disable-xml-link" select="$config/disable-xml-link"/>  
  <!-- Get the section depth to use when generating the minitoc (default is 2) -->
  <xsl:variable name="config-max-depth" select="$config/toc/@level"/>
  <!-- Whether to obfuscate email links -->
  <xsl:variable name="obfuscate-mail-links" select="$config/obfuscate-mail-links"/>

  <xsl:variable name="max-depth">
    <xsl:choose>
      <xsl:when test="string-length($config-max-depth)&gt;0">
        <xsl:value-of select="$config-max-depth"/>
      </xsl:when>
      <xsl:otherwise>2</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
    
  <xsl:param name="notoc"/>
  <xsl:param name="path"/>
  <!-- <xsl:include href="split.xsl"/> -->
  <xsl:include href="dotdots.xsl"/>
  <xsl:include href="pathutils.xsl"/>

  <!-- Path to site root, eg '../../' -->
  <xsl:variable name="root">
    <xsl:call-template name="dotdots">
      <xsl:with-param name="path" select="$path"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="filename-noext">
    <xsl:call-template name="filename-noext">
      <xsl:with-param name="path" select="$path"/>
    </xsl:call-template>
  </xsl:variable>
 
  <xsl:variable name="skin-img-dir" select="concat(string($root), 'skin/images')"/>

  <xsl:template match="document">
    <div class="content">
      <table summary="" class="title">
        <tr> 
          <td valign="middle"> 
            <xsl:if test="normalize-space(header/title)!=''">
              <h1>
                <xsl:value-of select="header/title"/>
              </h1>
            </xsl:if>
          </td>
          <xsl:call-template name="printlink"/> 
          <xsl:call-template name="pdflink"/>
          <xsl:call-template name="xmllink"/>
        </tr>
      </table>
      <xsl:if test="normalize-space(header/subtitle)!=''">
        <h3>
          <xsl:value-of select="header/subtitle"/>
        </h3>
      </xsl:if>
      <xsl:apply-templates select="header/type"/>
      <xsl:apply-templates select="header/notice"/>
      <xsl:apply-templates select="header/abstract"/>
      <xsl:apply-templates select="body"/>
      <div class="attribution">
        <xsl:apply-templates select="header/authors"/>
        <xsl:if test="header/authors and header/version">
          <xsl:text>; </xsl:text>
        </xsl:if>
        <xsl:apply-templates select="header/version"/>
      </div>
    </div>
  </xsl:template>

  <!-- Generates the "printer friendly version" link -->
  <xsl:template name="printlink">
    <xsl:if test="$disable-print-link = 'false'"> 
<script type="text/javascript" language="Javascript">
function printit() {  
if (window.print) {
    window.print() ;  
} else {
    var WebBrowser = '&lt;OBJECT ID="WebBrowser1" WIDTH="0" HEIGHT="0" CLASSID="CLSID:8856F961-340A-11D0-A96B-00C04FD705A2">&lt;/OBJECT>';
document.body.insertAdjacentHTML('beforeEnd', WebBrowser);
    WebBrowser1.ExecWB(6, 2);//Use a 1 vs. a 2 for a prompting dialog box    WebBrowser1.outerHTML = "";  
}
}
</script>

<script type="text/javascript" language="Javascript">
var NS = (navigator.appName == "Netscape");
var VERSION = parseInt(navigator.appVersion);
if (VERSION > 3) {
    document.write('<td align="center" width="40" nowrap="nowrap">');
    document.write('  <a href="javascript:printit()" class="dida">');
    document.write('    <img class="skin" src="{$skin-img-dir}/printer.gif" alt="Print this Page"/><br />');
    document.write('  print</a>');
    document.write('</td>');
}
</script>

    </xsl:if>
  </xsl:template>

  <!-- Generates the PDF link -->
  <xsl:template name="pdflink">
    <xsl:if test="not($config/disable-pdf-link) or $disable-pdf-link = 'false'"> 
      <td align="center" width="40" nowrap="nowrap"><a href="{$filename-noext}.pdf" class="dida">
          <img class="skin" src="{$skin-img-dir}/pdfdoc.gif" alt="PDF"/><br/>
          PDF</a>
      </td>
    </xsl:if>
  </xsl:template>
  

  <!-- Generates the XML link -->
  <xsl:template name="xmllink">
    <xsl:if test="$disable-xml-link = 'false'">
      <td align="center" width="40" nowrap="nowrap"><a href="{$filename-noext}.xml" class="dida">
          <img class="skin" src="{$skin-img-dir}/xmldoc.gif" alt="xml"/><br/>
          xml</a>
      </td>
    </xsl:if>
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


  <!-- Generate a <a name="..."> tag for an @id -->
  <xsl:template match="@id">
    <xsl:if test="normalize-space(.)!=''">
      <a name="{.}"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="section">
    <!-- count the number of section in the ancestor-or-self axis to compute
         the title element name later on -->
    <xsl:variable name="sectiondepth" select="count(ancestor-or-self::section)"/>
    <a name="{generate-id()}"/>
    <xsl:apply-templates select="@id"/>
    <!-- generate a title element, level 1 -> h3, level 2 -> h4 and so on... -->
    <xsl:element name="{concat('h',$sectiondepth + 2)}">
      <xsl:value-of select="title"/>
      <xsl:if test="$notoc='true' and $sectiondepth = 3">
        <span style="float: right"><a href="#{@id}-menu">^</a></span>
      </xsl:if>
    </xsl:element>

    <!-- Indent FAQ entry text 15 pixels -->
    <xsl:variable name="indent">
      <xsl:choose>
        <xsl:when test="$notoc='true' and $sectiondepth = 3">
          <xsl:text>15</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>0</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <div style="margin-left: {$indent} ; border: 2px">
          <xsl:apply-templates select="*[not(self::title)]"/>
    </div>
  </xsl:template>

  <xsl:template match="note | warning | fixme">
    <xsl:apply-templates select="@id"/>
    <div class="frame {local-name()}">
      <div class="label">
        <xsl:choose>
          <xsl:when test="@label"><xsl:value-of select="@label"/></xsl:when>
          <xsl:when test="local-name() = 'note'">Note</xsl:when>
          <xsl:when test="local-name() = 'warning'">Warning</xsl:when>
          <xsl:otherwise>Fixme (<xsl:value-of select="@author"/>)</xsl:otherwise>
        </xsl:choose>
      </div>
      <div class="content">
        <xsl:apply-templates/>
      </div>
    </div>
  </xsl:template>

  <xsl:template match="notice">
    <div class="notice">
    <!-- FIXME: i18n Transformer here -->
    <xsl:text>Notice: </xsl:text>
      <xsl:apply-templates/>
    </div>
  </xsl:template>

  <xsl:template match="link">
    <xsl:apply-templates select="@id"/>
    <xsl:choose>
      <xsl:when test="$obfuscate-mail-links='true' and starts-with(@href, 'mailto:') and contains(@href, '@')">
        <xsl:variable name="mailto-1" select="substring-before(@href,'@')"/>
        <xsl:variable name="mailto-2" select="substring-after(@href,'@')"/>
          <a href="{$mailto-1}.at.{$mailto-2}">
            <xsl:apply-templates/>
          </a>
       </xsl:when>
       <xsl:otherwise>
          <a href="{@href}">
            <xsl:apply-templates/>
          </a>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="jump">
    <xsl:apply-templates select="@id"/>
    <a href="{@href}" target="_top">
      <xsl:apply-templates/>
    </a>
  </xsl:template>

  <xsl:template match="fork">
    <xsl:apply-templates select="@id"/>
    <a href="{@href}" target="_blank">
      <xsl:apply-templates/>
    </a>
  </xsl:template>

  <xsl:template match="p[@xml:space='preserve']">
    <xsl:apply-templates select="@id"/>
    <div class="pre">
      <xsl:apply-templates/>
    </div>
  </xsl:template>

  <xsl:template match="source">
    <xsl:apply-templates select="@id"/>
    <pre class="code">
<!-- Temporarily removed long-line-splitter ... gives out-of-memory problems -->
      <xsl:apply-templates/>
<!--
    <xsl:call-template name="format">
    <xsl:with-param select="." name="txt" /> 
     <xsl:with-param name="width">80</xsl:with-param> 
     </xsl:call-template>
-->
    </pre>
  </xsl:template>

  <xsl:template match="anchor">
    <a name="{@id}"/>
  </xsl:template>

  <xsl:template match="icon">
    <xsl:apply-templates select="@id"/>
    <img src="{@src}" alt="{@alt}" class="icon">
      <xsl:if test="@height">
        <xsl:attribute name="height"><xsl:value-of select="@height"/></xsl:attribute>
      </xsl:if>
      <xsl:if test="@width">
        <xsl:attribute name="width"><xsl:value-of select="@width"/></xsl:attribute>
      </xsl:if>
    </img>
  </xsl:template>

  <xsl:template match="code">
    <xsl:apply-templates select="@id"/>
    <span class="codefrag"><xsl:value-of select="."/></span>
  </xsl:template>

  <xsl:template match="figure">
    <xsl:apply-templates select="@id"/>
    <div align="center">
      <img src="{@src}" alt="{@alt}" class="figure">
        <xsl:if test="@height">
          <xsl:attribute name="height"><xsl:value-of select="@height"/></xsl:attribute>
        </xsl:if>
        <xsl:if test="@width">
          <xsl:attribute name="width"><xsl:value-of select="@width"/></xsl:attribute>
        </xsl:if>
      </img>
    </div>
  </xsl:template>

  <xsl:template match="table">
    <xsl:apply-templates select="@id"/>
    <table cellpadding="4" cellspacing="1" class="ForrestTable">
      <xsl:if test="@cellspacing"><xsl:attribute name="cellspacing"><xsl:value-of select="@cellspacing"/></xsl:attribute></xsl:if>
      <xsl:if test="@cellpadding"><xsl:attribute name="cellpadding"><xsl:value-of select="@cellpadding"/></xsl:attribute></xsl:if>
      <xsl:if test="@border"><xsl:attribute name="border"><xsl:value-of select="@border"/></xsl:attribute></xsl:if>
      <xsl:if test="@class"><xsl:attribute name="class"><xsl:value-of select="@class"/></xsl:attribute></xsl:if>
      <xsl:if test="@bgcolor"><xsl:attribute name="bgcolor"><xsl:value-of select="@bgcolor"/></xsl:attribute></xsl:if>
      <xsl:apply-templates/>
    </table>
  </xsl:template>

  <xsl:template match="acronym/@title">
    <xsl:attribute name="title">
      <xsl:value-of select="normalize-space(.)"/>
    </xsl:attribute>
  </xsl:template>

  <xsl:template name="minitoc">  
    <xsl:param name="tocroot"/>
    <xsl:param name="depth"/>     
    <xsl:if test="count($tocroot/section) > 0">
      <ul class="minitoc">
        <xsl:for-each select="$tocroot/section">
          <li>
            <xsl:call-template name="toclink"/>
            <xsl:if test="$depth&lt;$max-depth">
              <xsl:call-template name="minitoc">
                <xsl:with-param name="tocroot" select="."/>
                <xsl:with-param name="depth" select="$depth + 1"/>
              </xsl:call-template>
            </xsl:if>
          </li>
        </xsl:for-each>
      </ul>
    </xsl:if>
  </xsl:template>

  <xsl:template name="toclink">
    <xsl:variable name="tocitem" select="normalize-space(title)"/>
    <xsl:if test="string-length($tocitem)>0">
      <a>
        <xsl:attribute name="href">
          <xsl:text>#</xsl:text>
          <xsl:if test="@id">
            <xsl:value-of select="@id"/>
          </xsl:if>
        </xsl:attribute>
        <xsl:value-of select="$tocitem"/>
      </a>
    </xsl:if>
  </xsl:template>

  <xsl:template match="header/authors">
    <xsl:for-each select="person">
      <xsl:choose>
        <xsl:when test="position()=1">by&#160;</xsl:when>
        <xsl:otherwise>,&#160;</xsl:otherwise>
      </xsl:choose>
      <xsl:value-of select="@name"/>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="version">
    <span class="version">
      <xsl:apply-templates select="@major"/>
      <xsl:apply-templates select="@minor"/>
      <xsl:apply-templates select="@fix"/>
      <xsl:apply-templates select="@tag"/>
      <xsl:choose>
        <xsl:when test="starts-with(., '$Revision: ')">
          version <xsl:value-of select="substring(., 12, string-length(.) -11-2)"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="."/>
        </xsl:otherwise>
      </xsl:choose>
    </span>
  </xsl:template>
  
  <xsl:template match="@major">
     v<xsl:value-of select="."/>
  </xsl:template>
  
  <xsl:template match="@minor">
     <xsl:value-of select="concat('.',.)"/>
  </xsl:template>
  
  <xsl:template match="@fix">
     <xsl:value-of select="concat('.',.)"/>
  </xsl:template>
  
  <xsl:template match="@tag">
     <xsl:value-of select="concat('-',.)"/>
  </xsl:template>

  <xsl:template match="type">
    <p class="type">
    <!-- FIXME: i18n Transformer here -->
    <xsl:text>Type: </xsl:text>
    <xsl:value-of select="."/>
    </p>
  </xsl:template>

  <xsl:template match="abstract">
    <p>
      <xsl:apply-templates/>
    </p>
  </xsl:template>

  <xsl:template name="email">
    <a>
    <xsl:attribute name="href">
      <xsl:choose>
      <xsl:when test="$obfuscate-mail-links='true'">
        <xsl:variable name="user" select="substring-before(@email,'@')"/>
	<xsl:variable name="host" select="substring-after(@email,'@')"/>
	<xsl:value-of select="concat('mailto:',$user,'.at.',$host)"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="concat('mailto:',@email)"/>
      </xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
       <xsl:value-of select="@name"/>
    </a>
  </xsl:template>

  <xsl:template match="node()|@*" priority="-1">
    <xsl:copy>
      <xsl:apply-templates select="@*"/>
      <xsl:apply-templates/>
    </xsl:copy>
  </xsl:template>
  
</xsl:stylesheet>

