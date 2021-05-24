/**
 *  Store
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
import Vuex from "vuex"

Vue.use(Vuex)

export default new Vuex.Store({
  state: {
    catalog: [],
    projects: {},
    attributeTableTypename: "",
    tocs: {},
    error: "",
    status: "loading",
    activeTool: "",
    identifyResults: {},
    baseMap: "",
    mapScaleDenominator: 1,
  },
  mutations: {
    setCatalog(state, payload) {
      state.catalog = payload
    },
    setBaseMap(state, payload) {
      state.baseMap = payload
    },
    setStatus(state, payload) {
      state.status = payload
    },
    setActiveTool(state, payload) {
      state.activeTool = payload
    },
    setError(state, payload) {
      state.error = payload
    },
    clearError(state) {
      state.error = ""
    },
    clearIdentifyResults(state) {
      state.identifyResults = {}
    },
    setProject(state, project) {
      Vue.set(state.projects, project.id, project)
    },
    setToc(state, { projectId, toc }) {
      Vue.set(state.tocs, projectId, toc)
    },
    setIdentifyResults(state, { identifyResults }) {
      state.identifyResults = identifyResults
    },
    setAttributeTableLayerTypename(state, tree_hash_id) {
      state.attributeTableTypename = tree_hash_id
    },
    clearAttributeTableTypename(state) {
      state.attributeTableTypename = ""
    },
    setMapScaleDenominator(state, mapScaleDenominator) {
      state.mapScaleDenominator = mapScaleDenominator
    },
  },
  actions: {
    async getCatalog({ commit }) {
      try {
        fetch(`./index.json`)
          .then((response) => {
            if (!response) {
              throw Error(`Error fetching data from QGIS Server`)
            }
            if (!response.ok) {
              throw Error(response.statusText)
            }
            return response
          })
          .then((response) => response.json())
          .then((json) => {
            json.projects.forEach((element) => {
              element.show = false
            })
            commit("setCatalog", json.projects)
            commit("setStatus", json.projects.length ? `projects` : `empty`)
          })
          .catch((error) => {
            commit("setError", error.message)
          })
      } catch (error) {
        commit("setError", error.message)
      }
    },
    async getProject({ commit }, projectId) {
      try {
        fetch(`./map/${projectId}.json`)
          .then((response) => {
            if (!response) {
              throw Error(`Error fetching data from QGIS Server`)
            }
            if (!response.ok) {
              throw Error(response.statusText)
            }
            return response
          })
          .then((response) => response.json())
          .then((json) => {
            commit("setProject", json.project)
            commit("setStatus", `project`)
          })
          .catch((error) => {
            commit("setError", error.message)
          })
      } catch (error) {
        commit("setError", error.message)
      }
    },
    setStatus({ commit }, status) {
      commit("setStatus", status)
    },
    /**
     * Fetches the TOC style icons from GetLegendGraphics
     */
    async getToc({ commit }, payload) {
      let toc_url = `./project/${payload.projectId}/?SERVICE=WMS&REQUEST=GetLegendGraphics&LAYERS=${payload.layers}&FORMAT=application/json`
      fetch(toc_url)
        .then(this.handleErrors)
        .then((response) => response.json())
        .then((toc) => {
          commit("setToc", { projectId: payload.projectId, toc })
        })
        .catch((error) => {
          commit("setError", error.message)
        })
    },
    /*,
    async getIdentifyResults({ commit }, payload) {
      let toc_url = payload.url
      fetch(toc_url)
        .then(this.handleErrors)
        .then((response) => response.json())
        .then((identifyResults) => {
          commit("setIdentifyResults", { projectId: payload.projectId, identifyResults })
        })
        .catch((error) => {
          commit("setError", error.message)
        })
    },*/
  },
})
