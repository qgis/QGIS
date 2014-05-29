##Point pattern analysis=group
##Layer=vector
##Size=number 10
##Output= output vector
pts=spsample(Layer,Size,type="regular")
Output=SpatialPointsDataFrame(pts, as.data.frame(pts))
