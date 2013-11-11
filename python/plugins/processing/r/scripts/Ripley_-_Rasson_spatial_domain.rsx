##Point pattern analysis=group
##Layer=vector
##Output=output vector
library("spatstat")
library("maptools")
proj4string(Layer)->crs
spatpoints = as(Layer,"SpatialPoints")
ripras=ripras(as(spatpoints,"ppp"))
polyg=as(ripras,"SpatialPolygons")
Output1= SpatialPolygonsDataFrame(polyg, data.frame(1))
proj4string(Output1)<-crs
Output<-Output1
