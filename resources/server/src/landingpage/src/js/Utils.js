/**
 *  Shared functions for catalog and map view
 *
 *  Author:    elpaso@itopen.it
 *  Date:      2020-08-06
 *  Copyright: Copyright 2020, ItOpen
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 */

/**
 * Returns the root layer if any or all layers
 */
function getAllLayers(project) {
  let layers = project.wms_root_name
  if (!layers) {
    let _layers = []
    Object.values(project.wms_layers_map).forEach((layer_id) => _layers.push(layer_id))
    layers = _layers.join(`,`)
  }
  return layers
}

export default {
  getAllLayers,
}
