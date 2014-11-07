GRID CALCULATOR
===============

Description
-----------

The raster calculator allows the computation of algebraic expressions with raster layers, obtaining derived information in the form of new layers of geographical data

The following operators and functions are available:

- The following operators are available for the formula definition:
- + Addition
- - Subtraction
- * Multiplication
- / Division
- ^ power
- abs(a) - absolute value
- sqrt(a) - square root
- ln(a) - natural logarithm
- log(a) - base 10 logarithm
- exp(a) - exponential
- sin(a) - sine
- cos(a) - cosine
- tan(a) - tangent
- asin(a) - arcsine
- acos(a) - arccosine
- atan(a) - arctangent
- atan2(a, b) - arctangent of a/b
- gt(a, b) - if a>b the result is 1.0, else 0.0
- lt(a, b) - if a<b the result is 1.0, else 0.0
- eq(a, b) - if a=b the result is 1.0, else 0.0
- mod(a, b) - returns the floating point remainder of a/b
- ifelse(c, a, b) - if c=1 the result is a, else b
- int(a) - integer part of floating point value a
- pi() - returns the value of Pi


If when calculating the value of a given cell, a no-data value appears in any of the layers used in the formula, the resulting value will always be a no-data value.

Parameters
----------

- ``Grids[MultipleInput]``: The grids to use. They can be refered in the formula using letters (``a, b, c...``). The order for this naming is the same order as they appear in the selection window.
- ``Formula[String]``: The formula to use, which can use any of the expressions listeed above. 

Outputs
-------

- ``Result[Raster]``: The resulting layer.

Example
-------

You can for instance use the Grid Calculator module to flag 0 cells with -1:

``ifelse(eq(a,0), (-1), b)``

The formula reads like "if the cell is equal to zero, then write -1 else write the current value to the output grid".

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gridcalculator', grids, formula, use_nodata, result)
