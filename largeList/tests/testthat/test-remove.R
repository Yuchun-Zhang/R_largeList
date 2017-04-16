context("remove")

## TODO: Rename context
## TODO: Add more tests

test_that("remove from list", {
  skip_on_cran()
  skip_on_travis()
  source(paste0(path.package("largeList"), "/tests/config.R"))
  if (!ready_to_test) return(invisible(TRUE))
  flog.info("read random list file")
  data <- readRDS(rds_file_name)
  names(data) <- as.character(1:length(data))
  data_names <- names(data)
  bat <- length(data) / 5
  
  flog.info("remove from list")
  if (!original) {lf <- getList(llo_file_name, truncate = T, compress = compress)}
  
  ####
  flog.info("Part 1. positive index")
  if (original) {saveList(data,llo_file_name, append =  F, compress = compress)} 
    else {lf[[]] <- data}
  remain <- 1:length(data)
  res_real <- data
  while (length(res_real) > 0) {
    num <- sample(bat:(2*bat),1)
    index <- sample(c(1:length(res_real), rep(NA_integer_, 0.3*length(res_real))) , num, replace = T)
    index <- sample(index, 4*length(index), replace = T)
    flog.info("Remain %d ", length(res_real))
    
    if (original) {removeFromList(llo_file_name, index)} else {lf[index] <- NULL}
    res_real[index] <- NULL
    
    if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
    expect_identical(res, res_real)
    
    if (original) {res <- readList(llo_file_name,names(res_real))} else {res <- lf[names(res_real)]}
    expect_identical(res, res_real)
    cat("\n")
  }
  
  ####
  flog.info("Part 2. negative index")
  if (original) {saveList(data, llo_file_name, append =  F, compress = compress)} else {lf[[]] <- data}
  remain <- 1:length(data)
  res_real <- data
  while (length(res_real) > 0) {
    index <- -sample(1:(2*length(res_real)), 2*length(res_real)*0.7, replace = T)
    index <- sample(index, 4*length(index), replace = T)
    flog.info("Remain %d ", length(res_real))
    
    if (original) {removeFromList(llo_file_name, index)} else {lf[index] <- NULL}
    res_real[index] <- NULL
    
    if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
    expect_identical(res, res_real)
    
    if (original) {res <- readList(llo_file_name,names(res_real))} else {res <- lf[names(res_real)]}
    expect_identical(res, res_real)
    cat("\n")
  }
  
  ####
  flog.info("Part 3. logical index")
  if (original) {saveList(data,llo_file_name, append =  F, compress = compress)} else {lf[[]] <- data}
  remain <- 1:length(data)
  res_real <- data
  while (length(res_real) > 0) {
    index <- sample(c(T, F, NA), length(res_real), replace = T)
    flog.info("Remain %d ", length(res_real))
    
    if (original) {removeFromList(llo_file_name, index)} else {lf[index] <- NULL}
    res_real[index] <- NULL
    
    if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
    expect_identical(res, res_real)
    
    if (original) {res <- readList(llo_file_name,names(res_real))} else {res <- lf[names(res_real)]}
    expect_identical(res, res_real)
    cat("\n")
  }
  
  ####
  flog.info("Part 4. character index")
  if (original) {saveList(data,llo_file_name, append =  F, compress = compress)} else {lf[[]] <- data}
  res_real <- data
  index_random <- sample(1:length(data),length(data))
  select_list <- split(index_random, 1:5)
  for (select in select_list) {
    index <- c(as.character(select), as.character(-length(res_real):-1), rep(NA_character_, 0.3*length(select)))
    index <- sample(rep(index,4), 4*length(index))
    flog.info("Remain %d ", length(res_real))
    
    if (original) {removeFromList(llo_file_name, index)} else {lf[index] <- NULL}
    res_real[index] <- NULL
    
    if (original) {res <- readList(llo_file_name)} else {res <- lf[]}
    expect_identical(res, res_real)
    
    if (original) {res <- readList(llo_file_name,names(res_real))} else {res <- lf[names(res_real)]}
    expect_identical(res, res_real)
    cat("\n")
  }
})
