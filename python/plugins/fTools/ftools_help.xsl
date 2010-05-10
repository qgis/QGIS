<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/functions">

<html>
<head>
<title>fTools - Help</title>
<script language="JavaScript">
    function show_hide(id, show)
    {
	  if (el = document.getElementById(id))
	  {
	    if (null==show) show = el.style.display=='none';
  		el.style.display = (show ? '' : 'none');
	  }
    }

	function hide(id)
	{
	  if (el = document.getElementById(id))
	  {
		el.style.display = 'none'
	  }
	}
	
</script>
<style>
#function { 
 float:center;
 background-color: white;
 clear:both;
 display:block;
 padding:0 0 0.5em;
 margin:1em;
}

#head { 
  background-color: white;
  border-bottom-width:0;
  color: black;
  display:block;
  font-size:150%;
  font-weight:bold;
  margin:0;
  padding:0.3em 1em;
}

#description{ 
  display: block;
  float:left;
  width: auto;
  margin:0;
  text-align: left;
  padding:0.2em 0.5em 0.4em;
  color: black;
  font-size:100%;
  font-weight:normal;
 }

#category, #image, #long_description{ 
  font-size: 80%;
  padding: 0em 0em 0em 1em;
 }
</style>
<style type="text/css">
#analysis, #sampling, #geoprocessing, #management, #table, #home{
	background-color: white;
	text-align: left;
	font-size: 9pt;
}
#leftcontainer {
	float: left;
	position: relative;
	width: 300px;
	height: auto;
	margin-left: auto;
	margin-right: auto;
	text-align: left;
	overflow: hidden;
	padding-left: 0px;
	padding-top: 0px;
	background-color: white;
}

#pagecontainer {
	position: relative;
	width: auto;
	height: auto;
	margin-left: auto;
	margin-right: auto;
	text-align: left;
	overflow: hidden;
	padding-left: 0px;
	padding-top: 0px;
	background-color: white;
}

#navcontainer {
	float: left;
	position: relative;
	width: auto;
	height: 190px;
	margin-left: auto;
	margin-right: auto;
	text-align: left;
	overflow: hidden;
	padding-left: 0px;
	padding-top: 0px;
	background-color: white;
}

#logocontainer {
	float: left;
	position: relative;
	width: auto;
	height: 190px;
	margin-left: auto;
	margin-right: auto;
	text-align: left;
	overflow: hidden;
	padding-left: 15px;
	padding-top: 0px;
	background-color: white;
}

#contentcontainer {
	position: relative;
	float: left;
	width: 500px;
	height: auto;
	margin-left: auto;
	margin-right: auto; 
	color: #222222;
	text-align: left;
	font-size: 9pt;
	overflow: auto;
	background-color: white;
	padding-left: 10px;
	padding-right: 10px;
}

p.navbuttons a {
	color: #222222;
	text-decoration: none;
	background-color: none;
	display: block;
	padding: 3px 0px 3px 1.5px;
	width: 100%;
	border-top: none;
	border-right: none;
	border-bottom: none;
	border-left: none;
	background-repeat: no-repeat;
	font-size: 12pt;
} 

