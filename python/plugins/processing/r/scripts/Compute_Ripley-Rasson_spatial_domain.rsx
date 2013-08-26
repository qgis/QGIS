##[Example scripts]=group
##points=vector
##out=output vector
library("spatstat")
library("maptools")
spatpoints = as(points,"SpatialPoints")
ripras=ripras(as(spatpoints,"ppp"))
polyg=as(ripras,"SpatialPolygons")
out = SpatialPolygonsDataFrame(polyg, data.frame(1))
