function addComparison(id,rendered,expected,w,h) {
	var e = document.getElementById(id);

	var div = document.createElement("DIV");
	div.style = "margin: 0 auto; font-size: 0;";
	e.appendChild(div);

	var div0 = document.createElement("DIV");
	div0.style = "font-size: 0; position: relative;";
	div.appendChild(div0);

	var img = document.createElement("IMG");
	img.src = rendered;
	img.width = w;
	img.height = h;
	// img.style = "height: auto; width:100%";
	div0.appendChild(img);

	var div1 = document.createElement("DIV");
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
