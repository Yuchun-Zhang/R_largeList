#' Get number of elements in a list file.
#' @param file Name of file.
#' @return An integer.
#' @seealso \code{\link{largeList}}
#' @examples 
#' list1 <- list("A" = c(1,2), "B" = "abc", list(1, 2, 3))
#' saveList(list1, "example.llo")
#' getListLength("example.llo")
#' @export
getListLength <- function(file) {
    .Call('getListLength', PACKAGE = 'largeList', file)
}

#' Get names of elements in a list file.
#' @param file Name of file.
#' @return A charater vector.
#' @seealso \code{\link{largeList}}
#' @examples 
#' list1 <- list("A" = c(1,2), "B" = "abc", list(1, 2, 3))
#' saveList(list1, "example.llo")
#' getListName("example.llo")
#' @export
getListName <- function(file) {
    .Call('getListName', PACKAGE = 'largeList', file)
}

#' Get elements from a list file.
#' @details  
#' If no index provided, the whole list will be read. Given index could be a numeric (integer) 
#' vector or a character vector representing the names. If there exist more then one elements 
#' corresponding to a given name, , the first matched one will be returned. If there are no elements 
#' with given name, \code{NULL} will be returned. \cr
#' Files created by \code{\link{saveRDS}} can't be read.
#' @param file Name of file.
#' @param index \code{NULL}, a numeric vector or a character vecter.
#' @return A list object.
#' @seealso \code{\link{largeList}}
#' @examples 
#' list1 <- list("A" = c(1,2), "B" = "abc", list(1, 2, 3))
#' saveList(list1, "example.llo")
#' readList("example.llo")
#' readList("example.llo",c(1, 3))
#' readList("example.llo",c("A", "B"))
#' @export
readList <- function(file, index = NULL) {
    .Call('readList', PACKAGE = 'largeList', file, index)
}

#' Remove elements from a list file.
#' @details 
#' It removes elements with given indices or names. This function may relocate all the data 
#' in the stored file, thus can be very slow! Please consider to call this function 
#' batchwise instead of one by one.
#' @param file Name of file.
#' @param index  A numeric vector or a character vecter.
#' @return \code{TRUE} if no error occurs.
#' @seealso \code{\link{largeList}}
#' @examples 
#' list1 <- list("A" = c(1,2), "B" = "abc", list(1, 2, 3))
#' saveList(list1, "example.llo")
#' removeFromList("example.llo", c("A"))
#' removeFromList("example.llo", c(2))
#' @export
removeFromList <- function(file, index) {
    res <- .Call('removeFromList', PACKAGE = 'largeList', file, index)
}

#' Save or append elements to a list file.
#' @details 
#' Save or append a list with / without names to a file.
#' Notice that, all the names will be truncated to 16 characters. The rest attributes of lists
#' will be discarded. \cr
#' The generated file is not readable by \code{\link{readRDS}}.
#' @param object A list object to save or append.
#' @param file Name of file.
#' @param append \code{TRUE/FALSE}, \code{TRUE} refers to truncating and saving. 
#' \code{FALSE} refers to appending.
#' @return \code{TRUE} if no error occurs.
#' @seealso \code{\link{largeList}}
#' @examples 
#' list1 <- list("A" = c(1,2), "B" = "abc", list(1, 2, 3))
#' saveList(list1, "example.llo")
#' @export

saveList <- function(object, file, append = FALSE) {
    res <- .Call('saveList', PACKAGE = 'largeList', object, file, append)
}

#' Modify elements in a list file.
#' @details 
#' It modifies elements with given indices by replacement values provided in parameter object. If the length 
#' of replacement values is shorter than the length of indices, values will be used circularly. This function may 
#' relocate all the data in the stored file, thus can be very slow! Please consider to call this
#' function batchwise instead of one by one.
#' @param file Name of file.
#' @param index A numeric vector or a character vecter.
#' @param object A list consisting of replacement values.
#' @return \code{TRUE} if no error occurs.
#' @seealso \code{\link{largeList}}
#' @examples 
#' list1 <- list("A" = c(1,2), "B" = "abc", list(1, 2, 3))
#' saveList(list1, "example.llo")
#' modifyInList("example.llo", c(1,2), list("AA","BB"))
#' modifyInList("example.llo", c("AA","BB"), list("A","B"))
#' @export
modifyInList <- function(file, index, object) {
  res <- .Call('modifyInList', PACKAGE = 'largeList', file, index, object)
}

#' Modify names of elements in a list file.
#' @details 
#' Modify naems of elements with given indices by replacement values provided in parameter name If the length 
#' of replacement values is shorter than the length of indices, values will be used circularly. 
#' @param file Name of file.
#' @param index A numeric vector.
#' @param name A character vector consisting replacement names.
#' @return \code{TRUE} if no error occurs.
#' @seealso \code{\link{largeList}}
#' @examples 
#' list1 <- list("A" = c(1,2), "B" = "abc", list(1, 2, 3))
#' saveList(list1, "example.llo")
#' modifyNameInList("example.llo", c(1,2), c("AA","BB"))
#' @export
modifyNameInList <- function(file, index, name) {
  res <- .Call('modifyNameInList', PACKAGE = 'largeList', file, index, name)
}


