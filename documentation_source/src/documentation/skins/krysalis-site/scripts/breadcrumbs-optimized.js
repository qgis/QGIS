var PREPREND_CRUMBS=new Array();
PREPREND_CRUMBS.push(new Array("Apache","http://www.apache.org/"));
PREPREND_CRUMBS.push(new Array("Jakarta","http://jakarta.apache.org/"));
var DISPLAY_SEPARATOR=" &gt; ";
var DISPLAY_PREPREND="";
var DISPLAY_POSTPREND=":";
var CSS_CLASS_CRUMB="breadcrumb";
var CSS_CLASS_TRAIL="breadcrumbTrail";
var CSS_CLASS_SEPARATOR="crumbSeparator";
var FILE_EXTENSIONS=new Array( ".html", ".htm", ".jsp", ".php", ".php3", ".php4" );
var PATH_SEPARATOR="/";

function sc(s) {
	var l=s.toLowerCase();
	return l.substr(0,1).toUpperCase()+l.substr(1);
}
function getdirs() {
	var t=document.location.pathname.split(PATH_SEPARATOR);
	var lc=t[t.length-1];
	for(var i=0;i < FILE_EXTENSIONS.length;i++)
	{
		if(lc.indexOf(FILE_EXTENSIONS[i]))
			return t.slice(1,t.length-1); }
	return t.slice(1,t.length);
}
function getcrumbs( d )
{
	var pre = "/";
	var post = "/";
	var c = new Array();
	if( d != null )
	{
		for(var i=0;i < d.length;i++) {
			pre+=d[i]+postfix;
			c.push(new Array(d[i],pre)); }
	}
	if(PREPREND_CRUMBS.length > 0 )
		return PREPREND_CRUMBS.concat( c );
	return c;
}
function gettrail( c )
{
	var h=DISPLAY_PREPREND;
	for(var i=0;i < c.length;i++)
	{
		h+='<a href="'+c[i][1]+'" >'+sc(c[i][0])+'</a>';
		if(i!=(c.length-1))
			h+=DISPLAY_SEPARATOR; }
	return h+DISPLAY_POSTPREND;
}

function gettrailXHTML( c )
{
	var h='<span class="'+CSS_CLASS_TRAIL+'">'+DISPLAY_PREPREND;
	for(var i=0;i < c.length;i++)
	{
		h+='<a href="'+c[i][1]+'" class="'+CSS_CLASS_CRUMB+'">'+sc(c[i][0])+'</a>';
		if(i!=(c.length-1))
			h+='<span class="'+CSS_CLASS_SEPARATOR+'">'+DISPLAY_SEPARATOR+'</span>'; }
	return h+DISPLAY_POSTPREND+'</span>';
}

if(document.location.href.toLowerCase().indexOf("http://")==-1)
	document.write(gettrail(getcrumbs()));
else
	document.write(gettrail(getcrumbs(getdirs())));

