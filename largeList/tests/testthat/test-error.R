context("error")

## TODO: Rename context
## TODO: Add more tests

test_that("multiplication works", {
  skip_on_cran()
  skip_on_travis()
  source(paste0(path.package("largeList"), "/tests/config.R"))
  if (!ready_to_test) return(invisible(TRUE))
  if (original) {
    ####
    flog.info("Test : object is not list")
    expect_error(saveList(1, "./example.llo") , "object is not a list.")
    
    saveList(list(1), "example.llo")
    expect_error(modifyInList("./example.llo", 1, 1) , "object is not a list.")
    cat("\n")
    
    ####
    flog.info("Test : index Test")
    expect_error(readList("example.llo", c(-1, 1)), 
                 "only 0's may be mixed with negative subscripts")
    expect_error(removeFromList("example.llo", c(-1, 1)), 
                 "only 0's may be mixed with negative subscripts")    
    expect_error(modifyInList("example.llo", c(-1, 1), list(1)), 
                 "only 0's may be mixed with negative subscripts") 
    cat("\n")
    
    ####
    flog.info("Test : file does not exist")
    suppressWarnings(file.remove("notexist"))
    expect_error(saveList(list(1), "notexist", append = T),
                 "file does not exist.")
    expect_error(readList("notexist"),
                 "file does not exist.")
    expect_error(removeFromList("notexist",1),
                 "file does not exist.")
    expect_error(modifyInList("notexist",1,list(1)),
                 "file does not exist.")
    cat("\n")
    
    ####
    flog.info("Test : file is not invalid form")
    saveRDS(list(), "rdsFile.rds")
    expect_error(saveList(list(1), "rdsFile.rds", append = T), "unkown file format!")
    expect_error(readList("rdsFile.rds"), "unkown file format!")
    expect_error(removeFromList("rdsFile.rds",1), "unkown file format!")
    expect_error(modifyInList("rdsFile.rds",1, list(1)), "unkown file format!")
    file.remove("rdsFile.rds")
    
    saveList(list(), "wrongVersion.llo")
    write_ptr = file("wrongVersion.llo", "r+b") 
    seek(write_ptr, where = 10, origin = "start", rw = "write")
    seek(write_ptr, where = 10, origin = "start", rw = "write")
    writeBin(as.raw(c(1*16,0,0,0)), write_ptr, endian = "big")
    close(write_ptr)
    expect_error(saveList(list(1), "wrongVersion.llo", append = T), 
                 "can only read files created by version above ")
    expect_error(readList("wrongVersion.llo"), 
                 "can only read files created by version above ")
    expect_error(removeFromList("wrongVersion.llo", 1), 
                 "can only read files created by version above ")
    expect_error(modifyInList("wrongVersion.llo", 1, list(1)), 
                 "can only read files created by version above ")
    file.remove("wrongVersion.llo")
    cat("\n")
    
    ####
    flog.info("Test : data type not supported")
    listWithFun <- list(sum)
    expect_error(saveList(listWithFun, "someFile"), 
                 "Data type not supported. Please check \\?largeList")
    expect_error(modifyInList("example.llo", 1, listWithFun), 
                 "Data type not supported. Please check \\?largeList")
    file.remove("example.llo")
    cat("\n")
    
  } else {
    flog.info("Test : index Test")
    lf <- getList("example.llo")
    lf[[]] <- list(1,2,3)
    expect_error(lf[[]], "invalid subscript type 'symbol'")
    expect_error(lf[[c(1,2)]], "subscript out of bounds")
    expect_error(lf[[c(4)]], "subscript out of bounds")
    expect_error(lf[[c(-1)]], "attempt to select less than one element")
    expect_error(lf[[0]], "attempt to select less than one element")
    
    expect_error(lf[[c(1,2)]] <- 1, "subscript out of bounds")
    expect_error(lf[[c(-1)]] <- 1, "attempt to select less than one element")
    expect_error(lf[[0]] <- 1, "attempt to select less than one element")
    cat("\n")
    
    ####
    flog.info("Test : file is not invalid form")
    saveRDS(list(), "rdsFile.rds")
    expect_error(lf <- getList("rdsFile.rds"), "unkown file format!")
    file.remove("rdsFile.rds")
    
    saveList(list(), "wrongVersion.llo")
    write_ptr = file("wrongVersion.llo", "r+b") 
    seek(write_ptr, where = 10, origin = "start", rw = "write")
    seek(write_ptr, where = 10, origin = "start", rw = "write")
    writeBin(as.raw(c(1*16,0,0,0)), write_ptr, endian = "big")
    close(write_ptr)
    expect_error(lf <- getList("wrongVersion.llo"), 
                 "can only read files created by version above")
    file.remove("wrongVersion.llo")
    cat("\n")
    
    ####
    flog.info("Test : data type not supported")
    listWithFun <- list(sum)
    expect_error(lf[] <-  listWithFun, 
                 "Data type not supported. Please check \\?largeList")
    expect_error(lf[1] <-  listWithFun, 
                 "Data type not supported. Please check \\?largeList")    
    cat("\n")
    file.remove("example.llo")
  }
})
