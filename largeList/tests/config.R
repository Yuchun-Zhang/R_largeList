library(futile.logger)

mac_dir <- "~/Documents/Rtest/"
win_dir <- "C:/Rtest"
linux_dir <- "~/Documents/Rtest"

rds_file_name <- "./ranListSmall.rds"
llo_file_name <- "./ranListSmall.llo"
repeat_time <- 20
original <- FALSE
compress <- TRUE

# rds_file_name <- "./ranListMid.rds"
# llo_file_name <- "./ranListMid.llo"
# repeat_time <- 20
# original <- FALSE
# compress <- TRUE

switch(Sys.info()['sysname'],
       "Darwin" = {setwd(mac_dir)},
       "Windows" = {setwd(win_dir)},
       "Linux" = {setwd(linux_dir)})