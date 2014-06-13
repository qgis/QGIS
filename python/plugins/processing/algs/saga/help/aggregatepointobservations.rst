AGGREGATE POINT OBSERVATIONS
============================

Description
-----------

Parameters
----------

- ``Reference Points[Vector]``:
- ``ID[TableField]``:
- ``Observations[Table]``:
- ``X[TableField]``:
- ``Y[TableField]``:
- ``Track[TableField]``:
- ``Date[TableField]``:
- ``Time[TableField]``:
- ``Parameter[TableField]``:
- ``Maximum Time Span (Seconds)[Number]``:
- ``Maximum Distance[Number]``:

Outputs
-------

- ``Aggregated[Table]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:aggregatepointobservations', reference, reference_id, observations, x, y, track, date, time, parameter, eps_time, eps_space, aggregated)
