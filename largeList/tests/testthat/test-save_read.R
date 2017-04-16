context("save_read")

## TODO: Rename context
## TODO: Add more tests

test_that("Append == FALSE", {
  skip_on_cran()
  skip_on_travis()
  source(paste0(path.package("largeList"), "/tests/config.R"))
  if (!ready_to_test) return(invisible(TRUE))
  flog.info("read random list file")
  data <- readRDS(rds_file_name)
  names(data) <- as.character(1:length(data))
  data_names <- names(data)
  bat <- length(data) / 5

  flog.info("save & test, append == F")
  
  ####
  flog.info("Part 1. save list")
  if (original) {
    saveList(data, llo_file_name, append =  F, compress = compress)
  } else {
    lf <- getList(llo_file_name, truncate = T, compress = compress)
    lf[[]] <- data
  }
  
  ####
  flog.info("Part 2. get length")
  expect_identical(length(data), ifelse(original, getListLength(llo_file_name), length(lf)))
  cat("\n")
  ####
  flog.info("Part 3. default index")
  if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
  expect_identical(data,res)
  cat("\n")
  ####
  flog.info("Part 4. positive index")
  for (i in 1:repeat_time) {
    index <- sample(c(1:(2*length(data)), rep(NA_integer_, 0.1*length(data))), length(data), replace = FALSE)
    if (original) {res <- readList(llo_file_name, index = index)} else {res <- lf[index]}
    res_real <- data[index]
    expect_identical(res, res_real)
  }
  cat("\n")
  ####
  flog.info("Part 5. negative index")
  for (i in 1:repeat_time) {
    index <- sample(-(2*length(data)):-1, 2*length(data), replace = TRUE)
    if (original) {res <- readList(llo_file_name, index = index)} else {res <- lf[index]}
    res_real <- data[index]
    expect_identical(res, res_real)
  }
  cat("\n")
  
  ####
  flog.info("Part 6. logical index")  
  for (i in 1:repeat_time) {
    index <- sample(c(T,F,NA), 0.3*length(data), replace = TRUE)
    if (original) {res <- readList(llo_file_name, index = index)} else {res <- lf[index]}
    res_real <- data[index]
    expect_identical(res, res_real)
  }
  cat("\n")
  
  ####
  flog.info("Part 7. character index")   
  for (i in 1:repeat_time) {
    index <- sample(c(names(data),as.character(-0.2*length(data):0), rep(NA_character_, 0.1*length(data))), length(data), replace = FALSE)
    if (original) {res <- readList(llo_file_name, index = index)} else {res <- lf[index]}
    res_real <- data[index]
    expect_identical(res, res_real)
  }
  cat("\n")
})


test_that("Append == TRUE", {
  skip_on_cran()
  skip_on_travis()
  if (!ready_to_test) return(invisible(TRUE))
  flog.info("read random list file")
  data <- readRDS(rds_file_name)
  names(data) <- as.character(1:length(data))
  data_names <- names(data)
  bat <- length(data) / 5
  
  flog.info("save & test, append == T")
  lf <- getList(llo_file_name, truncate = T, compress = compress)
  
  ####
  flog.info("Part 1. save list")
  if (original) {
    saveList(list(), llo_file_name, append =  F, compress = compress)
  }else {
    lf[[]] <- list()
  }
  stored_name <- c()
  res_real <- c()
  index_random <- sample(1:length(data),length(data))
  select_list <- split(index_random, 1:5)
  for (select in select_list) {
    # print(select)
    stored_name <- c(stored_name,data_names[select])
    res_real <- c(res_real, data[select])
    if (original) { 
      saveList(data[select], llo_file_name, append =  T)
    } else {
      lf[] <- data[select]
    }
    if (original) { res <- readList(llo_file_name)} else {res <- lf[]}
    expect_identical(res, res_real)
    if (original) { res <- readList(llo_file_name, stored_name)} else {res <- lf[stored_name]}
    expect_identical(res, res_real)
  }
})
