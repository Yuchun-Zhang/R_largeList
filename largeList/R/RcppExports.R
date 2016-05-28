#' Save, append or read large lists to/from given file. 
#'
#' Two functions \code{saveLargeList} & \code{readLargeList} are provided,
#' which makes it possible to append to or read from lists without loading 
#' the whole lists into memory.
#'
#' Function \code{saveLargeList} is mainly based on R functions 
#' \code{\link{serialize}} and \code{\link{writeBin}}. The output contains the 
#' uncompressed non-ascii result of \code{\link{saveRDS}} and positions of all elements in 
#' the list. The generated files can also be read by \code{\link{readRDS}}.
#' 
#' Function \code{readLargeList} is mainly based on R functions \code{\link{unserialize}}
#' and \code{\link{readBin}}. Given indices of the elements, positions will be extracted 
#' first, then required elements are located and unserialized. Files created by 
#' \code{\link{saveRDS}} can't be read.
#' 
#' @param object A list to save or append.
#' @param fileName The name of the file where the R object is saved to or read from.
#' @param append Create a new file or append to an existing file. Default value is FALSE. 
#' If append is TRUE but the file does not exist, a new file will be created.
#' @param index A numeric vector representing index or indicies of required element(s) of the list. 
#' If not given, the whole list will be read.
#' 
#' @return For \code{saveLargeList}, \code{TRUE} if successfully.
#' 
#'         For \code{readLargeList},  a List.
#' @seealso \code{\link{serialize}}, \code{\link{unserialize}}, 
#' \code{\link{writeBin}} and \code{\link{readBin}}.
#' @export
#' @name largeList
#' @author Yuchun Zhang
#' @examples
#' ## save to file
#' list1 <- list(c(1,2), "abc", list(1, 2, 3))
#' saveLargeList(list1, "example.rds")
#' ## append to file
#' list2 <- list("d", "e")
#' saveLargeList(list2, "example.rds", append = TRUE)
#' ## read from file
#' readLargeList("example.rds")
#' readLargeList("example.rds",1)
#' readLargeList("example.rds",c(1,3,5))

saveLargeList <- function(object, fileName, append = FALSE) {
.Call('largeList_saveLargeList', PACKAGE = 'largeList', object, fileName, append)
}

#' @rdname largeList
readLargeList <- function(fileName, index = NULL) {
    .Call('largeList_readLargeList', PACKAGE = 'largeList', fileName, index)
}



