##Point pattern analysis=group
##points=vector
##covariate=raster
##covariate_name=string mandatory_covariate_name_(no_spaces)
##x_label=string
##plot_name=string
##legend_position=string float
##showplots
library(geostatsp)
library(maptools)
library(rpanel)
if (covariate_name == "") {
rp.messagebox('"covariate name" must not be empty!', title = 'oops!')
}
else {
S <- points
SP <- as(S, "SpatialPoints")
P <- as(SP, "ppp")
covariate <- raster(covariate, layer = 1)
covariate <- as.im(covariate)
library(spatstat)
S <- points
SP <- as(S, "SpatialPoints")
P <- as(SP, "ppp")
plot(rhohat(P, covariate, covname=covariate_name), xlab= x_label,
legendpos = legend_position,
legendargs=list(bg="transparent"),
main = plot_name)
}
