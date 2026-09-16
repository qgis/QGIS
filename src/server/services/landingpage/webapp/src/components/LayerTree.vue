/**
 *  Layer tree
 *
 *  Author:    elpaso@itopen.it
 *  Date:      2020-06-30
 *  Copyright: Copyright 2020, ItOpen
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 */
<template>
  <v-card flat class="mx-auto layertree-container" v-if="project">
    <v-card-text>
      <!-- OSM base map -->
      <div class="base-map">
        <div class="v-treeview-node">
          <v-icon color="light-green lighten-3">mdi-checkerboard</v-icon>
          <v-tooltip top>
            <template v-slot:activator="{ on }">
              <v-btn v-on="on" icon @click="toggleBaseMap('openstreetmap')">
                <v-icon
                  >mdi-checkbox-{{
                    baseMap == "openstreetmap" ? `marked` : `blank-outline`
                  }}</v-icon
                >
              </v-btn>
            </template>
            Toggle base map visibility
          </v-tooltip>
          <span class="group-title">OpenStreetMap</span>
        </div>
      </div>
      <!-- TOC tree -->
      <div
        id="layertree"
        v-for="(element, entry) in project.toc.children"
        :key="uniqueKey(entry)"
      >
        <LayerTreeNode
          :node="element"
          v-on:toggleLayer="toggleLayer"
          v-on:toggleGroup="toggleGroup"
        />
      </div>
    </v-card-text>
  </v-card>
</template>

<script>
import LayerTreeNode from "@/components/LayerTreeNode.vue";
import { v4 as uuidv4 } from 'uuid';
export default {
  name: "LayerTree",
  props: {
    projectId: String,
    project: {},
    drawer: null,
    map: null,
  },
  components: {
    LayerTreeNode,
  },
  data: function () {
    return {
      uniqueKey: function () {
        return uuidv4();
      },
    };
  },
  computed: {
    baseMap() {
      return this.$store.state.baseMap;
    },
  },
  methods: {
    /**
     * Find a layer node from typename and children
     */
    findLayerNode(typename, children) {
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
     * Find a node from tree id hash and children
     */
    findNodeByHash(tree_id_hash, children) {
      if (children) {
        for (let i = 0; i < children.length; ++i) {
          if (children[i].tree_id_hash == tree_id_hash) {
            return children[i];
          }
          let res = this.findNodeByHash(tree_id_hash, children[i].children);
          if (res) {
            return res;
          }
        }
      }
    },
    /**
     * Toggle a single layer  by tree id hash
     */
    toggleLayer(tree_id_hash) {
      let node = this.findNodeByHash(tree_id_hash, this.project.toc.children);
      if (node) {
        node.visible = !node.visible;
        this.$emit("setLayerVisibility", node.typename, node.visible);
      }
    },
    /**
     * Toggle a single basemap by name
     */
    toggleBaseMap(basemap) {
      if (this.baseMap == basemap) {
        basemap = "";
      }
      this.$store.commit("setBaseMap", basemap);
    },
    /**
     * Toggle a group by tree id hash
     */
    toggleGroup(tree_id_hash) {
      let node = this.findNodeByHash(tree_id_hash, this.project.toc.children);
      if (node) {
        this.setGroupNodeVisibility(node, !node.visible);
      }
    },
    /**
     * Recursively set a group node visibility
     */
    setGroupNodeVisibility(groupNode, visible) {
      groupNode.visible = visible;
      // Emit if it's a layer
      if (groupNode.is_layer) {
        this.$emit("setLayerVisibility", groupNode.typename, visible);
      }
      if (groupNode.children) {
        for (let i = 0; i < groupNode.children.length; ++i) {
          this.setGroupNodeVisibility(groupNode.children[i], visible);
        }
      }
    },
  },
};
</script>

<style>
.leaflet-top.layertree-toggle-container {
  top: 8.5rem;
}

#layertree-toggle {
  cursor: pointer;
}

ul.layer-group {
  padding-left: 1em;
}

.b-sidebar-body > div > ul.layer-group {
  padding-left: 0;
}

.v-navigation-drawer {
  z-index: 5000;
}
</style>
