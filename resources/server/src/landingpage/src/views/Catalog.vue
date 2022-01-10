<template>
  <v-app id="catalog">
    <v-overlay light v-if="status == `loading` && error.length == 0">
      <v-progress-circular
        indeterminate
        color="lime"
        size="64"
      ></v-progress-circular>
    </v-overlay>
    <v-app-bar app dense hide-on-scroll color="green" dark>
      <v-toolbar-title>QGIS Server Catalog</v-toolbar-title>
      <v-spacer></v-spacer>
    </v-app-bar>
    <v-main>
      <v-container id="catalog" class="fill-height" fluid v-if="catalog">
        <v-row align="center" v-if="error.length > 0 || status == `empty`">
          <v-col cols="12">
            <Error v-if="error.length > 0" :error="error" />
            <v-alert type="warning" v-if="status == `empty`">
              <h2>This QGIS Server catalog does not contain any project.</h2>
              <p>
                The projects (.QGS and .QGZ files) are searched in directories
                set by the environment variable
                <code>QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES</code>
                (multiple paths can be specified by joining them with
                <code>||</code>).
              </p>
              <p>
                Example:
                <br />
                <code
                  >QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES=/path/to/my/projects||/another_path/to/my/projects</code
                >
              </p>
              <p>
                Postgres projects are searched in the connections set by the
                environment variable
                <code>QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS</code>
                (multiple connections can be specified by joining them with
                <code>||</code>).
              </p>
              <p>
                Example:
                <br />
                <code
                  >QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS=postgresql://myusername:mypassword@myhost:myport?sslmode=disable&amp;dbname=mydatabase&amp;schema=public</code
                >
              </p>
            </v-alert>
          </v-col>
        </v-row>

        <template v-if="catalog">
          <v-card
            class="mx-auto mb-4"
            max-width="800"
            min-width="400"
            :key="project.identifier"
            v-for="project in catalog"
          >
            <l-map
              :ref="'mapid-' + project.id"
              @ready="loadMap(project, $event)"
              :options="{attributionControl: false}"
            >
              <l-control-attribution
                position="bottomright"
                :options="{prefix: false}"
              ></l-control-attribution>
              <l-tile-layer
                url="https://tile.openstreetmap.org/{z}/{x}/{y}.png"
                v-if="
                  project.capabilities.wmsOutputCrsList.includes('EPSG:3857')
                "
                attribution="&copy; &lt;a href='https://www.openstreetmap.org/copyright'&gt;OpenStreetMap&lt;/a&gt; contributors"
                :options="{maxZoom: 19}"
              ></l-tile-layer>
            </l-map>
            <v-card-title>{{ project.title }}</v-card-title>
            <v-card-subtitle class="description" v-if="project.description">{{
              project.description
            }}</v-card-subtitle>

            <v-card-actions>
              <v-dialog
                class="metadata"
                scrollable
                v-model="project.show"
                max-width="800px"
              >
                <template v-slot:activator="{ on }">
                  <v-btn color="orange" text v-on="on">
                    <v-icon>mdi-information</v-icon>Metadata
                  </v-btn>
                </template>
                <v-card>
                  <v-card-title>{{ project.title }}</v-card-title>
                  <v-divider></v-divider>
                  <v-card-text style="height: 300px">
                    <Metadata :project="project" />
                  </v-card-text>
                  <v-divider></v-divider>
                  <v-card-actions>
                    <v-btn
                      color="blue darken-1"
                      text
                      @click="project.show = false"
                      >Close</v-btn
                    >
                  </v-card-actions>
                </v-card>
              </v-dialog>

              <v-btn
                color="orange"
                text
                :to="{ name: 'map', params: { projectId: project.id } }"
              >
                <v-icon>mdi-map</v-icon>Browse
              </v-btn>
            </v-card-actions>
          </v-card>
        </template>
      </v-container>
    </v-main>
    <v-footer color="lime" app>
      <!-- your logo/ad here -->
    </v-footer>
  </v-app>
</template>

<script>
import { LMap, LControlAttribution, LTileLayer } from "vue2-leaflet";
import "leaflet/dist/leaflet.css";
import { latLng, Polygon } from "leaflet";
import WMS from "leaflet.wms/dist/leaflet.wms.js";
import Metadata from "@/components/Metadata.vue";
import Error from "@/components/Error.vue";
import Utils from "@/js/Utils.js";

export default {
  name: "Catalog",
  components: {
    LMap,
    LControlAttribution,
    LTileLayer,
    Metadata,
    Error,
  },
  computed: {
    status() {
      return this.$store.state.status;
    },
    catalog() {
      return this.$store.state.catalog;
    },
    error() {
      let error = this.$store.state.error;
      this.$store.commit("clearError");
      return error;
    },
  },
  created() {
    if (!this.catalog.length) {
      this.$store.dispatch("setStatus", `loading`);
      this.$store.dispatch("getCatalog");
    }
  },
  methods: {
    loadMap(project, map) {
      let west = project.geographic_extent[0];
      let south = project.geographic_extent[1];
      let east = project.geographic_extent[2];
      let north = project.geographic_extent[3];
      let p1 = new latLng(south, west);
      let p2 = new latLng(north, west);
      let p3 = new latLng(north, east);
      let p4 = new latLng(south, east);
      let polygonPoints = [p1, p2, p3, p4];
      let jl = new Polygon(polygonPoints, { fill: false }).addTo(map);
      map.setView(jl.getBounds().getCenter());
      if (
        jl.getBounds().getEast() != jl.getBounds().getWest() &&
        jl.getBounds().getNorth() != jl.getBounds().getSouth()
      ) {
        map.fitBounds(jl.getBounds());
      }

      WMS.overlay(`./project/${project.id}/?`, {
        layers: Utils.getAllLayers(project),
        transparent: true,
        format: "image/png",
        maxZoom: 19
      }).addTo(map);
    },
  },
};
</script>

<style scoped>
.leaflet-container {
  height: 20rem;
}
.card-footer .btn {
  margin-right: 0.5em;
}

h4.loading {
  margin-top: 0.35em;
}

.metadata {
  z-index: 1001;
}
.v-footer {
    height: 1.5rem;
}
</style>
