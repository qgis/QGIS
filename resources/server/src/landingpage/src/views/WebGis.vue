<template>
  <v-app id="project">
    <v-overlay light v-if="status == `loading` && error.length == 0">
      <v-progress-circular
        indeterminate
        color="lime"
        size="64"
      ></v-progress-circular>
    </v-overlay>
    <Error v-if="error.length > 0" :error="error" />
    <template v-else>
      <v-app-bar app dense collapse-on-scroll clipped-left color="green" dark>
        <v-app-bar-nav-icon
          @click.stop="expandedSidebar = !expandedSidebar"
        ></v-app-bar-nav-icon>
        <v-toolbar-title v-if="project">{{ project.title }}</v-toolbar-title>
        <v-spacer></v-spacer>
        <v-btn icon title="Home Page" to="/">
          <v-icon>mdi-home-circle</v-icon>
        </v-btn>
      </v-app-bar>
      <LeftSidebar
        :map="map"
        :project="project"
        :drawer="expandedSidebar"
        :showIdentify="showIdentify"
        v-on:setLayerVisibility="setLayerVisibility"
      />
      <v-main :class="attributeTableTypename ? `show-table` : ''">
        <v-container
          id="map"
          :class="
            `fill-height activetool-` +
            activeTool +
            (expandedSidebar ? ' expanded-sidebar' : '')
          "
          fluid
        >
          <v-layout>
            <l-map
              ref="map"
              v-resize="onResize"
              @ready="setMap"
              style="z-index: 0"
              maxZoom="22"
              :options="{attributionControl: false}"
            >
              <l-control-attribution
                position="bottomright"
                :options="{prefix: false}"
              ></l-control-attribution>
              <l-tile-layer
                :visible="baseMap == 'openstreetmap'"
                url="https://tile.openstreetmap.org/{z}/{x}/{y}.png"
                v-if="
                  project &&
                  project.capabilities.wmsOutputCrsList.includes('EPSG:3857')
                "
                attribution="&copy; &lt;a href='https://www.openstreetmap.org/copyright'&gt;OpenStreetMap&lt;/a&gt; contributors"
                :options="{maxZoom: 22, maxNativeZoom: 19}"
              ></l-tile-layer>
            </l-map>
          </v-layout>
        </v-container>
      </v-main>

      <AttributeTable
        :class="expandedSidebar ? 'mb-4 attributetable-small' : 'mb-4'"
        v-if="attributeTableTypename"
        :project="project"
        :map="this.map"
      />

      <MapToolbar class="map-toolbar" :map="map" :project="project" />

      <MapFooter />
    </template>
  </v-app>
</template>

<script>
import MapToolbar from "@/components/MapToolbar.vue";
import MapFooter from "@/components/MapFooter.vue";
import LeftSidebar from "@/components/LeftSidebar.vue";
import AttributeTable from "@/components/AttributeTable.vue";
import Error from "@/components/Error.vue";
import { LMap, LControlAttribution, LTileLayer } from "vue2-leaflet";
import WmsSource from "@/js/WmsSource.js";
import "leaflet/dist/leaflet.css";
import { latLng, Polygon } from "leaflet";
import L from "leaflet";
import Utils from "@/js/Utils.js";

// Patch for https://github.com/Leaflet/Leaflet/issues/4968#issuecomment-269750768
delete L.Icon.Default.prototype._getIconUrl;

L.Icon.Default.mergeOptions({
  iconRetinaUrl: require("leaflet/dist/images/marker-icon-2x.png"),
  iconUrl: require("leaflet/dist/images/marker-icon.png"),
  shadowUrl: require("leaflet/dist/images/marker-shadow.png"),
});
// end patch

L.Control.include({
  _refocusOnMap: L.Util.falseFn, // Do nothing.
});