p.navbuttons a:hover, p.navbuttons a:active {
	background-color: gray;
	background-repeat: no-repeat;
	color: white;

</style>
</head>

<body>
<div id="pagecontainer">
  <div id="leftcontainer">
	<div id="logocontainer">
		<img src="icons/ftoolslogo.png" align="left"/>
	</div>
	<div id="navcontainer">
			<p class="navbuttons">
					<a href="javascript:show_hide('home', 'show'); javascript:hide('analysis'); javascript:hide('sampling');
					javascript:hide('geoprocessing'); javascript:hide('table'); javascript:hide('new');
					javascript:hide('management')">Home</a>
					<a href="javascript:hide('home'); javascript:show_hide('analysis', 'show'); javascript:hide('sampling');
					javascript:hide('geoprocessing'); javascript:hide('table'); javascript:hide('new');
					javascript:hide('management')">Analysis Tools</a>
					<a href="javascript:hide('home'); javascript:hide('analysis'); javascript:show_hide('sampling', 'show');
					javascript:hide('geoprocessing'); javascript:hide('table'); javascript:hide('new');
					javascript:hide('management')">Sampling Tools</a>
					<a href="javascript:hide('home'); javascript:hide('analysis'); javascript:hide('sampling');
					javascript:show_hide('geoprocessing', 'show'); javascript:hide('table'); javascript:hide('new');
					javascript:hide('management')">Geoprocessing Tools</a>
					<a href="javascript:hide('home'); javascript:hide('analysis'); javascript:hide('sampling');
					javascript:hide('geoprocessing'); javascript:show_hide('table', 'show'); javascript:hide('new');
					javascript:hide('management')">Geometry Tools</a>
					<a href="javascript:hide('home'); javascript:hide('analysis'); javascript:hide('sampling');
					javascript:hide('geoprocessing'); javascript:hide('table'); javascript:hide('new');
					javascript:show_hide('management', 'show')">Data Management Tools</a>
					<a href="javascript:show_hide('new', 'show'); javascript:hide('analysis'); javascript:hide('sampling');
					javascript:hide('geoprocessing'); javascript:hide('table');
					javascript:hide('management'); javascript:hide('home')">What's New</a>
			</p>
	</div>
  </div>
		<div id="contentcontainer">
			<div id="home" style="display:show" class="content-box">
  				<h1>fTools help</h1>
  					<h3> Note: To use fTools properly, you must remove all other versions of fTools 
  					from your /plugins directory</h3>
					<p>The goal of fTools is to provide a one-stop resource for many common
					 vector-based GIS tasks, without the need for additional software, libraries,
					 or complex workarounds.</p>
					<p>fTools is designed to extend the functionality of Quantum GIS using only 
					core QGIS and python libraries. It provides a growing suite of spatial data 
					management and analysis functions that are both quick and functional. In addition, 
					the geoprocessing functions of  Dr. Horst Duester and Stefan ZieglerI have been 
					incorporated to further facilitate and streamline GIS based research and analysis.</p>
					<p>I have made every attempt to use the most common GIS terms to describe the individual
					fTools functions. However, if you think the tools could be more useful under a different
					name or category, please email me with your suggestions. Indeed, if you would like 
					to report a bug, make suggestions for improving fTools,	or have a question about 
					the tools, please email me: carson.farmer@gmail.com</p>
					<p>I hope you enjoy,</p><p>Carson Farmer</p>
			</div>
			<div id="analysis" style="display:none" class="content-box">
  				<h1>Analysis Tools</h1>
					<xsl:for-each select="/functions/analysis/ftools_function" >
						<div id="function">
							<div id="head">
								<xsl:element name="img">
									<xsl:attribute name="src">
										<xsl:value-of select="image" />
									</xsl:attribute>
								</xsl:element>
								:
								<xsl:value-of select="name" />
							</div>
							<div id="description">
								<xsl:value-of select="description" />
							</div>
							<div id="long_description">
								<xsl:value-of select="long_description" />
							</div>
						</div>
					</xsl:for-each>
			</div>
			<div id="sampling" style="display:none" class="content-box">
  				<h1>Sampling Tools</h1>
					<xsl:for-each select="/functions/sampling/ftools_function">
						<div id="function">
							<div id="head">
								<xsl:element name="img">
									<xsl:attribute name="src">
										<xsl:value-of select="image" />
									</xsl:attribute>
								</xsl:element>
								:
								<xsl:value-of select="name" />
							</div>
							<div id="description">
								<xsl:value-of select="description" />
							</div>
							<div id="long_description">
								<xsl:value-of select="long_description" />
							</div>
						</div>
					</xsl:for-each>
			</div>
			<div id="geoprocessing" style="display:none" class="content-box">
  				<h1>Geoprocessing Tools</h1>
					<xsl:for-each select="/functions/geoprocessing/ftools_function">
						<div id="function">
							<div id="head">
								<xsl:element name="img">
									<xsl:attribute name="src">
										<xsl:value-of select="image" />
									</xsl:attribute>
								</xsl:element>
								:
								<xsl:value-of select="name" />
							</div>
							<div id="description">
								<xsl:value-of select="description" />
							</div>
							<div id="long_description">
								<xsl:value-of select="long_description" />
							</div>
						</div>
					</xsl:for-each>
			</div>
			<div id="table" style="display:none" class="content-box">
  				<h1>Geometry Tools</h1>
					<xsl:for-each select="/functions/geometry/ftools_function">
						<div id="function">
							<div id="head">
								<xsl:element name="img">
									<xsl:attribute name="src">
										<xsl:value-of select="image" />
									</xsl:attribute>
								</xsl:element>
								:
								<xsl:value-of select="name" />
							</div>
							<div id="description">
								<xsl:value-of select="description" />
							</div>
							<div id="long_description">
								<xsl:value-of select="long_description" />
							</div>
						</div>
					</xsl:for-each>
			</div>
			<div id="management" style="display:none" class="content-box">
  				<h1>Management Tools</h1>
					<xsl:for-each select="/functions/management/ftools_function">
						<div id="function">
							<div id="head">
								<xsl:element name="img">
									<xsl:attribute name="src">
										<xsl:value-of select="image" />
									</xsl:attribute>
								</xsl:element>
								:
								<xsl:value-of select="name" />
							</div>
							<div id="description">
								<xsl:value-of select="description" />
							</div>
							<div id="long_description">
								<xsl:value-of select="long_description" />
							</div>
						</div>
					</xsl:for-each>
			</div>
			<div id="new" style="display:none" class="content-box">
  				<h1>What's New</h1>
					<ul>
						<li>Inputs and outputs allowed to contain non-ascii characters</li>
						<li>User able to specify encoding style for all outputs</li>
						<li>Nearest neighbour analysis</li>
						<li>Mean coordinates</li>
						<li>New geometry menu item with several new geometry functions, including:
							<ul>
								<li>Check geometry validity</li>
								<li>Simplify</li>
								<li>Multipart to singleparts</li>
								<li>Singleparts to multipart</li>
								<li>Polygons to lines</li>
								<li>Export/add geometry info</li>
								<li>and more...</li>
							</ul>
						</li>

						<li>Several new geoprocessing functions, including:
							<ul>
								<li>Intersection</li>
								<li>Union</li>
								<li>Symmetrical difference</li>
								<li>Clip</li>
								<li>Difference</li>
								<li>Dissolve</li>
								<li>and more...</li>
							</ul>
						</li>
						<li>As well as several new bug fixes, corrections, and improvements</li>
					</ul>
			</div>
		</div>
</div>
</body>
</html>
</xsl:template>
</xsl:stylesheet>
