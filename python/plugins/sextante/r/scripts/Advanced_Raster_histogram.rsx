##userasterpackage
##[Example scripts]=group
##Layer = raster
##no_data_value = number 0
##showplots
Layer <- raster(Layer, 1)
NAvalue(Layer) = no_data_value
hist(as.matrix(Layer), breaks=100, xlab = basename(filename(Layer)))
