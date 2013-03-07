##passFileNames
##[Example scripts]=group
##RasterFile=raster
##RasterLayer=string all
##w=string 3
##Function=string mean
##outputRasterFile=output raster

library(raster)

if(tolower(RasterLayer)=="all")
{
	RasterFile  <- brick(RasterFile)
	outputRasterFile <- stack(RasterFile)
} else 
{
	RasterFile  <- raster(RasterFile,band=as.numeric(RasterLayer))
	outputRasterFile <- raster(RasterFile)
}

w <- as.numeric(strsplit(w,",")[[1]])

if(length(w)>1)
{
    if (sqrt(length(w))%%2!=1)
    {   
        stop("It is not possible to create a convolution matrix from your 'w', check the length of your 'w', if must be possible to create a 3x3, 5x5, 7x7... matrix\n")
    }
	w <- matrix(w, nrow=sqrt(length(w)))
	Function <- NULL
}

if (!is.null(Function))
{
    if (tolower(Function)=="laplacian")
    {
    	if (w[1]!=3)
    	{
    	    cat("Laplacian filter requires a moving window size of 3, changed w=3!\n")
    	}
    	w <- matrix(c(0,1,0,1,-4,1,0,1,0), nrow=3)
    	Function=NULL
    } else if (tolower(Function)=="sobel")
    {
    	if (w[1]!=3)
    	{
    	    cat("Sobel filter requires a moving window size of 3, changed w=3!\n")
    	}
    	w <- matrix(c(1,2,1,0,0,0,-1,-2,-1) / 4, nrow=3)
    	Function=NULL
    } else if (tolower(Function)=="mean")
    {
    	w <- matrix(1/(w*w), nrow=w,ncol=w)
    	Function=NULL
    } else if (tolower(Function)=="sharpen")
    {
    	if (w[1]!=3)
    	{
    	    cat("Sharpen filter requires a moving window size of 3, changed w=3!\n")
    	}
    	w <- matrix(c(0,-1,0,-1,5,-1,0,-1,0), nrow=3)
    	Function=NULL
    } else if (tolower(Function)=="emboss")
    {
    	if (w[1]!=3)
    	{
    	    cat("Emboss filter requires a moving window size of 3, changed w=3!\n")
    	}
    	w <- matrix(c(-2,-1,0,-1,1,1,0,1,2), nrow=3)
    	Function=NULL
    }
}

for (i in 1:nlayers(RasterFile))
{
	if (is.null(Function))
	{
		outputRasterFile[[i]] <- focal(RasterFile[[i]],w=w)
	} else 
	{
		outputRasterFile[[i]] <- focal(RasterFile[[i]],w=w,fun=match.fun(Function))
	}
}











