/**
 *  Metadata
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
  <v-tabs>
    <v-tab :href="`#general-${project.id}`">General</v-tab>
    <v-tab-item :value="`general-${project.id}`">
      <v-card flat tile>
        <v-card-text>
          <template v-for="(value, entry) in project.metadata">
            <template v-if="entry == 'categories' || entry == 'history'">
              <template v-if="value.length">
                <dt :key="uniqueKey(entry + 'k')">{{ entry|camelTitle }}</dt>
                <dd :key="uniqueKey(entry + 'v')">
                  <ul v-for="item in value" :key="uniqueKey(item)">
                    <li :key="uniqueKey(item)">{{ item }}</li>
                  </ul>
                </dd>
              </template>
            </template>

            <template v-else-if="entry != 'links' && entry != 'contacts' && value ">
              <dt :key="uniqueKey(entry)">{{ entry|camelTitle }}</dt>
              <dd :key="uniqueKey(value)">{{ value }}</dd>
            </template>
          </template>
        </v-card-text>
      </v-card>
    </v-tab-item>

    <template v-if="project.metadata.contacts.length > 0">
      <v-tab :href="`#contacts-${project.id}`">Contacts</v-tab>

      <v-tab-item :value="`contacts-${project.id}`" v-if="project.metadata.contacts">
        <v-card flat tile>
          <v-card-text>
            <template v-for="contact in project.metadata.contacts">
              <h5 :key="uniqueKey(contact.name)">{{ contact.name }}</h5>
              <dl :key="uniqueKey(contact.name)">
                <template v-for="(cv, ce) in contact">
                  <template v-if="ce != 'addresses' && ce != 'name' && cv ">
                    <dt :key="uniqueKey(ce)">{{ ce|camelTitle }}</dt>
                    <dd :key="uniqueKey(cv)">{{ cv }}</dd>
                  </template>
                </template>
              </dl>
              <template v-for="address in contact.addresses">
                <dl class="address" :key="uniqueKey(address.name)">
                  <template v-for="(av, ae) in address">
                    <template v-if="av">
                      <dt :key="uniqueKey(ae)">{{ ae|camelTitle }}</dt>
                      <dd :key="uniqueKey(av)">{{ av }}</dd>
                    </template>
                  </template>
                </dl>
              </template>
            </template>
          </v-card-text>
        </v-card>
      </v-tab-item>
    </template>

    <template v-if="project.metadata.links.length > 0">
      <v-tab :href="`#links-${project.id}`">Links</v-tab>

      <v-tab-item :value="`links-${project.id}`" v-if="project.metadata.links">
        <v-card flat tile>
          <v-card-text>
            <ul v-for="link in project.metadata.links" :key="uniqueKey(link.url)">
              <li>
                <a :href="link.url">{{ link.name }}</a>
                &mdash; {{ link.description }}
              </li>
            </ul>
          </v-card-text>
        </v-card>
      </v-tab-item>
    </template>

    <v-tab :href="`#services-${project.id}`">Services</v-tab>
    <v-tab-item :value="`services-${project.id}`">
      <v-card flat tile>
        <v-card-text>
          <v-list>
            <v-list-item-group>
              <template v-if="project.capabilities.wfsLayerIds.length">
                <v-list-item :href="'./project/' + project.id + '/wfs3'">
                  <v-list-item-content>
                    <v-list-item-title>OAPIF/WFS3</v-list-item-title>
                  </v-list-item-content>
                </v-list-item>

                <v-list-item
                  :href="'./project/' + project.id + '/?SERVICE=WFS&amp;REQUEST=GetCapabilities'"
                >
                  <v-list-item-content>
                    <v-list-item-title>WFS GetCapabilities</v-list-item-title>
                  </v-list-item-content>
                </v-list-item>
              </template>

              <v-list-item
                :href="'./project/' + project.id + '/?SERVICE=WMS&amp;REQUEST=GetCapabilities'"
              >
                <v-list-item-content>
                  <v-list-item-title>WMS GetCapabilities</v-list-item-title>
                </v-list-item-content>
              </v-list-item>
            </v-list-item-group>
          </v-list>
        </v-card-text>
      </v-card>
    </v-tab-item>
  </v-tabs>
</template>


<script>
import { v4 as uuidv4 } from 'uuid';

export default {
  props: {
    project: Object
  },
  filters: {
    camelTitle(str) {
      str = str
        .replace(/([A-Z])/g, " $1")
        .toLowerCase()
        .split(" ");
      for (let i = 0; i < str.length; i++) {
        str[i] = str[i].charAt(0).toUpperCase() + str[i].slice(1);
      }
      return str.join(" ");
    }
  },
  data() {
    return {
      uniqueKey(value) {
        return uuidv4() + value;
      }
    };
  }
};
</script>

<style lang="scss" scoped>
dt {
  font-weight: bold;
}
</style>
