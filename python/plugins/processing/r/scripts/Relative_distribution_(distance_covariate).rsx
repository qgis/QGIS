##Point pattern analysis=group
##Layer=vector
##Covariate=vector
##Covariate_name=string mandatory_covariate_name_(no_spaces)
##x_label=string
##Plot_name=string
##Legend_position=string float
library(spatstat)
library(maptools)
library(rpanel)
if (Covariate_name == "") {
rp.messagebox('Covariate name must not be emply!', title = 'oops!')}
else {
S <- Layer
SP <- as(S, "SpatialPoints")
P <- as(SP, "ppp")
r <- Covariate
rp <- as.psp(r)
rdist <- distfun(rp)
plot(rhohat(P, rdist, covname=Covariate_name), xlab= x_label,
legendpos = Legend_position,
legendargs=list(bg="transparent"),
main = Plot_name)
##showplots
}
