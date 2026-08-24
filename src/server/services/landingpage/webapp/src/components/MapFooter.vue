/**
 *  Map Footer
 *
 *  Author:    elpaso@itopen.it
 *  Date:      2020-10-28
 *  Copyright: Copyright 2020, ItOpen
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 */
<template>
<v-footer color="lime" app>
    <v-layout>
        <v-row no-gutters>
            <v-col align="left">
                <!-- your logo/ad here -->
            </v-col>
            <v-col align="right">
                <span>Scale 1:{{ this.mapScaleDenominator() }}</span>
            </v-col>
        </v-row>
    </v-layout>
</v-footer>
</template>

<script>
export default {
  name: "MapFooter",
  methods: {
      mapScaleDenominator() {
          return Math.round(this.$store.state.mapScaleDenominator).toLocaleString();
      }
  }
}
</script>