 /**
  *  Map toolbar
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
  <!--v-toolbar light floating-->
  <v-layout column>
    <v-btn class="btn-fix" dark fab small @click="map.zoomIn()" color="orange">
      <v-icon>mdi-plus</v-icon>
    </v-btn>
    <v-btn class="btn-fix mt-1" dark fab small @click="map.zoomOut()" color="orange">
      <v-icon>mdi-minus</v-icon>
    </v-btn>
    <v-btn
      class="btn-fix mt-3"
      v-bind:class="{ active: activeTool == `identify`}"
      :depressed="activeTool == `identify`"
      dark
      fab
      small
      color="blue"
      @click="onIdentifyToolClicked()"
    >
      <v-icon>mdi-information-outline</v-icon>
    </v-btn>
    <!-- not yet implemented:
    v-btn class="btn-fix mt-1" dark fab small @click="map._onResize()" color="blue">
      <v-icon>mdi-selection-drag</v-icon>
    </v-btn-->
  </v-layout>
  <!--/v-toolbar-->
</template>

<style lang="scss" scoped>
</style>

<script>
export default {
  name: "MapToolbar",
  props: {
    map: Object
  },
  methods: {
    onIdentifyToolClicked() {
      this.$store.state.activeTool =
        this.activeTool == "identify" ? "" : "identify";
      //console.log(this.$store.state.activeTool);
    }
  },
  computed: {
    activeTool() {
      return this.$store.state.activeTool;
    }
  }
};
</script>

<style scoped lang="scss">
:not(.active).btn-fix:focus::before {
  opacity: 0 !important;
}
.active::before {
  opacity: 0.24 !important;
}
</style>