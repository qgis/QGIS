##Point pattern analysis=group
##Layer=vector
##Simulations=number 100
##Optional_plot_name=string
##showplots
library(spatstat)
library(maptools)
sp <- as(Layer, "SpatialPoints")
sp <- as(sp, "ppp")
e <- envelope(sp, Kest, nsim = Simulations)
>e
plot(e, main = Optional_plot_name)
