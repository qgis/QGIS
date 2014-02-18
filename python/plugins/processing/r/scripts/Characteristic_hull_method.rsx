##Home Range Analysis=group
##Layer=vector
##Field=Field Layer
##Home_ranges=Output vector
library(adehabitatHR)
library(deldir)
res <- CharHull(Layer[,Field])
Home_ranges<-getverticeshr(res)
