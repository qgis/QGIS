INVERSE DISTANCE WEIGHTED
=========================

Description
-----------
It uses a point shapes data layer to provide values for “control points”. 
The points are irregularly distributed. A numeric attribute is chosen from the attribute table 
linked to the shapes data layer. The attribute data are the data values for the control points.

Parameters
----------

- ``Points[Vector]``: 
	Points shapes data layer for input.
	
- ``Attribute[TableField]``: 
	Identifying the attribute of the point shapes data layer that will provide the numeric 
	data for the point objects
	
- ``Target Grid[Selection]``:
	Grid data layer that will contain the interpolated data values as grid cell data. 
	With “user defined” option (default) user must enter the target dimensions for the output 
	grid data layer. Other options are:
	- ‘Grid Size’: the length of one side of the grid cells defining the grid data layer.
	- ‘Fit Extent’: if the box is checked, the west-east and north-south boundaries of the generated grid 
	  data layer will be defined by the minimum and maximum x and y coordinates for the data points. 
	  When the box is un-checked, the coordinate values entered for the ‘X-Extent’ and ‘Y-Extent’ parameters 
	  will be used to define the boundaries for the generated grid data layer.
	- “grid project”:allows the user to choose an existing grid system. 
	- “grid”: it allows choosing a specific grid data layer to provide the target dimensions for the output 
	  grid data layer. The numeric data type used in the chosen grid data layer will be the same type used 
	  in the output grid data layer.
	
- ``Distance Weighting[Selection]``:
- ``Inverse Distance Power[Number]``:
	The degree of the equation used in the interpolation
	
- ``Exponential and Gaussian Weighting Bandwidth[Number]``:
- ``Search Range[Selection]``:
	The distance out from each known data point the algorithm will search to locate additional known data 
	points for use in the interpolation equation. The default is 100.
	
- ``Search Radius[Number]``:
- ``Search Mode[Selection]``:
	There are two search modes to choose. The default is “all directions”. The second choice is “quadrants”. 
	In this case, the search will be on a quadrant basis. When it finds the ‘Maximum Points’ in the quadrant, 
	it will move to the next point object.

- ``Number of Points[Selection]``:
- ``Maximum Number of Points[Number]``:
	Identifies how many points will be sought for use in the interpolation formula. 
	The default maximum number of points is 10.
	
- ``Output extent[Extent]``:
- ``Cellsize[Number]``:

Outputs
-------

- ``Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:inversedistanceweighted', shapes, field, target, weighting, power, bandwidth, range, radius, mode, points, npoints, output_extent, user_size, user_grid)

	Available options for selection parameters:

	target(Target Grid)
		0 - [0] user defined

	weighting(Distance Weighting)
		0 - [0] inverse distance to a power
		1 - [1] linearly decreasing within search radius
		2 - [2] exponential weighting scheme
		3 - [3] gaussian weighting scheme

	range(Search Range)
		0 - [0] search radius (local)
		1 - [1] no search radius (global)

	mode(Search Mode)
		0 - [0] all directions
		1 - [1] quadrants

	points(Number of Points)
		0 - [0] maximum number of points
		1 - [1] all points
