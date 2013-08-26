##[Example scripts]=group
##points=vector
##showplots
library("maptools")
library("spatstat")
ppp=as(as(points, "SpatialPoints"),"ppp")
qc=quadratcount(ppp)
plot(points)
plot(qc, add=TRUE)
>quadrat.test(ppp);
