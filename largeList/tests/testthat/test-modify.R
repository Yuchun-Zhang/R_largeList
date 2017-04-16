context("modify")

## TODO: Rename context
## TODO: Add more tests

test_that("modify in list", {
  skip_on_cran()
  skip_on_travis()
  source(paste0(path.package("largeList"), "/tests/config.R"))
  if (!ready_to_test) return(invisible(TRUE))
  flog.info("read random list file")
  data <- readRDS(rds_file_name)
  names(data) <- as.character(1:length(data))
  data_names <- names(data)
  bat <- length(data) / 5
  
  flog.info("modify in list")
  if (!original) {lf <- getList(llo_file_name, truncate = T, compress = compress)}
  
  #### 
  flog.info("Part 1. positive index")
  if (original) {saveList(data, llo_file_name, append =  F, compress = compress)} else {lf[[]] <- data}
  memData <- data
  for (i in 1:repeat_time) {
    if (original) {
      index <- sample(c(1:length(data), rep(NA_integer_, 0.0*length(data))),100)
    } else {
      index <- sample(c(1:(1.5*length(data)), rep(NA_integer_, 0.0*length(data))),100)  
    }
    index <- sample(index, 4*length(index), replace = T)
    swap <- sample(1:length(data),100)
    if (original) {modifyInList(index, data[swap],file = llo_file_name)} else {lf[index] <- data[swap]}
    memData[index] <- data[swap]
    names(memData)[names(memData) == ""] <- NA_character_
    if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
    expect_identical(res, memData)
  }
  cat("\n")
  
  ####
  flog.info("Part 2. negative index")
  if (original) {saveList(data,llo_file_name, append =  F, compress = compress)} else {lf[[]] <- data}
  memData <- data
  for (i in 1:repeat_time) {
    index <- -sample(1:(2*length(data)), 100, replace = T)
    index <- sample(index, 4*length(index), replace = T)
    swap <- sample(1:length(data),100)
    if (original) {modifyInList(index, data[swap],file = llo_file_name)} else {lf[index] <- data[swap]}
    memData[index] <- data[swap]
    if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
    expect_identical(res, memData)
  }
  cat("\n")
  
  ####
  flog.info("Part 3. logical index")
  if (original) {saveList(data,llo_file_name, append =  F, compress = compress)} else {lf[[]] <- data}
  memData <- data
  for (i in 1:repeat_time) {
    index <- sample(c(T, F), 100, replace = T)
    index <- sample(index, 4*length(index), replace = T)
    swap <- sample(1:length(data),100)
    if (original) {modifyInList(index, data[swap],file = llo_file_name)} else {lf[index] <- data[swap]}
    memData[index] <- data[swap]
    if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
    expect_identical(res, memData)
  }
  cat("\n")
  
  ####
  flog.info("Part 4. character index")
  if (original) {saveList(data,llo_file_name, append =  F, compress = compress)} else {lf[[]] <- data}
  memData <- data
  for (i in 1:repeat_time) {
    if (original) {
      index <- sample(c(data_names, rep(NA_character_, 0.0*length(data_names))),100)
    } else {
      index <- sample(c(data_names, 
                        as.character((length(data_names) + 1):(2*length(data_names))), 
                        rep(NA_character_, 0.0*length(data_names)) 
      ),100, replace = T) 
    }
    index <- sample(index, 4*length(index), replace = T)
    swap <- sample(1:length(data),100)
    if (original) {modifyInList(index, data[swap],file = llo_file_name)} else {lf[index] <- data[swap]}
    memData[index] <- data[swap]
    if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
    expect_identical(res, memData)
  }
  cat("\n")
})
