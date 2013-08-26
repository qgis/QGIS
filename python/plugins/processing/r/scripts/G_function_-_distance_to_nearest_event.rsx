##[Example scripts]=group
##points=vector
##showplots
library("maptools")
library("spatstat")
ppp=as(as(points, "SpatialPoints"),"ppp")
plot(envelope(ppp, Gest, nsim=100))
