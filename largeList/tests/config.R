mac_dir <- "~/Documents/Rtest/"
win_dir <- "C:/Rtest"
linux_dir <- "~/Documents/Rtest"

switch(Sys.info()['sysname'],
       "Darwin" = {working_dir <- mac_dir},
       "Windows" = {working_dir <- win_dir},
       "Linux" = {working_dir <- linux_dir})

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

options(list(largeList.report.progress = FALSE))

if (dir.exists(working_dir)) {
  setwd(working_dir)
  if (file.exists(rds_file_name)) {
    library(futile.logger)
    ready_to_test <- TRUE
  } else {
    ready_to_test <- FALSE
  }
} else {
  ready_to_test <- FALSE
}




