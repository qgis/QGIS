/*
 * The MIT License

Copyright (c) 2010 by Juergen Marsch

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

(function ($)
{	 var options =
     {	series: {
	 	grow: {
	 		active: true,
			valueIndex: 1,
	 		stepDelay: 20,
			steps:100,
	 		stepMode: "linear",
			stepDirection: "up"
	 	}
	 }
    };
    function init(plot) {
		var done = false;
		var growfunc;
		var plt = plot;
		var data = null;
		var opt = null;
		var valueIndex;
		plot.hooks.bindEvents.push(processbindEvents);
		plot.hooks.drawSeries.push(processSeries);
		function processSeries(plot, canvascontext, series)
		{   opt = plot.getOptions();
		    valueIndex = opt.series.grow.valueIndex;
			if(opt.series.grow.active == true)
			{	if (done == false)
				{	data = plot.getData();
					data.actualStep = 0;
					for (var j = 0; j < data.length; j++)
					{	data[j].dataOrg = clone(data[j].data);
						for (var i = 0; i < data[j].data.length; i++)
							data[j].data[i][valueIndex] = 0;
					}
					plot.setData(data);
					done = true;
				}
			}
		}
		function processbindEvents(plot,eventHolder)
		{	opt = plot.getOptions();
			if (opt.series.grow.active == true)
			{	var d = plot.getData();
				for (var j = 0; j < data.length; j++) {
					opt.series.grow.steps = Math.max(opt.series.grow.steps, d[j].grow.steps);
				}
				if(opt.series.grow.stepDelay == 0) opt.series.grow.stepDelay++;
				growfunc = window.setInterval(growing, opt.series.grow.stepDelay);
			}
		}
		function growing()
		{	if (data.actualStep < opt.series.grow.steps)
			{	data.actualStep++;
				for(var j = 0; j < data.length; j++)
				{ if (typeof data[j].grow.stepMode == "function")
					{	data[j].grow.stepMode(data[j],data.actualStep,valueIndex); }
					else
					{	if (data[j].grow.stepMode == "linear") growLinear();
						else if (data[j].grow.stepMode == "maximum") growMaximum();
						else if (data[j].grow.stepMode == "delay") growDelay();
						else growNone();
					}
				}
				plt.setData(data);
				plt.draw();
			}
			else
			{	window.clearInterval(growfunc); }
			function growNone()
			{	if (data.actualStep == 1)
				{	for (var i = 0; i < data[j].data.length; i++)
					{	data[j].data[i][valueIndex] = data[j].dataOrg[i][valueIndex]; }
				}
			}
			function growLinear()
			{	if (data.actualStep <= data[j].grow.steps)
				{	for (var i = 0; i < data[j].data.length; i++)
					{	if (data[j].grow.stepDirection == "up")
						{	data[j].data[i][valueIndex] = data[j].dataOrg[i][valueIndex] / data[j].grow.steps * data.actualStep;}
						else if(data[j].grow.stepDirection == "down")
						{	data[j].data[i][valueIndex] = data[j].dataOrg[i][valueIndex] + (data[j].yaxis.max - data[j].dataOrg[i][valueIndex]) / data[j].grow.steps * (data[j].grow.steps - data.actualStep); }
					}
				}
			}
			function growMaximum()
			{	if (data.actualStep <= data[j].grow.steps)
				{	for (var i = 0; i < data[j].data.length; i++)
					{	if (data[j].grow.stepDirection == "up")
						{	data[j].data[i][valueIndex] = Math.min(data[j].dataOrg[i][valueIndex], data[j].yaxis.max / data[j].grow.steps * data.actualStep); }
						else if (data[j].grow.stepDirection == "down")
						{	data[j].data[i][valueIndex] = Math.max(data[j].dataOrg[i][valueIndex], data[j].yaxis.max / data[j].grow.steps * (data[j].grow.steps - data.actualStep) ); }
					}
				}
			}
			function growDelay()
			{	if (data.actualStep == data[j].grow.steps)
				{	for (var i = 0; i < data[j].data.length; i++)
					{	data[j].data[i][valueIndex] = data[j].dataOrg[i][valueIndex]; }
				}
			}
		}
		function clone(obj)
		{	if(obj == null || typeof(obj) != 'object') return obj;
			var temp = new obj.constructor();
			for(var key in obj)	temp[key] = clone(obj[key]);
			return temp;
		}
    }

	$.plot.plugins.push({
      init: init,
      options: options,
      name: 'grow',
      version: '0.2'
    });
})(jQuery);
