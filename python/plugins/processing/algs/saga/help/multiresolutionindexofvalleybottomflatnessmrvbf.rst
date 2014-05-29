MULTIRESOLUTION INDEX OF VALLEY BOTTOM FLATNESS (MRVBF)
=======================================================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Initial Threshold for Slope[Number]``:
- ``Threshold for Elevation Percentile (Lowness)[Number]``:
- ``Threshold for Elevation Percentile (Upness)[Number]``:
- ``Shape Parameter for Slope[Number]``:
- ``Shape Parameter for Elevation Percentile[Number]``:
- ``Update Views[Boolean]``:
- ``Classify[Boolean]``:
- ``Maximum Resolution (Percentage)[Number]``:

Outputs
-------

- ``MRVBF[Raster]``:
- ``MRRTF[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:multiresolutionindexofvalleybottomflatnessmrvbf', dem, t_slope, t_pctl_v, t_pctl_r, p_slope, p_pctl, update, classify, max_res, mrvbf, mrrtf)
