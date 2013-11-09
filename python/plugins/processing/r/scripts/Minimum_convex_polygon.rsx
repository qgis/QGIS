##Home Range Analysis=group
##Layer=vector
##Percentage=number 10
##Field=Field Layer
##Home_ranges=Output vector
library(adehabitatHR)
Home_ranges<-mcp(Layer[,Field],percent=Percentage)
