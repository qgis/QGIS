# Landing page/catalog webapp

[Landing page/catalog webapp](https://docs.qgis.org/latest/en/docs/server_manual/services.html#qgis-server-catalog) source code.

## Building

To build the QGIS server landingpage webapp additional dependencies are required:

Node.js (current LTS recommended): https://nodejs.org/en/download/<br>
Yarn Package Manager: https://yarnpkg.com/getting-started/install

To build the webapp along with QGIS server just turn the cmake flag `WITH_SERVER_LANDINGPAGE_WEBAPP` on and build as you would normally do.

To test the webapp with a local QGIS project after your build is finished, set the [environment variable](https://docs.qgis.org/latest/en/docs/server_manual/config.html#environment-variables) for your project directory and run the [development server](https://docs.qgis.org/latest/en/docs/server_manual/development_server.html):

```
export QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES=/path/to/projectdirectory

./output/bin/qgis_mapserver -p /path/to/projectdirectory/test.qgz
```

Then open http://localhost:8000 in a web browser (default port `8000`).


## Development

Development is done by modifying the source code and building and running the webapp as described above.

#### update `yarn.lock`

To update `yarn.lock` run `yarn install` manually and make sure to remove the `node_modules` directory from your QGIS source tree before running cmake and building.

#### Lints and fixes files
```
yarn lint
```

#### Customize configuration
See [Configuration Reference](https://cli.vuejs.org/config/).
