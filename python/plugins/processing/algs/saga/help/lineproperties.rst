LINE PROPERTIES
===============

Description
-----------
This algorithm performs some simple calculation on each line of the layer and adds them as new field in the attribute table. 
You can decide to activate/deactivate every single option.   

Parameters
----------

- ``Lines[Vector]``: input lines layer
- ``Number of Parts[Boolean]``: number of parts of each line
- ``Number of Vertices[Boolean]``: number of vertices of each line
- ``Length[Boolean]``: length of each line (in meters)

Outputs
-------

- ``Lines with Property Attributes[Vector]``: resulting layer with the updated attribute table

See also
---------


Console usage
-------------


::

	processing.runalg('saga:lineproperties', lines, bparts, bpoints, blength, output)
