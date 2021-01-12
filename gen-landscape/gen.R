## 2019-11-06

## These counties have infections initially:
## Foard, Blanco, Childress, Collingsworth, Cottle, Hardeman, Kendall, King, Scurry, Wheeler
## ids: 4, 38, 46, 49, 53, 61, 71, 74, 87, 93

## output hibernacula.csv has columns:
## patch_idx,county_idx,init_inf,intemp_hi,intemp_lo,intemp_base,
##   outemp_ampl,outemp_base,K,wdist

tp <- read.table('topology.txt',header=TRUE)
ct <- read.table('cavetemps.txt',header=TRUE)
ncounties <- nrow(tp)
prism <- read.csv('prism-all-counties.csv')
initic <- c('Foard','Blanco','Childress','Collingsworth','Cottle',
            'Hardeman','Kendall','King','Scurry','Wheeler')

hibm <- matrix(0,nr=nrow(ct),nc=10)
counter <- 1
for(i in 1:nrow(tp)){
    countyname <- as.character(tp[i,'county'])
    wdist <- tp[i,'wdist']
    tsub <- subset(prism, Name == countyname)
    time <- 1:nrow(tsub)
    xc <- cos(2.0*pi*time/365.15)
    xs <- sin(2.0*pi*time/365.15)
    fitlm <- lm(tsub$TempMeanC ~ xc + xs)
    pred <- predict(fitlm, newdata = data.frame(time = time))
    obase <- (fitlm$coefficients)['(Intercept)']
    oampl <- max(pred) - obase
    csub <- subset(ct, county == countyname)
    for(j in 1:nrow(csub)){
        hibm[counter,1] <- (counter-1) # patch id
        hibm[counter,2] <- tp[i,'id'] # county id
        if(is.element(countyname,initic)){
            hibm[counter,3] <- 1  # init infection
        }else{
            hibm[counter,3] <- 0
        }
        hibm[counter,4] <- csub[j,'inhi']  # intemp_hi
        hibm[counter,5] <- csub[j,'inlo']  # intemp_lo
        hibm[counter,6] <- csub[j,'intemp']  # intemp_base
        hibm[counter,7] <- oampl #14 # outemp_ampl
        hibm[counter,8] <- obase #10 # outemp_base
        hibm[counter,9] <- csub[j,'ccap'] # K
        hibm[counter,10] <- wdist
        counter <- counter + 1
    }
}

colnames(hibm) <- c('patch','county','init','ihi','ilo','ibase',
                    'oampl','obase','ccap','wdist')

write.table(hibm,file='hibernacula.csv',sep=',',
            row.names=FALSE,col.names=TRUE)

cutoff <- 100.0 ## 140.0
dists <- matrix(0,nr=0,nc=4)

size <- nrow(hibm)
for(i in 1:size){
    for(j in 1:size){
        if(i != j){
            frpidx <- hibm[i,1]
            topidx <- hibm[j,1]
            frcidx <- hibm[i,2]
            tocidx <- hibm[j,2]
            if(frcidx == tocidx){
                ## same county
                tprow <- which(tp$id == frcidx)
                distance <- tp[tprow,'wdist']
                within <- 0
            }else{
                ## different county
                ftprow <- which(tp$id == frcidx)
                ttprow <- which(tp$id == tocidx)
                fx <- tp[ftprow,'xloc']
                fy <- tp[ftprow,'yloc']
                tx <- tp[ttprow,'xloc']
                ty <- tp[ttprow,'yloc']
                distance <- sqrt((fx - tx)^2 + (fy - ty)^2)
                within <- 1
            }
            if(distance < cutoff){
                dists <- rbind(dists,
                               c(frpidx, topidx, within, distance))
            }
        }
    }
}

write.table(dists,file='distances.csv',sep=',',
            row.names=FALSE,col.names=FALSE)

