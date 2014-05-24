ADD POLYGON ATTRIBUTES TO POINTS
================================

Description
-----------
This algorithm adds the specified field of the polygons layer to the attribute table of the points layer in input. 
The new attributes added for each point depend on the value of the background polygon layer.

Parameters
----------

- ``Points[Vector]``: points layer
- ``Polygons[Vector]``: polygons layer
- ``Attribute[TableField]``: attribute of the polygons layer that will be added to the table attribute

Outputs
-------

- ``Result[Vector]``: resulting points layer with the new field

See also
---------


Console usage
-------------


::

	processing.runalg('saga:addpolygonattributestopoints', input, polygons, field, output)
