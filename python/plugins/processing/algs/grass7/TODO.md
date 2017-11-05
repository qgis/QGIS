TODO List for GRASS7 algorithms support into QGIS Processing

QGIS3 Processing Port
=====================

* Things to do elsewhere
  * TODO We need Null QgsProcessingParameterNumber!
  * TODO We need NULL QgsProcessingParameterPoint!
  * TODO We need a QgsParameterMultipleInputLayers parameter for minimum and maximum number of layers.
  * TODO Open all the files in a QgsProcessingOutputFolder at the end of the algorithm.
* TODO Review all the methods of QgsProcessingAlgorithm.
* TODO Make tests under MS-Windows 7 for Utf-8 support.
  * DONE Algorithms can handle data with utf-8 in filepath.
  * TODO Support utf-8 profiles filepath.
* TODO Review Python3 port.
  * dict iteritems
* TODO Improve unit tests.
* TODO Use prepareAlgorithm for algorithm preparation.
* TODO Support ParameterTable.
* TODO Support multiple output vector formats.
  * TODO Try to use v.external.out on simple algorithms.
  * TODO Add an optional/advanced 'format option' textbox if vector output is detected.
* TODO Support multiple input vector formats
  * DONE create a general inputVectorLayer method.
  * TODO Support database connections.
  * TODO Support Auth API for databases connections.
  * TODO Some formats can't be correctly used by v.external:
    * GML.
  * TODO Build a workaround for those formats (use v.in.ogr).
* TODO Review all algorithm parameters.
  * MOD r.basins.fill
  * OK r.blend
  * OK r.buffer
  * OK r.buffer.lowmem
  * OK r.carve
  * OK r.category
  * MOD r.circle
  * MOD r.clump
  * OK r.coin
  * TODO r.colors OutputDirectory
  * OK r.colors.out
  * OK r.colors.stddev
  * OK r.composite
  * OK r.compress
  * MOD r.contour
  * MOD r.cost
  * OK r.covar
  * OK r.cross
  * r.describe
  * r.distance
  * r.drain
  * r.external
  * r.external.out
  * r.fill.dir
  * r.fillnulls
  * r.flow
  * r.grow.distance
  * r.grow
  * r.gwflow
  * r.his
  * r.horizon
  * r.import
  * r.in.ascii
  * r.in.aster
  * r.in.bin
  * r.in.gdal
  * r.in.gridatb
  * r.in.lidar
  * r.in.mat
  * r.in.png
  * r.in.poly
  * r.in.srtm
  * r.in.wms
  * r.in.xyz
  * r.info
  * r.kappa
  * r.lake
  * r.latlong
  * r.li.cwed
  * r.li.daemon
  * r.li.dominance
  * r.li.edgedensity
  * r.li
  * r.li.mpa
  * r.li.mps
  * r.li.padcv
  * r.li.padrange
  * r.li.padsd
  * r.li.patchdensity
  * r.li.patchnum
  * r.li.pielou
  * r.li.renyi
  * r.li.richness
  * r.li.shannon
  * r.li.shape
  * r.li.simpson
  * r.mapcalc
  * r.mask
  * r.mfilter
  * r.mode
  * r.neighbors
  * r.null
  * r.out.ascii
  * r.out.bin
  * r.out.gdal
  * r.out.gridatb
  * r.out.mat
  * r.out.mpeg
  * r.out.png
  * r.out.pov
  * r.out.ppm
  * r.out.ppm3
  * r.out.vrml
  * r.out.vtk
  * r.out.xyz
  * r.pack
  * r.param.scale
  * r.patch
  * r.plane
  * r.profile
  * r.proj
  * r.quant
  * r.quantile
  * r.random.cells
  * r.random
  * r.random.surface
  * r.reclass.area
  * r.reclass
  * r.recode
  * r.region
  * r.regression.line
  * r.regression.multi
  * r.relief
  * r.report
  * r.resamp.bspline
  * r.resamp.filter
  * r.resamp.interp
  * r.resamp.rst
  * r.resamp.stats
  * r.resample
  * r.rescale.eq
  * r.rescale
  * r.rgb
  * r.ros
  * r.series.accumulate
  * r.series
  * r.series.interp
  * r.shade
  * r.sim.sediment
  * r.sim.water
  * r.slope.aspect
  * r.solute.transport
  * r.spread
  * r.spreadpath
  * r.statistics
  * r.stats
  * r.stats.quantile
  * r.stats.zonal
  * r.stream.extract
  * r.sun
  * r.sunhours
  * r.sunmask
  * r.support
  * r.support.stats
  * r.surf.area
  * r.surf.contour
  * r.surf.fractal
  * r.surf.gauss
  * r.surf.idw
  * r.surf.random
  * r.terraflow
  * r.texture
  * r.thin
  * r.tile
  * r.tileset
  * r.timestamp
  * r.to.rast3
  * r.to.rast3elev
  * r.to.vect
  * r.topidx
  * r.topmodel
  * r.transect
  * r.univar
  * r.unpack
  * r.uslek
  * r.usler
  * r.viewshed
  * r.volume
  * r.walk
  * r.water.outlet
  * r.watershed
  * r.what.color
  * r.what

