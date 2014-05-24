SHAPES BUFFER
=============

Description
-----------
This algorithm creates buffer(s) around features based on fixed distance or distance field.  

Parameters
----------

- ``Shapes[Vector]``: layer in input (polygons, lines or points)
- ``Buffer Distance[Selection]``: how the buffer(s) distance has to be calculated (``fixed value`` or ``attribute field``)
- ``Buffer Distance (Fixed)[Number]``: if fixed is activated, this is the buffer radius (in meters) 
- ``Buffer Distance (Attribute)[TableField]``: field distance of the attribute table (skip this if you have already chosen ``fixed value``)
- ``Scaling Factor for Attribute Value[Number]``: if you have chosen ``field distance``, set the scaling factor for the values. 
- ``Number of Buffer Zones[Number]``: set the number of buffer features  
- ``Circle Point Distance [Degree][Number]``: this option controls the smoothness of the buffer borders: great numbers means rough borders.
- ``Dissolve Buffers [Boolean]``: YES if you want to dissolve all buffers into one single feature, NO if you want to keep buffers separated

Outputs
-------

- ``Buffer[Vector]``: the resulting buffer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:shapesbuffer', shapes, buf_type, buf_dist, buf_field, buf_scale, buf_zones, dcircle, dissolve, buffer)

	Available options for selection parameters:

	buf_type(Buffer Distance)
		0 - [0] fixed value
		1 - [1] attribute field
