/**
 *  LayerTreeNode
 *
 * Author:    elpaso@itopen.it
 * Date:      2020-06-30
 * Copyright: Copyright 2020, ItOpen
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 */
<template>
  <div :id="node.tree_id_hash">
    <div
      class="v-treeview-node"
      v-if="node.is_layer && node.layer_type == 'vector'"
    >
      <div class="node-title">
        <v-icon
          data-toggle="collapse"
          :aria-expanded="node.expanded ? 'true' : 'false'"
          aria-controls="'node-' + node.tree_id_hash"
          @click="node.expanded = !node.expanded"
          v-if="node.children.length"
          >mdi-menu-{{ node.expanded ? `down` : `right` }}</v-icon
        >
        <v-tooltip top>
          <template v-slot:activator="{ on }">
            <v-btn v-on="on" icon @click="toggleLayer(node.tree_id_hash)">
              <v-icon
                >mdi-checkbox-{{
                  node.visible ? `marked` : `blank-outline`
                }}</v-icon
              >
            </v-btn>
          </template>
          Toggle layer visibility
        </v-tooltip>
        <v-tooltip top>
          <template v-slot:activator="{ on }">
            <span
              v-on="on"
              class="group-title"
              @click="node.expanded = !node.expanded"
              @contextmenu.prevent.stop="showContextMenu($event)"
              >{{ node.title }}</span
            >
          </template>
          <div>
            {{ node.title }}
            <i>({{ node.typename }})</i>
          </div>
          <div v-if="node.description">{{ node.description }}</div>
        </v-tooltip>
      </div>

      <template v-if="node.layer_type == 'vector' && node.children.length">
        <v-expand-transition>
          <div
            class="vector-legend"
            v-if="node.expanded"
            @contextmenu.prevent.stop="function () {}"
          >
            <div
              class="v-treeview-node layer-legend"
              v-for="child in node.children"
              :id="'node-' + node.tree_id_hash"
              :key="child.title + uuid()"
              :aria-expanded="node.expanded ? 'true' : 'false'"
            >
              <div
                v-if="isVisible(child)"
                class="v-treeview-node vector-legend-entry"
              >
                <img
                  v-if="child.icon"
                  class="symbol"
                  :src="`data:image/png;base64,${child.icon}`"
                />
                <span v-else class="symbol" />
                <v-tooltip top>
                  <template v-slot:activator="{ on }">
                    <span class="vector-legend-entry-label" v-on="on">{{
                      child.title
                    }}</span>
                  </template>
                  <div>{{ child.title }}</div>
                </v-tooltip>
              </div>
            </div>
          </div>
        </v-expand-transition>
      </template>
    </div>
    <div v-else>
      <!-- it's a group or a raster -->
      <v-icon
        @click="node.expanded = !node.expanded"
        v-if="node.layer_type != 'raster'"
        >mdi-menu-{{ node.expanded ? `down` : `right` }}</v-icon
      >
      <v-icon v-else color="light-green lighten-3">mdi-checkerboard</v-icon>
      <v-tooltip top>
        <template v-slot:activator="{ on }">
          <v-btn v-on="on" icon @click="toggleGroup(node.tree_id_hash)">
            <v-icon
              >mdi-checkbox-{{
                node.visible ? `marked` : `blank-outline`
              }}</v-icon
            >
          </v-btn>
        </template>
        Toggle group visibility
      </v-tooltip>
      <v-tooltip top>
        <template v-slot:activator="{ on }">
          <span
            v-on="on"
            class="group-title"
            @click="node.expanded = !node.expanded"
            @contextmenu.prevent.stop="function () {}"
            >{{ node.title }}</span
          >
        </template>
        <div>{{ node.title }}</div>
        <div v-if="node.description">{{ node.description }}</div>
      </v-tooltip>

      <v-expand-transition>
        <div
          :class="`group-container group-father-of-` + node.children.length"
          :id="'node-' + node.tree_id_hash"
          v-show="node.expanded"
        >
          <LayerTreeNode
            :node="child_node"
            v-on:toggleLayer="toggleLayer"
            v-on:toggleGroup="toggleGroup"
            v-for="child_node in node.children"
            :key="child_node.tree_id_hash"
          />
        </div>
      </v-expand-transition>
    </div>

    <!-- Context menu -->
    <v-menu
      :value="showMenu"
      :close-on-content-click="true"
      :close-on-click="false"
      :position-x="x"
      :position-y="y"
      absolute
      offset-y
    >
      <div @mouseleave="onMouseLeave">
        <v-list>
          <v-list-item
            v-for="item in options"
            :key="item.name"
            @click="onContextMenuOptionClicked(item.name, node.typename)"
          >
            <v-list-item-icon>
              <div v-text="item.icon"></div>
              <v-icon>{{item.icon}}</v-icon>
            </v-list-item-icon>
            <v-list-item-title>{{ item.title }}</v-list-item-title>
          </v-list-item>
        </v-list>
      </div>
    </v-menu>
  </div>
