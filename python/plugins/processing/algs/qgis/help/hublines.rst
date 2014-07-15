HUB LINES
=========

Description
-----------
This algorithm connects with a line layer 2 points layers. You have to specify the common field of the attribute tables.
For example, you can connect 2 points layers that have ID field in common. 

Parameters
----------

- ``Hub Point Layer[Vector]``: first points layer
- ``Hub ID Attribute[TableField]``: attribute field of the first layer
- ``Spoke Point Layer[Vector]``: second points layer
- ``Spoke Hub ID Attribute[TableField]``: attribute field of the second layer

Outputs
-------

- ``Output[Vector]``: the resulting lines layer 

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:hublines', hubname, hubattribute, spokename, spokeattribute, savename)
