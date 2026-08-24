/**
 * Identify
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
  <v-container class="mx-auto">
    <template>
      <v-row v-if="identifyResults.features">
        <v-col class="col-auto">
          <v-card
            class="mx-auto mb-4"
            v-for="feature in identifyResults.features"
            :key="feature.id"
          >
            <v-card-title>
              <v-row>
                <v-col>{{ feature.id }}</v-col>
                <v-col class="col-auto" v-if="feature.geometry">
                  <v-tooltip top>
                    <template v-slot:activator="{ on }">
                      <v-btn v-on="on" icon @click="zoomToFeature(feature)">
                        <v-icon>mdi-magnify-plus</v-icon>
                      </v-btn>
                    </template>
                    Zoom to feature
                  </v-tooltip>
                </v-col>
              </v-row>
            </v-card-title>
            <v-simple-table dense>
              <template v-slot:default>
                <tbody>
                  <tr v-for="(value, name) in feature.properties" :key="name">
                    <th>{{ name }}</th>
                    <td>{{ value }}</td>
                  </tr>
                </tbody>
              </template>
            </v-simple-table>
          </v-card>
        </v-col>
      </v-row>
      <v-row v-else>
        <v-progress-linear indeterminate query></v-progress-linear>
      </v-row>
    </template>
  </v-container>
</template>

<script>
import reproject from "reproject";
export default {
  name: "IdentifyResults",
  props: {
    drawer: null,
    map: null
  },
  computed: {
    identifyResults() {
      //console.log("Computed", this.$store.state.identifyResults.features);
      return this.$store.state.identifyResults;
    }
  },
  methods: {
    zoomToFeature(feature) {
      this.map.highlightLayer.clearLayers();
      this.map.highlightLayer.addData(
        reproject.toWgs84(feature, this.map.options.crs.code)
      );
      this.map.setView(this.map.highlightLayer.getBounds().getCenter());
    }
  }
};
</script>