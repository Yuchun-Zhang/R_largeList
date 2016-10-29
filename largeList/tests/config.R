mac_dir <- "~/Documents/Rtest/"
win_dir <- "C:/Rtest"

rds_file_name <- "./ranListSmall.rds"
llo_file_name <- "./ranListSmall.llo"
repeat_time <- 50
original <- FALSE
compress <- TRUE

# rds_file_name <- "./ranListMid.rds"
# llo_file_name <- "./ranListMid.llo"
# repeat_time <- 20
# original <- TRUE
# compress <- FALSE

## TODO
# (1) protection stack overflow if there are too many elements in the list
# (2) 