export default {
  name: "WebGis",
  props: { projectId: String },
  components: {
    LMap,
    LControlAttribution,
    LTileLayer,
    MapToolbar,
    MapFooter,
    Error,
    LeftSidebar,
    AttributeTable,
  },
  data: function () {
    return {
      map: {},
      wms_source: {},
      expandedSidebar: false,
      // Whether the identify tab must be automatically shown
      showIdentify: false,
    };
  },
  computed: {
    project() {
      return this.$store.state.projects[this.projectId];
    },
    attributeTableTypename() {
      return this.$store.state.attributeTableTypename;
    },
    toc() {
      return this.$store.state.tocs[this.projectId];
    },
    status() {
      return this.$store.state.status;
    },
    error() {
      let error = this.$store.state.error;
      this.$store.commit("clearError");
      return error;
    },
    activeTool() {
      return this.$store.state.activeTool;
    },
    baseMap() {
      return this.$store.state.baseMap;
    },
  },
  watch: {
    project() {
      this.initializeMap();
    },
    toc() {
      this.initializeToc();
    },
  },
  mounted() {
    this.$store.commit("clearAttributeTableTypename");
    this.$store.dispatch("setStatus", `loading`);

    if (!this.project) {
      this.$store.dispatch("getProject", this.projectId);
    } else {
      //console.log("Project already loaded ...");
      this.setMap(this.$refs["map"].mapObject);
      this.initializeMap();
    }
  },
  methods: {
    setMap() {
      this.map = this.$refs["map"].mapObject;
    },
    /**
     * Called when project has been fetched
     */
    initializeMap() {
      //console.log(`Initializing map for project ${this.project.id}`);
      if (!this.project.toc) {
        //console.log(`initializeMap error: no project toc!`);
      }
      this.loadMap(this.project);
      Object.values(this.getOrderedLayerTypenames()).forEach((typename) => {
        if (!this.project.toc) {
          //console.log(`Loading layer ${title} failed: no toc!`);
        }
        let node = this.findLayerNodeByTypename(
          typename,
          this.project.toc.children
        );
        if (node && node.visible) {
          //console.log(`Loading layer ${typename}`);
          this.wms_source._subLayers[node.typename] = this.wms_source.getLayer(
            node.typename
          );
        } else if (!node) {
          console.log(`Could not find layer node: ${typename}`);
        } else if (!node.visible) {
          //console.log(`Not loading layer (not visible): ${typename}`);
        }
      });
      //console.log(this.wms_source._subLayers);
      this.wms_source.refreshOverlay();

      // Fetch TOC
      if (!this.toc) {
        let layers = Utils.getAllLayers(this.project);
        this.$store.dispatch("getToc", { projectId: this.projectId, layers });
      }

      this.$nextTick(() => {
        this.map.zoomControl.remove();
      });
      this.$store.dispatch("setStatus", `project`);
    },
    onResize() {
      this.$refs["map"].mapObject._onResize();
    },
    /**
     * Called when TOC was fetched
     */
    initializeToc() {
      this.toc.nodes.forEach((layer) => {
        let node = this.findLayerNode(layer.title, this.project.toc.children);
        if (node) {
          if (layer.icon) {
            node.children.push(layer);
          } else {
            layer.symbols.forEach((symbol) => node.children.push(symbol));
          }
        }
      });
    },
    /**
     * Toggles a layer by typename
     */
    setLayerVisibility(typename, visible) {
      if (typename in this.wms_source._subLayers && !visible) {
        //console.log(`Removing layer: ${typename}`);
        this.wms_source.removeSubLayer(typename);
      } else if (visible && !(typename in this.wms_source._subLayers)) {
        // We need to respect drawing order!
        this.wms_source._subLayers[typename] = this.wms_source.getLayer(
          typename
        );
        let new_sub_layers = {};
        for (const _type_name of this.getOrderedLayerTypenames()) {
          if (_type_name in this.wms_source._subLayers) {
            //console.log(`Adding layer: ${typename}`);
            new_sub_layers[_type_name] = this.wms_source._subLayers[_type_name];
          }
        }
        this.wms_source._subLayers = new_sub_layers;
        this.wms_source.refreshOverlay();
      } else {
        console.log(`Nothing to do for: ${typename} - ${visible}`);
      }
    },
    /**
     * Find a layer by title
     */
    findLayerNode(title, children) {
      if (children) {
        for (let i = 0; i < children.length; ++i) {
          if (children[i].title == title) {
            return children[i];
          }
          let res = this.findLayerNode(title, children[i].children);
          if (res) {
            return res;
          }
        }
      }
    },
    /**
     * Find a layer by typename
     */
    findLayerNodeByTypename(typename, children) {
      if (children) {
        for (let i = 0; i < children.length; ++i) {
          if (children[i].typename == typename) {
            return children[i];
          }
          let res = this.findLayerNode(typename, children[i].children);
          if (res) {
            return res;
          }
        }
      }
    },
    /**
     * Loads map
     */
    loadMap(project) {
      let west = project.geographic_extent[0];
      let south = project.geographic_extent[1];
      let east = project.geographic_extent[2];
      let north = project.geographic_extent[3];
      if (project.initial_extent) {
        west = project.initial_extent[0];
        south = project.initial_extent[1];
        east = project.initial_extent[2];
        north = project.initial_extent[3];
      }
      let p1 = new latLng(south, west);
      let p2 = new latLng(north, west);
      let p3 = new latLng(north, east);
      let p4 = new latLng(south, east);
      let polygonPoints = [p1, p2, p3, p4];
      let jl = new Polygon(polygonPoints, { fill: false }); // Don't: .addTo(this.map);
      this.map.setView(jl.getBounds().getCenter());
      if (
        jl.getBounds().getEast() != jl.getBounds().getWest() &&
        jl.getBounds().getNorth() != jl.getBounds().getSouth()
      ) {
        this.map.fitBounds(jl.getBounds());
      }
      let that = this;
      this.wms_source = WmsSource.source(`./project/` + project.id + `/?`, {
        tileSize: 512,
        transparent: true,
        format: "image/png",
        maxZoom: 22,
        dpi: window.devicePixelRatio * 96,
        onGetFeatureInfo: this.onGetFeatureInfo,
        onGetFeatureInfoStarted: this.onGetFeatureInfoStarted,
        onGetFeatureInfoParamsEnded: this.onGetFeatureInfoParamsEnded,
        onError: this.onError,
        activeTool() {
          return that.activeTool;
        },
      }).addTo(this.map);

      // Add an highlight layer to the map
      let highlightLayer = L.geoJson(
        { features: [] },
        {
          style: function (/* feature */) {
            return {
              weight: 2,
              opacity: 1,
              color: "yellow",
            };
          },
          onEachFeature: function (feature, layer) {
            layer.on({
              mouseover: function (e) {
                e;
                //console.log(e.target, feature);
              },
              mouseout: function (e) {
                e;
                //console.log(e.target);
              },
              click: function (e) {
                e;
                //console.log(e.target);
              },
            });
          },
        }
      ).addTo(this.map);
      this.map.highlightLayer = highlightLayer;

      L.control.scale().addTo(this.map);

      this.updateMapScale();
      this.map.on("move", this.updateMapScale, this);

      // For debugging and development: add llmap to window
      window.llmap = this.map;
    },
    /**
     * GFI results
     */
    onGetFeatureInfo(latLng, info) {
      this.$store.commit("setIdentifyResults", {
        identifyResults: JSON.parse(info),
      });
    },
    /**
     * GFI start: show sidebar and results tab
     */
    onGetFeatureInfoStarted() {
      this.map.highlightLayer.clearLayers();
      if (this.$store.state.activeTool == "identify") {
        this.showIdentify = true;
        this.$store.commit("clearIdentifyResults");
        this.expandedSidebar = true;
      } else {
        this.showIdentify = false;
      }
    },
    /**
     * Called to exclude layers not queryable
     */
    onGetFeatureInfoParamsEnded(result) {
      //console.log(result);
      let query_layers = result.query_layers.split(",");
      let queryable = [];
      for (let i = 0; i < query_layers.length; ++i) {
        if (
          this.project.wms_layers_queryable.includes(
            this.project.wms_layers_typename_id_map[query_layers[i]]
          )
        ) {
          queryable.push(query_layers[i]);
        }
      }
      result.query_layers = queryable;
      return result;
    },
    /**
     * Error handler for the map source WMS layer
     */
    onError(error) {
      console.log("Error:", error);
      this.$store.commit("setError", error.message);
    },
    /**
     * Convenience: return ordered list of layer typenames from the toc
     * the order is reversed, because of WMS drawing order
     */
    getOrderedLayerTypenames() {
      let _getLayers = (parent, layerList) => {
        if (parent.is_layer) {
          layerList.push(parent.typename);
        } else {
          for (let i = 0; i < parent.children.length; ++i) {
            _getLayers(parent.children[i], layerList);
          }
        }
      };
      let layerList = [];
      _getLayers(this.project.toc, layerList);
      //console.log(layerList)
      return layerList.reverse();
    },
    /**
     * Compute map scale
     */
    updateMapScale() {
      let map = this.map;
      let width = map.getSize().x;
      let bounds = map.getBounds();
      let extentHeight = bounds.getNorth() - bounds.getSouth();
      let minLon = bounds.getWest();
      let maxLon = bounds.getEast();
      let midLat = bounds.getSouth() + extentHeight / 2;
      let conversionFactor = 39.3700787;
      let dpi = 96 * window.devicePixelRatio; // this.$store.state.dpi;
      let dpm = dpi * conversionFactor;
      let physicalWidth = width / dpm;
      let delta = map.distance(
        L.latLng(midLat, minLon),
        L.latLng(midLat, maxLon)
      );
      let scaleDenom = delta / physicalWidth;
      this.$store.commit("setMapScaleDenominator", scaleDenom);
      return scaleDenom;
    },
  },
};
</script>

<style>
#wrapper {
  height: 100%;
}

.alert-danger {
  position: absolute;
  top: 4em;
  margin: 0 8em;
  z-index: 10000;
}

#map {
  padding: 0;
}

#map.activetool-identify .leaflet-grab {
  cursor: pointer !important;
}

.v-main.show-table {
  padding-bottom: 0 !important;
}
.map-toolbar {
  position: fixed;
  top: 90px;
  right: 30px;
}

.leaflet-container {
  background-color: white;
}

.attributetable-small {
  margin-left: 300px;
}

.expanded-sidebar .leaflet-left {
  left: 300px !important;
}
</style>
