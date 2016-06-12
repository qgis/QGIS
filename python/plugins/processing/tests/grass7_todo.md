# Here is a list of grass7 algorithms without test units

## Raster algorithms

* r.basins.fill
* r.carve: needs a vector input
* r.category
* r.category.out
* r.coin
* r.colors
* r.colors.out
* r.colors.stddev
* r.composite
* r.contour.level
* r.contour.step
* r.cost.coordinates
* r.cost.points
* r.cost.raster
* r.covar
* r.cross
* r.describe
* r.distance
* r.drain
* r.fill.dir
* r.fillnulls
* r.flow
* r.flow.aspect
* r.flow.aspect.barrier
* r.flow.barrier
* r.grow
* r.grow.distance
* r.gwflow
* r.his
* r.horizon
* r.horizon.height
* r.in.lidar
* r.in.lidar.info
* r.info
* r.kappa
* r.lake.coords
* r.lake.layer
* r.latlong
* r.li.cwed
* r.li.cwed.ascii
* r.li.dominance
* r.li.dominance.ascii
* r.li.edgedensity
* r.li.edgedensity.ascii
* r.li.mpa
* r.li.mpa.ascii
* r.li.mps
* r.li.mps.ascii
* r.li.padcv
* r.li.padcv.ascii
* r.li.padrange
* r.li.padrange.ascii
* r.li.padsd
* r.li.padsd.ascii
* r.li.patchdensity
* r.li.patchdensity.ascii
* r.li.patchnum
* r.li.patchnum.ascii
* r.li.pielou
* r.li.pielou.ascii
* r.li.renyi
* r.li.renyi.ascii
* r.li.richness
* r.li.richness.ascii
* r.li.shannon
* r.li.shannon.ascii
* r.li.shape
* r.li.shape.ascii
* r.li.simpson
* r.li.simpson.ascii
* r.mapcalc
* r.mask.rast
* r.mask.vect
* r.median
* r.mfilter
* r.mfilter.fp
* r.mode
* r.neighbors
* r.null
* r.out.gridatb
* r.out.ppm
* r.out.vrml
* r.param.scale
* r.patch
* r.profile
* r.quant
* r.quantile
* r.random
* r.random.cells
* r.random.raster
* r.random.surface
* r.reclass.area.greater
* r.reclass.area.lesser
* r.recode
* r.regression.line
* r.regression.multi
* r.relief
* r.relief.scaling
* r.report
* r.resamp.bspline
* r.resamp.filter
* r.resamp.interp
* r.resamp.rst
* r.resamp.stats
* r.resample
* r.rescale
* r.rescale.eq
* r.rgb
* r.ros: too much input rasters !
* r.series.accumulate: can't reproduce same results !
* r.series.interp: needs to handle output directories
* r.sim.sediment: too much input rasters !
* r.sim.water: too much input rasters !
* r.solute.transport: too much input rasters !
* r.spreadpath: segfaults with test dataset !
* r.stats.quantile.rast: needs to handle output directories
* r.sunhours: can't reproduce same results !
* r.support: create a new test for raster: test after metadata.
* r.surf.fractal: random results !
* r.surf.gauss: random results !
* r.surf.random: random results !
* r.terraflow: can't produce output with test dataset !
* r.texture: needs to handle output directories
* r.tile: needs to handle output directories
* r.tileset: can't reproduce same results
* r.to.vect: needs a vector output in GML
* r.topmodel: too much manual inputs.
* r.uslek: can't produce output with test dataset (needs reclassified float rasters) !
* r.viewshed: can't produce output with test dataset !
* r.volume: needs a vector output in GML

## Imagery algorithms

* i.albedo
* i.aster.toar
* i.cca
* i.colors.enhance
* i.eb.evapfr
* i.evapo.time
* i.fft
* i.his.rgb
* i.ifft
* i.in.spotvgt
* i.landsat.acca
* i.landsat.toar
* i.pca
* i.rectify
* i.tasscap
* i.topo.corr

## Vector algorithms

We need to handle gml for output and input for GRASS7 algorithms before creating tests !
