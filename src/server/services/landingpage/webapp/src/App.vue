/**
 *  Main app wrapper
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
  <v-app id="app">
    <router-view></router-view>
  </v-app>
</template>

<script>
export default {
  name: "App"
};
</script>

<style>
.v-app-bar.v-app-bar--fixed,
.v-footer {
  z-index: 10000 !important;
}
</style>
