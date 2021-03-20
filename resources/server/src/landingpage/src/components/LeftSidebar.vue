/**
 *  Left sidebar
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
  <v-navigation-drawer absolute hide-overlay stateless width="300px" v-model="drawer">
    <v-tabs class="mt-12 mb-10" grow v-model="activeTab">
      <v-tab key="0">Legend</v-tab>
      <v-tab key="1" v-if="showIdentify">Results</v-tab>

      <v-tab-item key="0">
        <LayerTree :project="project" v-on:setLayerVisibility="setLayerVisibility" :map="map" />
      </v-tab-item>

      <v-tab-item key="1" v-if="showIdentify">
        <IdentifyResults :map="map" />
      </v-tab-item>
    </v-tabs>
  </v-navigation-drawer>
</template>


<script>
import LayerTree from "@/components/LayerTree.vue";
import IdentifyResults from "@/components/IdentifyResults.vue";

export default {
  name: "LeftSidebar",
  components: {
    LayerTree,
    IdentifyResults
  },
  props: {
    drawer: null,
    project: null,
    showIdentify: null,
    map: null
  },
  data: function() {
    return {
      currentTab: 0
    };
  },
  methods: {
    /**
     * Forward to parent
     */
    setLayerVisibility(typename, visible) {
      this.$emit("setLayerVisibility", typename, visible);
    }
  },
  watch: {
    identifyResults() {
      this.currentTab = 1;
    }
  },
  computed: {
    activeTab: {
      get() {
        return this.currentTab;
      },
      set(newValue) {
        this.currentTab = newValue;
      }
    },
    identifyResults() {
      return this.$store.state.identifyResults;
    }
  }
};
</script>