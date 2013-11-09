##Point pattern analysis=group
##Layer=vector
##showplots
library("maptools")
library("spatstat")
ppp=as(as(Layer, "SpatialPoints"),"ppp")
qc=quadratcount(ppp)
plot(Layer)
plot(qc, add=TRUE)
>quadrat.test(ppp);
