/**
 *  Landing page app entry point
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
import Vue from "vue"
import VueRouter from "vue-router"
import App from "@/App.vue"
import store from "@/store"

import vuetify from "./plugins/vuetify"

Vue.config.productionTip = false

// Routing
Vue.use(VueRouter)

const routes = [
  {
    path: "/",
    name: "catalog",
    component: () => import("@/views/ServerCatalog.vue"),
    meta: {
      title: "QGIS Server Catalog - Home Page",
      metaTags: [
        {
          name: "description",
          content: "List of available projects in QGIS Server catalog.",
        },
      ],
    },
  },
  {
    path: "/map/:projectId",
    name: "map",
    component: () => import("@/views/WebGis.vue"),
    props: true,
    meta: {
      title: "QGIS Server Project",
      metaTags: [
        {
          name: "description",
          content: "QGIS Server Project",
        },
      ],
    },
  },
]

const router = new VueRouter({
  routes, // short for `routes: routes`
})

// This callback runs before every route change, including on page load.
router.beforeEach((to, from, next) => {
  // This goes through the matched routes from last to first, finding the closest route with a title.
  // eg. if we have /some/deep/nested/route and /some, /deep, and /nested have titles, nested's will be chosen.
  const nearestWithTitle = to.matched
    .slice()
    .reverse()
    .find((r) => r.meta && r.meta.title)

  // Find the nearest route element with meta tags.
  const nearestWithMeta = to.matched
    .slice()
    .reverse()
    .find((r) => r.meta && r.meta.metaTags)

  // If a route with a title was found, set the document (page) title to that value.
  if (nearestWithTitle) document.title = nearestWithTitle.meta.title

  // Remove any stale meta tags from the document using the key attribute we set below.
  Array.from(document.querySelectorAll("[data-vue-router-controlled]")).map((el) => el.parentNode.removeChild(el))

  // Skip rendering meta tags if there are none.
  if (!nearestWithMeta) return next()

  // Turn the meta tag definitions into actual elements in the head.
  nearestWithMeta.meta.metaTags
    .map((tagDef) => {
      const tag = document.createElement("meta")

      Object.keys(tagDef).forEach((key) => {
        tag.setAttribute(key, tagDef[key])
      })

      // We use this to track which meta tags we create, so we don't interfere with other ones.
      tag.setAttribute("data-vue-router-controlled", "")

      return tag
    })
    // Add the meta tags to the document head.
    .forEach((tag) => document.head.appendChild(tag))

  next()
})

new Vue({
  router,
  vuetify,
  store,
  render: (h) => h(App),
}).$mount("#app")
