
Implements tiling of point clouds in two passes:
1. Read input files and write raw point data to files in a temporary directory
2. Write tiles as LAS/LAZ files from the temp point data files

The first pass is entirely based on untwine's "epf" implementation, with only
minor changes to grid/voxel structure to accommodate tiling requirements
(fixed tile edge size, only using X/Y dimensions for tile keys, single level).
Using commit `66cafb` as a base of the fork.

Single pass tiling can be done with "pdal tile" kernel, but it can easily run out
of open files (it keeps all output LAS/LAZ files open until it is finished).

License: GPL3+
