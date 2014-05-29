TRANSECT THROUGH POLYGON SHAPEFILE
==================================

Description
-----------

Transect for lines and polygon shapefiles The goal of this module is to create a transect along a line through a polygon map. Eg |____ST1_____!_ST2_!__ST1__!_______ST#_____| (Soil type 1 etc...) This is done by creating a table with the ID of each line, the distance to the starting point and the different transects:

::

|  line_id  |  start  |  end  |  code/field  |
|    0      |    0    |  124  |     ST1      |
|    0      |   124   |  300  |     ST2      |
|    0      |   300   | 1223  |     ST1      |
|    0      |  1223   | 2504  |     ST3      |
|    1      |    0    |  200  |     ST4      |
|   ...     |   ...   |  ...  |     ...      |

The module requires an input shape with all the line transects [Transect_Line] and a polygon theme [Theme]. You also have to select which field you want to have in the resulting table [Transect_Result]. This can be an ID of the polygon theme if you want to link the tables later on, or any other field [Theme_Field]. 

Parameters
----------

- ``Line Transect(s)[Vector]``:
- ``Theme[Vector]``:
- ``Theme Field[TableField]``:

Outputs
-------

- ``Result table[Table]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:transectthroughpolygonshapefile', transect, theme, theme_field, transect_result)
