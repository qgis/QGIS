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

<!-- ====================================================================== -->
<!-- document section -->
<!-- ====================================================================== -->

 <xsl:template match="document">
  <!-- checks if this is the included document to avoid neverending loop -->
  <xsl:if test="not(book)">
      <document>
      <xsl:choose>
		<xsl:when test="header/title">
		      <title><xsl:value-of select="header/title"/></title>
		</xsl:when>
		<xsl:otherwise>
			<title>NO TITLE</title>
		</xsl:otherwise>
	</xsl:choose>
      <body>
        <xsl:apply-templates/>
        <xsl:if test="header/authors">
            <div align="right" id="authors">       
               <xsl:for-each select="header/authors/person">
                     <xsl:choose>
                        <xsl:when test="position()=1">by&#160;</xsl:when>

                        <xsl:otherwise>,&#160;</xsl:otherwise>
                     </xsl:choose>
                     <!-- <a href="mailto:{@email}"> -->
                      <xsl:value-of select="@name" />
                     <!-- </a> -->
                  </xsl:for-each>
              </div>
         </xsl:if>         
      </body>
      </document>
   </xsl:if>

  
    
   <xsl:if test="book">
    <xsl:apply-templates/>
   </xsl:if>
  </xsl:template>

   <xsl:template match="body">
    <xsl:apply-templates/>
  </xsl:template>
  
 
<!-- ====================================================================== -->
<!-- header section -->
<!-- ====================================================================== -->

 <xsl:template match="header">
  <!-- ignore on general document -->
 </xsl:template>

<!-- ====================================================================== -->
<!-- body section -->
<!-- ====================================================================== -->

   <xsl:template match="section">
	
	 <xsl:variable name = "level" select = "count(ancestor::section)+1" />
	 
	 <xsl:choose>
	 	<xsl:when test="$level=1">
	 	  <div class="h3"><h3><xsl:value-of select="title"/></h3></div>
	      <xsl:apply-templates/>
	 	</xsl:when>
	 	<xsl:when test="$level=2">
	 	  <div class="h4"><h4><xsl:value-of select="title"/></h4></div>
	      <xsl:apply-templates/>
	 	</xsl:when>
	 	<xsl:when test="$level=3">
	 	  <div class="h2"><h2><xsl:value-of select="title"/></h2></div>
	      <xsl:apply-templates/>
	 	</xsl:when>
	 	<xsl:otherwise>
	 	  <div class="h5"><h5><xsl:value-of select="title"/></h5></div>
	      <xsl:apply-templates/>	 	 
	 	</xsl:otherwise>
	 </xsl:choose>

	</xsl:template>  

 <xsl:template match="title">
 </xsl:template>
 	       
<!-- ====================================================================== -->
<!-- footer section -->
<!-- ====================================================================== -->

 <xsl:template match="footer">
  <!-- ignore on general documents -->
 </xsl:template>

<!-- ====================================================================== -->
<!-- paragraph section -->
<!-- ====================================================================== -->

  <xsl:template match="p">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="note">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="source">
    <xsl:apply-imports/>
  </xsl:template>
  
  <xsl:template match="//source/font">
    <font color="{@color}"><xsl:apply-templates/></font>
  </xsl:template>
    
  <xsl:template match="fixme">
    <xsl:apply-imports/>
  </xsl:template>

<!-- ====================================================================== -->
<!-- list section -->
<!-- ====================================================================== -->

 <xsl:template match="ul|ol|dl">
    <xsl:apply-imports/>
 </xsl:template>
 
 <xsl:template match="li">
    <xsl:apply-imports/>
 </xsl:template>

 <xsl:template match="sl">
    <xsl:apply-imports/>
 </xsl:template>

 <xsl:template match="dt">
    <xsl:apply-imports/>
 </xsl:template>

<!-- ====================================================================== -->
<!-- table section -->
<!-- ====================================================================== -->

  <xsl:template match="table">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="tr">
    <xsl:variable name="index"><xsl:number/></xsl:variable>
    <tr>
       <xsl:choose>
          <xsl:when test="($index mod 2) = 0">
             <xsl:attribute name="class">a</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
             <xsl:attribute name="class">b</xsl:attribute>
          </xsl:otherwise>
       </xsl:choose>
     
       <xsl:apply-templates/>
    </tr>
  </xsl:template>

  <xsl:template match="th">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="td">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="tn">
    <xsl:apply-imports/>
  </xsl:template>
  
  <xsl:template match="caption">
    <!-- ignore since already used -->
  </xsl:template>

<!-- ====================================================================== -->
<!-- markup section -->
<!-- ====================================================================== -->

 <xsl:template match="strong">
    <xsl:apply-imports/>
 </xsl:template>

 <xsl:template match="em">
    <xsl:apply-imports/>
 </xsl:template>

 <xsl:template match="code">
    <xsl:apply-imports/>
 </xsl:template>
 
<!-- ====================================================================== -->
<!-- images section -->
<!-- ====================================================================== -->

 <xsl:template match="figure">
    <xsl:apply-imports/>
 </xsl:template>
 
 <xsl:template match="img">
    <xsl:apply-imports/>
 </xsl:template>

 <xsl:template match="icon">
    <xsl:apply-imports/>
 </xsl:template>

<!-- ====================================================================== -->
<!-- links section -->
<!-- ====================================================================== -->

 <xsl:template match="link">
    <xsl:apply-imports/>
 </xsl:template>

 <xsl:template match="connect">
    <xsl:apply-imports/>
 </xsl:template>

 <xsl:template match="jump">
    <xsl:apply-imports/>
 </xsl:template>

 <xsl:template match="fork">
    <xsl:apply-imports/>
 </xsl:template>

 <xsl:template match="anchor">
    <xsl:apply-imports/>
 </xsl:template>  

<!-- ====================================================================== -->
<!-- specials section -->
<!-- ====================================================================== -->

 <xsl:template match="br">
    <xsl:apply-imports/>
 </xsl:template>

</xsl:stylesheet>
