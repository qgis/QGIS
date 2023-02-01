/**
 *  Error
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
  <v-card class="mx-auto" max-width="600">
    <v-card-title>
      <v-icon color="red" large>mdi-emoticon-sad</v-icon>Epic failure
    </v-card-title>
    <v-card-text>
      <v-alert type="error">{{ error }}</v-alert>
    </v-card-text>
  </v-card>
</template>

<script>
export default {
  props: {
    error: String
  }
};
</script>