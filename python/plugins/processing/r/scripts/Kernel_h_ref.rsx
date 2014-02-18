##Home Range Analysis=group
##Layer=vector
##Field=Field Layer
##Grid=number 10
##Percentage=number 10
##Home_ranges=Output vector
##Folder=folder
library(adehabitatHR)
Layer[,Field]->relocs
kud <- kernelUD(relocs, grid=,Grid, h="href")
names(kud)->Names
for(i in 1:length(Names)){
writeGDAL(kud[[i]],paste(paste(Folder,"/",sep=""),paste(Names[i],".tiff",sep=""), sep=""),drivername="GTiff")
}
Home_ranges<- getverticeshr(kud,percent=Percentage)
