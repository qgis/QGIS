function addComparison(id,rendered,expected,w,h) {
	var e = document.getElementById(id);

	var div = document.createElement("DIV");
	div.style.margin = "0 auto";
	div.style.fontSize = "0";
	e.appendChild(div);

	var div0 = document.createElement("DIV");
	div0.style.fontSize = "0";
	div0.style.position = "relative";
	div.appendChild(div0);

	var img = document.createElement("IMG");
	img.src = rendered;
	img.width = w;
	img.height = h;
	// img.style = "height: auto; width:100%";
	div0.appendChild(img);

	var div1 = document.createElement("DIV");
	div1.setAttribute("style", "pointer-events: none");
	div1.style.backgroundImage = "url('" + expected + "')";
	div1.style.backgroundSize = "cover";
	div1.style.position = "absolute";
	div1.style.left = 0;
	div1.style.top = 0;
	div1.style.height = "100%";
	div1.style.width = "50%";
	div1.style.fontSize = 0;
	div1.style.borderRight = "1px solid black";

	div0.appendChild(div1);

	var trackLocation = function(e) {
		var rect = img.getBoundingClientRect();
		var w = e.pageX - rect.left;
		if ( w <= img.offsetWidth ) {
		  div1.style.width = w;
		}
	}

	img.addEventListener( "mousemove", trackLocation, false );
	div0.addEventListener( "mousemove", trackLocation, false );
}
// get just the directory, not the .html file part
const match = unescape(window.location.href).match(/(file:\/\/\/.*\/)([^/]+)$/);
const localPath = match[1];

function updatePathsToLocalGit(val, localGitFolder) {
  return val.replace(/^.*\/tests\/testdata\//, localGitFolder + '/tests/testdata/');
}

function updateLocalGitFolder() {
  // Get the value from the input
  const localGitFolder = document.getElementById('localGitFolder').value;
  
  // Loop through all links on the page
  const links = document.querySelectorAll('a');
  links.forEach((link) => {
      // store original link href, then replace with path to local file
      const originalHref = link.originalHref ? link.originalHref : link.href;
      link.originalHref = originalHref;
      link.href = localGitFolder ? updatePathsToLocalGit(originalHref, localGitFolder) : originalHref;
  });

  // Loop through all code on the page
  const codes = document.querySelectorAll('code');
  codes.forEach((code) => {
      // store original code, then update with path to local file
      const originalCode = code.originalCode ? code.originalCode : code.innerHTML;
      code.originalCode = originalCode;
      if (localGitFolder)
      {
          const matchPaths = originalCode.match(/(.*")(.*?)" "(.*)"/);
          code.innerHTML = matchPaths[1] + updatePathsToLocalGit(matchPaths[2], localGitFolder) + '" "' + matchPaths[3].replace('/tmp', localPath) + '"';
      }
      else
      {
         code.innerHTML = originalCode;
      }
  });

  // Loop through all divs on the page
  const divs = document.querySelectorAll('div');
  divs.forEach((div) => {
      // Extract original style
      const originalStyle = div.originalStyle ? div.originalStyle : div.style.cssText;
      div.originalStyle = originalStyle;
      if (localGitFolder)
      {
          const matchPaths = originalStyle.match(/(.*url\(\"file:\/\/)(.*?)(\".*)/);
          if (matchPaths)
          {
              div.style.cssText = matchPaths[1] + updatePathsToLocalGit(matchPaths[2], localGitFolder) + matchPaths[3];
          }
      }
      else
      {
          div.style.cssText = originalStyle;
      }
  });
}
