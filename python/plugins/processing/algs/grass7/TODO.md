TODO List for GRASS7 algorithms support into QGIS Processing

QGIS3 Processing Port
=====================

* Port to Python3.
  * print -> print(
  * unicode -> str
  * dict iteritems
* Replace all parameters by QgsProcessingParameters.
* Re-enable GRASS algorithm by default.
* Add GRASS 7.2 new algorithms.
* Improve unit tests
* GRASS_REGION_CELLSIZE_PARAMETER is integer or double?
* GRASS_SNAP_TOLERANCE_PARAMETER is integer or double?
* Do we need to use QgsProcessingParameters::parameterFromScriptCode for description files?
  We don't NEED but we have to improve getParameterFromString (or use an internal method) at least.
  There is also a problem for parameterFromScriptCode: it doesn't use description very well.
  We can also use parameterFromVariantMap and a custom internal Grass7Algorithm getParameterFromString.
* Make tests under MS-Windows 7 for Utf-8 support.

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
