const LicensePlugin = require('webpack-license-plugin')

module.exports = {
  publicPath: './',
  assetsDir: './',
  configureWebpack: {
    devtool: "source-map",
    optimization: {
      splitChunks: {
        chunks: "all",
      },
    },
    plugins: [
      new LicensePlugin()
    ]
  },
  runtimeCompiler: true,
  devServer: {
    proxy: "http://127.0.0.1:8001",
  },
  transpileDependencies: ["vuetify"],
}