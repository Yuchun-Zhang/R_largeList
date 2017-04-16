context("modify_name")

## TODO: Rename context
## TODO: Add more tests

test_that("modify name in list", {
  skip_on_cran()
  skip_on_travis()
  source(paste0(path.package("largeList"), "/tests/config.R"))
  if (!ready_to_test) return(invisible(TRUE))
  flog.info("read random list file")
  data <- readRDS(rds_file_name)
  names(data) <- as.character(1:length(data))
  data_names <- names(data)
  bat <- length(data) / 5
  
  flog.info("modify name in list")
  if (!original) {lf <- getList(llo_file_name, truncate = T)}
  
  #### 
  if (original) {saveList(data, llo_file_name, append =  F)} else {lf[[]] <- data}
  memData <- data
  flog.info("Part 1. positive index")
  for (i in 1:ceiling(repeat_time / 4)) {
    index <- sample(1:length(data), length(data) / 5)
    name_max <- max(as.numeric(names(memData)))
    new_name <- as.character((1 + name_max):(name_max + length(data) / 5))
    if (original) {modifyNameInList(llo_file_name, index, new_name)} else {names(lf)[index] <- new_name}
    names(memData)[index] <- new_name
    if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
    expect_identical(res, memData)
    if (original) {res <- readList(llo_file_name, names(memData))} else {res <- lf[names(memData)]}
    expect_identical(res, memData)
    if (original) {res_name <- getListName(llo_file_name)} else {res_name <-  names(lf)}
    expect_identical(res_name, names(memData))
  }
  cat("\n")
  
  flog.info("Part 2. negative index")
  for (i in 1:ceiling(repeat_time / 4)) {
    index <- -sample(1:length(data), length(data) - (length(data) / 5))
    name_max <- max(as.numeric(names(memData)))
    new_name <- as.character((1 + name_max):(name_max + length(data) / 5))
    if (original) {modifyNameInList(llo_file_name, index, new_name)} else {names(lf)[index] <- new_name}
    names(memData)[index] <- new_name
    if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
    expect_identical(res, memData)
    if (original) {res_name <- getListName(llo_file_name)} else {res_name <-  names(lf)}
    expect_identical(res_name, names(memData))
  }
  cat("\n")
  
  flog.info("Part 3. logical index")
  for (i in 1:ceiling(repeat_time / 4)) {
    index <- sample(c(T,F), length(data) / 5, replace = T)
    name_max <- max(as.numeric(names(memData)))
    new_name <- as.character((1 + name_max):(name_max + length(data) / 5))
    if (original) {modifyNameInList(llo_file_name, index, new_name)} else {names(lf)[index] <- new_name}
    names(memData)[index] <- new_name
    if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
    expect_identical(res, memData)
    if (original) {res_name <- getListName(llo_file_name)} else {res_name <-  names(lf)}
    expect_identical(res_name, names(memData))
  }
  cat("\n")
  
  flog.info("Part 4. Remove Names")
  if (original) {modifyNameInList(llo_file_name, NULL, NULL)} else {names(lf) <- NULL}
  names(memData) <- NULL
  if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
  expect_identical(res, memData)
  if (original) {res <- readList(llo_file_name, names(memData))} else {res <- lf[names(memData)]}
  expect_identical(res, memData)
  if (original) {res_name <- getListName(llo_file_name)} else {res_name <- names(lf)}
  expect_identical(res_name, names(memData))
  cat("\n")
  
  flog.info("Part 5. Add Names")
  index <- sample(c(T,F), length(data) / 5, replace = T)
  new_name <- as.character(1:length(index))
  if (original) {
    modifyNameInList(llo_file_name, index, new_name)
  } else {
    names(lf)[index] <- new_name
  }
  names(memData)[index] <- new_name
  if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
  expect_identical(res, memData)
  if (original) {res_name <- getListName(llo_file_name)} else {res_name <-  names(lf)}
  expect_identical(res_name, names(memData))
  cat("\n")
})
