#' largeList: Serialization Interface for Large List Objects
#' 
#' @description 
#' Functions to write or append a R list to a file, read or remove elements from it 
#' without restoring the whole list.
#' 
#' @details
#' 
#' R objects will be serialized with an uncompressed non-ascii little-endian format, 
#' which is similar to \code{\link{saveRDS}}. Two ordered tables are created at 
#' the end of data for quick lookups, one for indices and one for names. Notice that, 
#' all the names will be truncated to 16 characters. \cr
#' 
#' Given indices or names of elements, positions will be directly extracted or extracted 
#' via binary search within the name-position table. Then required elements are located and 
#' unserialized. Therefore it will not restore the whole list into memory. \cr
#' 
#' With overloads of operators, list objects stored in files can be manipulated as simply as
#' the normal list objects in R.
#' 
#' In the current version, only basic data types are supported, including NULL, integer, 
#' numeric, character, complex, raw, logic, factor, list, matrix, array and data.frame. 
#' Types like function, data.table are not supported. \cr
#' 
#' The supported maximum size of R objects is \code{2^31 -1}, the supported maximum file 
#' size is \code{2^63 -1} bytes. \cr
#' 
#' Following functions are provided: 
#' \itemize{
#' \item{\code{\link{saveList}}}  Save or append elements to a list file.
#' \item{\code{\link{readList}}}  Get elements from a list file.
#' \item{\code{\link{removeFromList}}} Remove elements from a list file.
#' \item{\code{\link{modifyInList}}} Modify elements in a list file.
#' \item{\code{\link{modifyNameInList}}} Modify names of elements in a list file.
#' \item{\code{\link{getListName}}} Get number of elements in a list file.
#' \item{\code{\link{getListLength}}} Get names of elements in a list file.
#' }
#' Some operators / functions are overloaded. 
#' \itemize{
#' \item{\code{\link{getList}}} Bind a R object with a list file.
#' \item{\code{\link{[.largeList}}} Get elements.
#' \item{\code{\link{[[.largeList}}} Get element.
#' \item{\code{\link{[<-.largeList}}} If index provided, it modifies, appends or removes the elements
#'  with given indices, otherwise it appends value to list. 
#' \item{\code{\link{[[<-.largeList}}}  If index provided, it modifies, appends or removes the element
#'  with given index, otherwise it saves value to list. 
#' \item{\code{\link{length.largeList}}} Get length of list stored in file.
#' \item{\code{\link{names.largeList}}} Get names of elements stored in file.

#' }
#' @docType package
#' @name largeList
#' @useDynLib largeList
NULL