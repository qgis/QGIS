##[Example scripts]=group
##layer=vector
##field=field layer
Summary_statistics<-data.frame(name.i=rbind(length(layer[[field]]),
length(unique(layer[[field]])),
min(layer[[field]]),
max(layer[[field]]), 
max(layer[[field]])-min(layer[[field]]),
mean(layer[[field]]),
median(layer[[field]]), 
sd(layer[[field]])),row.names=c("Count:","Unique values:","Minimum value:","Maximum value:","Range:","Mean value:","Median value:","Standard deviation:"))
colnames(Summary_statistics)<-c(field)
>Summary_statistics
        
