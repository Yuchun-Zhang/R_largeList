context("extrem_case")

## TODO: Rename context
## TODO: Add more tests

test_that("extrem cases", {
  skip_on_cran()
  skip_on_travis()
  source(paste0(path.package("largeList"), "/tests/config.R"))
  if (!ready_to_test) return(invisible(TRUE))
  flog.info("test extrem cases : read")
  l <- as.list(1:10)
  names(l) <- LETTERS[1:10]
  
  lf <- getList("test", truncate = T)
  lf[[]] <- as.list(1:10)
  names(lf) <- LETTERS[1:10]
  
  expect_identical(l[c(1, NA, 2)], lf[c(1, NA, 2)]) # get list(A = 1, <NA> = NULL, B = 2)
  expect_identical(l[c(1, 20)], lf[c(1, 20)]) # get list(A = 1, <NA> = NULL)
  expect_identical(l[c(0, 1)], lf[c(0, 1)]) # get list(A = 1)
  expect_identical(l[c(0)],lf[c(0)]) # get named list()
  expect_identical(l[c(-1)], lf[c(-1)]) # same as l[1:9]
  expect_identical(l[c(0,-1)], lf[c(0,-1)]) # same as l[c(-1)]
  expect_identical(l[c(0,NA_integer_)], lf[c(0,NA_integer_)]) # get <NA> = NULL
  expect_error(lf[c(-1,NA)],"may be mixed with negative subscripts") 
  # only 0's may be mixed with negative subscripts
  
  expect_identical(l[c(T,F)], lf[c(T,F)]) # get l[c(1,3,5,7,9)]
  expect_identical(l[c(T,NA,F)], lf[c(T,NA,F)])  # get l[c(1,NA,4,NA,7,NA,10)]
  
  expect_identical(l[c("A",NA, "Z")], lf[c("A",NA, "Z")])  # get list(A = 1, <NA> = NULL, <NA> = NULL)
  
  cat("\n")
  flog.info("test extrem cases : modify")
  l[c(1,NA,2)] <- "A" # assgin "A" to 1,2
  lf[c(1,NA,2)] <- "A"
  expect_identical(l, lf[])
  
  expect_error(lf[c(1,NA,2)] <- c("A", "B"), "not allowed in subscripted assignments") 
  #  NAs are not allowed in subscripted assignments
  
  l[c(1,1,1)] <- LETTERS[1:3] # take the last assignment
  lf[c(1,1,1)] <- LETTERS[1:3]
  expect_identical(l, lf[])
  
  l[c(-1)] <- c("A", "B") # A, B to 1:9
  lf[c(-1)] <- c("A", "B")
  expect_identical(l, lf[])
  
  expect_error(l[c(-1, NA)] <- c("A", "B"), "may be mixed with negative subscripts")
  # only 0's may be mixed with negative subscripts
  
  l[c(T, NA, F)] <- "A"  # "A" to 1,4,7,10
  lf[c(T, NA, F)] <- "A"
  expect_identical(l, lf[])
  
  expect_error(l[c(T, NA, F)] <- c("A","B"), "not allowed in subscripted assignment")
  #  NAs are not allowed in subscripted assignment
  
  l[c("Z", NA)] <- c("A","B")  # append "Z" =  "A", <NA> = "B"
  lf[c("Z", NA)] <- c("A","B")
  expect_identical(l, lf[])
  
  l[c("Z", NA)] <- c("C","D") # change "Z" =  "C", append <NA> = "D"
  lf[c("Z", NA)] <- c("C","D")
  expect_identical(l, lf[])
  
  l[c(20)] <- "A" # extend to length 20, with 11-19 as NULL
  lf[c(20)] <- "A"
  # expect_identical(l, lf[]) 
  # they are different since l has "" as new names and lf uses NA as new names
  
  cat("\n")
  flog.info("test extrem cases : modify names, if l has names")
  l <- as.list(1:10)
  names(l) <- LETTERS[1:10]
  lf[[]] <- as.list(1:10)
  names(lf) <- LETTERS[1:10]
  
  names(l)[c(1,NA,3)] <- c("A") # A to 1,3
  names(lf)[c(1,NA,3)] <- c("A")
  expect_identical(l, lf[])
  
  expect_error(names(lf)[c(1,NA,3)] <- c("A","B"), "not allowed in subscripted assignments")
  # NAs are not allowed in subscripted assignments
  
  names(l)[c(1,2,3)] <- c("A", "B") # A to 1, B to 2, A to 3
  names(lf)[c(1,2,3)] <- c("A", "B")
  expect_identical(l, lf[])
  
  expect_error(names(lf)[c(20)] <- "A", "must be the same length as")
  # 'names' attribute [20] must be the same length as the vector [10]
  
  names(l)[-1] <- "B" # B to 1:9
  names(lf)[-1] <- "B"
  expect_identical(l, lf[])
  
  names(l)[c(T,NA,F)] <- "A" # A to 1,4,7,10
  names(lf)[c(T,NA,F)] <- "A"
  expect_identical(l, lf[])
  
  expect_error(names(lf)[c(T,NA,F)] <- c("A","B"), "not allowed in subscripted assignments")
  # NAs are not allowed in subscripted assignments
  
  expect_error(names(lf)["A"] <- "Z", "must be the same length as")
  # 'names' attribute [11] must be the same length as the vector [10]
  
  cat("\n")
  flog.info("test extrem cases : modify names, if l has no names")
  names(l) <- NULL
  names(lf) <- NULL
  
  names(l)[c(1,NA,3)] <- c("A") # A to 1,3, rest NA
  names(lf)[c(1,NA,3)] <- c("A")
  expect_identical(l, lf[])
  
  names(lf) <- NULL
  expect_error(names(lf)[c(1,NA,3)] <- c("A","B"), "not allowed in subscripted assignments")
  # NAs are not allowed in subscripted assignments
  
  names(l) <- NULL
  names(lf) <- NULL
  names(l)[-1] <- "B" # all to <NA> !!!!!!!
  names(lf)[-1] <- "B"
  expect_identical(l, lf[])
  
  names(l) <- NULL
  names(lf) <- NULL
  names(l)[c(T,NA,F)] <- "A" # A to 1, rest NULL !!!!!!
  names(lf)[c(T,NA,F)] <- "A"
  expect_identical(l, lf[])
  
  expect_error(names(l)[c(T,NA,F)] <- c("A","B"), "not allowed in subscripted assignments")
  # NAs are not allowed in subscripted assignments
  
  names(l) <- NULL
  names(lf) <- NULL
  names(l)["A"] <- "Z" # Z to 1,rest NULL !!!!!
  names(lf)["A"] <- "Z"
  expect_identical(l, lf[])
  
})
