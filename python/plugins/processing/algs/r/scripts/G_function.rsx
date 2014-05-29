##Point pattern analysis=group
##Layer=vector
##Nsim=number 10
##showplots
library("maptools")
library("spatstat")
ppp=as(as(Layer, "SpatialPoints"),"ppp")
plot(envelope(ppp, Gest, nsim=Nsim))