</template>

<script>
import { v4 as uuidv4 } from 'uuid';
export default {
  name: "LayerTreeNode",
  props: {
    node: {},
  },
  data() {
    return {
      x: 0,
      y: 0,
      // Trick to make sure menu appears on the first click! (see mounted)
      showMenu: true,
    };
  },
  mounted() {
    // Trick to make sure menu appears on the first click!
    this.showMenu = false;
  },
  methods: {
    uuid() {
      return uuidv4();
    },
    /**
     * Checks for legend item map scale
     */
    isVisible(child) {
      let scaleMaxDenom = child.scaleMaxDenom;
      let scaleMinDenom = child.scaleMinDenom;
      let mapScaleDenominator = this.$store.state.mapScaleDenominator;
      if (scaleMinDenom && mapScaleDenominator < scaleMinDenom) {
        return false;
      }
      if (scaleMaxDenom && mapScaleDenominator > scaleMaxDenom) {
        return false;
      }
      return true;
    },
    toggleLayer(tree_id_hash) {
      this.$emit("toggleLayer", tree_id_hash);
    },
    toggleGroup(tree_id_hash) {
      this.$emit("toggleGroup", tree_id_hash);
    },
    onContextMenuOptionClicked(name, typename) {
      //console.log("onContextMenuOptionClicked", name, typename);
      if (name == "attributes") {
        this.$store.commit("setAttributeTableLayerTypename", typename);
      } else {
        console.log("Menu item:", name, typename);
      }
    },
    showContextMenu(e) {
      e.preventDefault();
      this.showMenu = false;
      this.x = e.clientX;
      this.y = e.clientY;
      this.$nextTick(() => {
        this.showMenu = true;
      });
    },
    onMouseLeave() {
      this.showMenu = false;
    },
  },
  computed: {
    options() {
      let options = [];
      if (this.node["wfs_enabled"]) {
        options.push({
          title: "Attribute Table",
          name: "attributes",
          icon: "mdi-table-large",
        });
      }
      /* Not in scope for first release
      options.push({
        title: "Download",
        name: "download",
        icon: "mdi-download"
      });
      */
      return options;
    },
  },
};
</script>

<style scoped>
@import "https://stackpath.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css";

.group-title {
  cursor: pointer;
  overflow: hidden;
}

.group-container {
  margin-left: 2em;
}

.group-title,
.node-title,
.vector-legend-entry {
  white-space: nowrap;
}
.vector-legend-entry span {
  margin-left: 10px;
}
.vector-legend-entry img.symbol {
  vertical-align: middle;
  height: 16px;
  width: 16px;
}
.vector-legend-entry span.symbol {
  vertical-align: middle;
  display: inline-block;
  height: 16px;
  width: 16px;
  margin-left: 0;
}

.vector-legend {
  margin-left: 1.3em;
}
.vector-legend-entry-label {
  overflow: hidden;
  text-overflow: ellipsis;
}
.layer-legend {
  margin-left: 16px;
}
</style>