working_dir <- "working_directory"
rds_file_name <- "rds_file_use_to_test.rds"
llo_file_name <- "temp_large_list_file.llo"

repeat_time <- 20
original <- FALSE
compress <- TRUE

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




