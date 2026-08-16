/**
 * Override leaflet WMS for custom get feature info
 */
import L from "leaflet"
import WMS from "leaflet.wms/dist/leaflet.wms.js"

var WmsSource = WMS.Source.extend({
  identify: function(evt) {
    if (this.options["activeTool"]() == "identify") {
      // Identify map features in response to map clicks. To customize this
      // behavior, create a class extending wms.Source and override one or
      // more of the following hook functions.

      var layers = this.getIdentifyLayers()
      if (!layers.length) {
        return
      }
      this.getFeatureInfo(evt.containerPoint, evt.latlng, layers, this.showFeatureInfo)
    }
  },
  // Overridden to avoid custom methods to be leaked into the URL
  createOverlay: function(untiled) {
    // Create overlay with all options other than untiled & identify
    var overlayOptions = {}
    for (var opt in this.options) {
      if (
        opt != "untiled" &&
        opt != "identify" &&
        // Added:
        opt != "activeTool" &&
        opt != "onGetFeatureInfo" &&
        opt != "onGetFeatureInfoStarted" &&
        opt != "onGetFeatureInfoParamsEnded" &&
        opt != "onError"
      ) {
        overlayOptions[opt] = this.options[opt]
      }
    }
    if (untiled) {
      return WMS.overlay(this._url, overlayOptions)
    } else {
      return WMS.tileLayer(this._url, overlayOptions)
    }
  },
  getFeatureInfo: function(point, latlng, layers, callback) {
    // Request WMS GetFeatureInfo and call callback with results
    // (split from identify() to faciliate use outside of map events)
    var params = this.getFeatureInfoParams(point, layers),
      url = this._url + L.Util.getParamString(params, this._url)

    this.showWaiting()
    this.ajax(url, done)

    function done(result) {
      this.hideWaiting()
      var text = this.parseFeatureInfo(result, url)
      callback.call(this, latlng, text)
    }
  },
  showFeatureInfo: function(latlng, info) {
    try {
      this.options["onGetFeatureInfo"](latlng, info)
    } catch (error) {
      this.options["onError"](error)
    }
  },
  showWaiting: function() {
    try {
      this.options["onGetFeatureInfoStarted"]()
    } catch (error) {
      // do nothing
    }
  },
  // Overridden to set info_format to json
  getFeatureInfoParams: function(point, layers) {
    // Hook to generate parameters for WMS service GetFeatureInfo request
    var wmsParams, overlay
    if (this.options.untiled) {
      // Use existing overlay
      wmsParams = this._overlay.wmsParams
    } else {
      // Create overlay instance to leverage updateWmsParams
      overlay = this.createOverlay(true)
      overlay.updateWmsParams(this._map)
      wmsParams = overlay.wmsParams
      wmsParams.layers = layers.join(",")
    }
    var infoParams = {
      request: "GetFeatureInfo",
      query_layers: layers.join(","),
      X: Math.round(point.x),
      Y: Math.round(point.y),
      info_format: "application/json",
      WITH_GEOMETRY: 1,
    }
    let result = L.extend({}, wmsParams, infoParams)
    try {
      result = this.options["onGetFeatureInfoParamsEnded"](result)
    } catch (error) {
      // Do nothing
    }
    return result
  },
})

export default {
  source(url, options) {
    return new WmsSource(url, options)
  },
}
