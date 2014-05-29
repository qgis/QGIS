##Home Range Analysis=group
##showplots
##Layer=vector
##Field=Field Layer
##Percentage=number 10
##Home_ranges=Output vector
library(adehabitatHR)
uu<-clusthr(Layer[,Field])
Home_ranges<-getverticeshr(uu,percent=Percentage)
ii <- MCHu2hrsize(uu, percent=seq(50, 100, by=5))
par(mar=c(2,2,2,2))
plot(ii)
