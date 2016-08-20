#' Create a R object and bind with file.
#' 
#' @details 
#' Create a R object of class "largeList" and bind it with a file.
#' If the file exists, the R object will get the length of list and the names of objects.
#' If the file does not exist or \code{new == TRUE}, an empty list will be written into 
#' the given file and then bind it.
#' @param file Name of file 
#' @param verbose \code{TRUE/FALSE} Extra info
#' @param truncate \code{TRUE/FALSE} Truncate the file or not.
#' @return a R object of class "largeList"
#' @seealso \code{\link{largeList}}
#' @examples 
#' largeListObject <- getList("example.llo", verbose = TRUE, truncate = TRUE)
#' @export
#' 
getList <- function(file, verbose = FALSE, truncate = FALSE){
  if (file.exists(file) && truncate == FALSE){
    if (verbose == TRUE) {cat(sprintf("file exists, get meta data."))}
    listObject <-list()
    listObject$file <- file
    listObject$length <- getListLength(file)
    listObject$name <- getListName(file)
    class(listObject) <- "largeList"
    return(listObject)
  }else{
    if (verbose == TRUE) {cat(sprintf("file does not exist, create an empty list and store into the file."))}
    saveList(object = list(), file = file, append = FALSE)
    listObject <-list()
    listObject$file <- file
    listObject$length <- getListLength(file)
    listObject$name <- getListName(file)
    class(listObject) <- "largeList"
    return(listObject)
  }
}