* TODO Convert all ext scripts.
  * TODO Review i.py.
  * TODO Force projection in description file?
  * r_rgb.py
  * r_blend_combine.py
  * r_blend_rgb.py
  * r_drain.py
  * r_horizon.py
  * r_mask.py
  * r_mask_vect.py
  * r_mask_rast.py
  * r_null.py
  * r_statistics.py
  * v_voronoi.py
  * v_build_polylines.py => TO delete.
  * v_in_geonames.py.
  * v_sample.py.
  * v_to_3d.py.
  * v_pack.py.
  * v_what_vect.py => TO delete.
  * v_what_rast_points.py.
  * v_what_rast_centroids.py.
  * v_vect_stats.py
  * v_rast_stats.py
  * v_net.py
  * v_net_alloc.py
  * v_net_allpairs.py
  * v_net_arcs.py
  * v_net_articulation.py
  * v_net_connect.py
  * v_net_connectivity.py
  * v_net_flow.py
  * v_net_iso.py
  * v_net_nodes.py
  * v_net_path.py
  * v_net_steiner.py
  * v_net_visibility.py
  
* DONE Support multiple output file raster formats.
  * DONE Add an optional/advanced 'format option' textbox if raster output is detected.
  * DONE Detext file format from extension.
  * DONE Improve GdalUtils to report raster formats that can be created with GDAL.
* DONE Add GRASS 7.2 new algorithms.
  * DONE Remove r.aspect => r.slope.aspect.
  * DONE Remove r.median.
  * DONE r.out.ascii.
  * DONE r.out.mat.
  * DONE r.out.mpeg.
  * DONE r.out.png.
  * DONE r.out.pop.
  * DONE r.out.ppm3.
  * DONE r.out.vtk.
  * DONE r.out.xyz.
  * DONE r.proj.
  * DONE r.stats.zonal.
  * DONE v.decimate.
  * DONE v.in.e00.
  * DONE v.proj.
* DONE Support QgsProcessingParameterRange (error in processing/gui/wrappers.py).
  * DONE implement a basic RangePanel/wrapper.
  * DONE Improve Wrapper logic for min/max.
