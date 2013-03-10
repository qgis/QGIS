##[Example scripts]=group
##Layer = raster
##no_data_value = number 0
##breaks = number 100
##color = string red
##showplots
NAvalue(Layer) = no_data_value
hist(Layer, breaks=breaks,col=color)
