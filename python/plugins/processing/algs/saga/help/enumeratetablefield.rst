ENUMERATE TABLE FIELD
=====================

Description
-----------
This algorithm counts the number of unique values of a field in the input table. For this example table,

+--------+
|   ID   | 
+========+
| first  | 
+--------+
| first  | 
+--------+
| second | 
+--------+
| third  |   
+--------+
| third  |
+--------+
| third  |
+--------+

the resulting table would be:

+--------+---------+
|   ID   | ENUM_ID |
+========+=========+
| first  |    1    |
+--------+---------+
| first  |    1    |
+--------+---------+
| second |    2    | 
+--------+---------+
| third  |    3    |
+--------+---------+
| third  |    3    |
+--------+---------+
| third  |    3    |
+--------+---------+

Parameters
----------

- ``Input[Table]``: input table
- ``Attribute[TableField]``: field with values to be enumerated 

Outputs
-------

- ``Output[Table]``: the resulting table

See also
---------


Console usage
-------------


::

	processing.runalg('saga:enumeratetablefield', input, field, output)