* DONE Use some raster/vector layers with spacename into their path.
* DONE Use GRASS --exec instead of GRASS_BATCH_JOB.
* DONE Improve Grass Path and Binary detection for all OSs.
* DONE Replace all parameters by QgsProcessingParameters.
  * DONE Support multiple QgsProcessingParameterEnum.
  * DONE Review all ParameterFile
  * DONE Review all OutputDirectory.
    * DONE Convert all OutputDirectory to QgsProcessingParameterFolderDestination
    * DONE Default case:
      * Take the name of the output variable.
      * create a default value as basename.
      * export all layers into the directory with a shell loop.
    * DONE Remove all multipleOutputDir in ext/
    * r.colors: TODO ext | DONE desc | TODO tests.
    * r.texture: DONE ext | DONE desc | TODO tests.
    * r.stats.quantile: DONE ext | DONE desc | TODO tests.
    * r.series.interp: DONE ext | DONE desc | TODO tests.
    * r.mapcalc: DONE ext | DONE desc | TODO tests.
    * i.aster.toar: DONE ext | DONE desc | TODO tests.
    * i.tasscap: DONE ext | DONE desc | TODO tests.
    * i.rectify: DONE ext | DONE desc | TODO tests.
    * i.cca: DONE ext | DONE desc | TODO tests.
    * i.landsat.toar: DONE ext | DONE desc | TODO tests.
    * i.pca: DONE ext | DONE desc | TODO tests.
    * i.topo.corr: DONE ext | DONE desc | TODO tests.
  * DONE Review all OutputFile
    * DONE Replace by QgsProcessingParameterFileDestination
    * DONE QgsProcessingParameterFileDestination should use the file filter in Dialog.
      Replace fileOut with fileDestination in gui/ParametersUtils.py
* DONE Remove specific algorithms code in Grass7Algorithm.py (move them in ext).
* DONE Re-enable GRASS algorithm by default.
* DONE Support multiple bands input rasters.
* DONE Better support for files output that are HTML.
  * DONE All html output files will be report outputs.
  * DONE All html output will come as stdout files by default.
  * DONE OutputHtml must not be converted to OutputLayerDefinition.
  * DONE Convert false HTML files to real HTML files.
  * DONE Opens HTML files in Viewer.


Unit tests
==========

i.* modules:
------------

* i.albedo: needs better data
* i.aster.toar: needs OutputDir support in tests
* i.atcorr: OK (basic implementation)
* i.biomass: OK (basic implementation)
* i.cca: needs OutputDir support in tests
* i.cluster: OK (full implementation)
* i.colors.enhance: needs other raster data
* i.eb.eta: OK (basic implementation)
* i.eb.evapfr: needs better data
* i.eb.hsebal01: OK (basic implementation)
* i.eb.netrad: OK (basic implementation)
* i.eb.soilheatflux: OK (basic implementation)
* i.emissivity: OK (basic implementation)
* i.evapo.mh: OK (basic implementation)
* i.evapo.pm: OK (basic implementation)
* i.evapo.pt: OK (basic implementation)
* i.evapo.time: broken (don't know why, should work)
* i.fft: OK (full implementation)
* i.gensig: OK (full implementation)
* i.gensigset: OK (full implementation)
* i.group: OK (full implementation)
* i.his.rgb: needs better data
* i.ifft: needs specific raster data
* i.image.mosaic: OK (basic implementation)
* i.in.spotvgt: needs probably a true NVDI SPOT file (quite huge for tests).
* i.landsat.acca: needs better data
* i.landsat.toar: needs OutputDir support in tests
* i.maxlik: OK (full implementation)
* i.modis.qc: OK (full implementation)
* i.oif: OK (full implementation)
* i.ortho.camera: not implemented in Processing
* i.ortho.elev: not implemented in Processing
* i.ortho.rectify: not implemented in Processing
* i.pansharpen: OK (full implementation)
* i.pca: needs OutputDir support in tests
* i.rectify: needs OutputDir support in tests
* i.rgb.his: OK (full implementation)
* i.segment: OK (full implementation)
* i.smap: OK (full implementation)
* i.spectral: not implementable in Processing
* i.target: not implementable in Processing
* i.tasscap: needs OutputDir support in tests
* i.topo.corr.ill: OK (basic implementation)
* i.topo.corr: needs OutputDir support in tests
* i.vi: OK (basic implementation)
* i.zc: OK (basic implementation)

r.* modules
-----------

Need to write everything

v.* modules
-----------

Need to write everything

Other
=====

* TODO: decide what to do with nviz:
    nviz_cmd -> G7:m.nviz.image
