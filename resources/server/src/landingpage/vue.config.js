module.exports = {
    publicPath: process.env.NODE_ENV === 'production'
        ? './'
        : '/',
  assetsDir: './',
  configureWebpack: {
    devtool: "source-map",
    optimization: {
      splitChunks: {
        chunks: "all",
      },
    }
  },
  runtimeCompiler: true,
  devServer: {
    proxy: "http://192.168.99.47:8001",
  },
  transpileDependencies: ["vuetify"],
}
