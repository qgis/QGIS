/**
 *  Attribute table
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
  <v-card level="2">
    <v-card-text>
      <v-btn class="btn-close" icon @click="onCloseButtonClicked">
        <v-icon>mdi-close</v-icon>
      </v-btn>
      <v-card-title>
        {{ title }}
        <v-spacer></v-spacer>
        <v-text-field
          v-model="filterText"
          append-icon="mdi-magnify"
          label="Filter"
          hint="Case sensitive, use * to match any character"
          dense
          :error="this.hasSearchError"
          single-line
          hide-details
        ></v-text-field>
        <v-spacer></v-spacer>
        <v-combobox
          v-model="filterField"
          :items="searchableFields"
          label="Search field ..."
          no-filter
          hide-details
          dense
        ></v-combobox>
      </v-card-title>

      <v-data-table
        dense
        item-key="itemKeyInternalIdentifier"
        :page.sync="currentPage"
        :sort-by.sync="sortBy"
        :sort-desc.sync="sortDesc"
        :server-items-length="numberMatched"
        no-data-text="Attribute table has no data, search is case-sensitive, use * to match any character."
        :loading="tableHeaders.length == 0 || loading"
        :headers="tableHeaders"
        :items="tableData"
        :items-per-page="5"
        :footer-props="{
        itemsPerPageOptions: [5],
        itemsPerPageText: ''
      }"
      >
        <template v-slot:item.zoomToFeature="{ item }">
          <v-icon @click="zoomToFeature(item.feature)">mdi-magnify</v-icon>
        </template>
      </v-data-table>
    </v-card-text>
  </v-card>
</template>

<script>
import { v4 as uuidv4 } from 'uuid';
export default {
  name: "AttributeTable",
  props: {
    project: null,
    map: null
  },
  computed: {
    /**
     * Layer identifier for WFS3: layer id or short name or name
     */
    typename() {
      return this.$store.state.attributeTableTypename;
    },
    /**
     * Get layer name from typename
     */
    title() {
      return Object.keys(this.project.wms_layers_map).find(
        key => this.project.wms_layers_map[key] === this.typename
      );
    },
    searchableFields() {
      let layerId = this.project.wms_layers_typename_id_map[this.typename];
      let values = [];
      let fieldNames = Object.keys(this.project.wms_layers[layerId]["fields"]);
      for (let i = 0; i < fieldNames.length; i++) {
        let field = this.project.wms_layers[layerId]["fields"][fieldNames[i]];
        values.push({
          text: field["label"],
          value: fieldNames[i]
        });
      }
      return values;
    },
    fieldAliases() {
      let layerId = this.project.wms_layers_typename_id_map[this.typename];
      let aliases = {};
      let fieldNames = Object.keys(this.project.wms_layers[layerId]["fields"]);
      for (let i = 0; i < fieldNames.length; i++) {
        let field = this.project.wms_layers[layerId]["fields"][fieldNames[i]];
        aliases[fieldNames[i]] = field["label"];
      }
      return aliases;
    },
    hasSearchError() {
      return this.error > 0 && this.filterText.length;
    }
  },
  data() {
    return {
      error: null,
      currentPage: 1,
      sortBy: null,
      sortDesc: null,
      tableData: [],
      tableHeaders: [],
      numberMatched: 0,
      filterField: null,
      filterText: "",
      loading: false
    };
  },
  mounted() {
    this.loadData();
  },
  watch: {
    currentPage() {
      this.loadData();
    },
    sortBy() {
      this.loadData();
    },
    sortDesc() {
      this.loadData();
    },
    typename() {
      this.loadData();
    },
    filterField() {
      if (this.filterText) this.loadData();
    },
    filterText() {
      if (!this.filterField) {
        this.filterField = this.searchableFields[0];
      } else {
        this.loadData();
      }
    }
  },
  methods: {
    onCloseButtonClicked() {
      this.$store.commit("clearAttributeTableTypename");
    },
    /**
     * Load table data from WFS3
     */
    async loadData() {
      try {
        this.error = null;
        this.loading = true;
        let offset = (this.currentPage - 1) * 5;
        let sorting = "";
        if (this.sortBy) {
          sorting = "&sortby=" + encodeURIComponent(this.sortBy);
          if (this.sortDesc) {
            sorting += "&sortdesc=1";
          }
        }
        let filter = "";
        if (this.filterField && this.filterText) {
          filter = `&${this.filterField.value}=${this.filterText}`;
        }
        fetch(
          `./project/${this.project.id}/wfs3/collections/${this.typename}/items.json?limit=5&offset=${offset}${sorting}${filter}`
        )
          .then(response => {
            if (!response) {
              throw Error(
                `Error fetching attribute table data from QGIS Server`
              );
            }
            if (!response.ok) {
              throw Error(response.statusText);
            }
            return response;
          })
          .then(response => response.json())
          .then(json => {
            if (json.features.length) {
              let headers = [{ text: "", value: "zoomToFeature" }];
              for (let k in json.features[0].properties) {
                headers.push({
                  text: k,
                  value: this.fieldAliases[k] ? this.fieldAliases[k] : k
                });
              }
              this.tableHeaders = headers;
              let data = [];
              for (let i = 0; i < json.features.length; i++) {
                let dataRow = json.features[i].properties;
                dataRow["feature"] = json.features[i];
                dataRow["itemKeyInternalIdentifier"] = uuidv4();
                data.push(dataRow);
              }
              this.tableData = data;
              this.numberMatched = json.numberMatched;
            } else {
              this.tableData = [];
            }
          })
          .catch(error => {
            this.error = error.message;
            this.tableData = [];
          });
      } catch (error) {
        this.error = error.message;
        this.tableData = [];
      }
      this.loading = false;
    },
    zoomToFeature(feature) {
      this.map.highlightLayer.clearLayers();
      this.map.highlightLayer.addData(feature);
      this.map.setView(this.map.highlightLayer.getBounds().getCenter());
    }
  }
};
</script>

<style scoped>
.btn-close {
  float: right;
}
</style>
