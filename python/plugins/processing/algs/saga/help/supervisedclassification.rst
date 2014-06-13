SUPERVISED CLASSIFICATION
=========================

Description
-----------

Parameters
----------

- ``Grids[MultipleInput]``:
- ``Training Areas[Vector]``:
- ``Class Identifier[TableField]``:
- ``Method[Selection]``:
- ``Normalise[Boolean]``:
- ``Distance Threshold[Number]``:
- ``Probability Threshold (Percent)[Number]``:
- ``Probability Reference[Selection]``:
- ``Spectral Angle Threshold (Degree)[Number]``:

Outputs
-------

- ``Class Information[Table]``:
- ``Classification[Raster]``:
- ``Quality[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:supervisedclassification', grids, roi, roi_id, method, normalise, threshold_dist, threshold_prob, relative_prob, threshold_angle, class_info, classes, quality)

	Available options for selection parameters:

	method(Method)
		0 - [0] parallelepiped
		1 - [1] minimum distance
		2 - [2] mahalanobis distance
		3 - [3] maximum likelihood
		4 - [4] spectral angle mapping
		5 - [5] binary encoding

	relative_prob(Probability Reference)
		0 - [0] absolute
		1 - [1] relative
