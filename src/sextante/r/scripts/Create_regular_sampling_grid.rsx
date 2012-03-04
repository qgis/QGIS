//polyg=vector
//numpoints=number
//output=output vector
//[Example scripts]=group
pts=spsample(polyg,numpoints,type="regular")
output=SpatialPointsDataFrame(pts, as.data.frame(pts))