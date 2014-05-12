##Layer=raster
##Raster processing=group
##Dens_or_Hist=string Hist
##showplots
library(rpanel)
library(rasterVis)
str <- Dens_or_Hist
if (str !='Dens' & str != 'Hist'){
rp.messagebox('you must enter "Dens" or "Hist"', title = 'oops!')
} else {
if (nbands(Layer) == 1) {
Layer <- as.matrix(Layer)
Layer <- raster(Layer)
}
if (str == 'Dens') {
densityplot(Layer)
} else if (str == 'Hist') {
histogram(Layer)
}
}
