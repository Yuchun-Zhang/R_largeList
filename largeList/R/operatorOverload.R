#' Create a R object and bind with file.
#' 
#' @details 
#' Create a R object of class "largeList" and bind it with a file.
#' If the file exists, the R object will be bound to the file.
#' If the file does not exist or \code{truncate == TRUE}, an empty list will be written into 
#' the given file and then bind it.
#' @param file Name of file 
#' @param verbose \code{TRUE/FALSE} Extra info
#' @param truncate \code{TRUE/FALSE} Truncate the file or not.
#' @return A R object of class "largeList"
#' @seealso \code{\link{largeList}}
#' @examples 
#' largelist_object <- getList("example.llo", verbose = TRUE, truncate = TRUE)
#' @export
#' 
getList <- function(file, verbose = FALSE, truncate = FALSE){
  if (file.exists(file) && truncate == FALSE){
    if (verbose == TRUE) {cat(sprintf("file exists, check file / version."))}
    .Call('checkFileAndVersionExternal', PACKAGE = 'largeList', file)
    list_object <-list()
    list_object$file <- file
    class(list_object) <- "largeList"
    return(list_object)
  }else{
    if (verbose == TRUE) {cat(sprintf("file does not exist, create an empty list and store into the file."))}
    saveList(object = list(), file = file, append = FALSE)
    list_object <-list()
    list_object$file <- file
    class(list_object) <- "largeList"
    return(list_object)
  }
}

#' Overload of operator [].
#' @details It behaviours the same as a normal list object.
#' @param x A largeList object created by \code{\link{getList}}.
#' @param index  A numeric vector or a character vecter.
#' @return A list
#' @seealso \code{\link{largeList}} 
#' @export
"[.largeList" <- function(x, index = NULL) {
  res <- readList(file = x$file, index = index)
  return(res)
}

#' Overload of operator [[]].
#' @details It behaviours the same as a normal list object.
#' @param x A largeList object created by \code{\link{getList}}.
#' @param index  A numeric vector or a character vecter of length 1.
#' @return A R object.
#' @seealso \code{\link{largeList}} 
#' @export
"[[.largeList" <- function(x, index = NULL) {
  if (length(index) > 1) {stop("subscript out of bounds")}
  res <- readList(file = x$file, index = index)
  return(res[[1]])
}

#' Overload of operator []<-.
#' @details It behaviours the same as a normal list object. If index is not provided, elements in
#' value will be appended to the list file. If value is \code{NULL}, elements with given indices 
#' will be removed.
#' 
#' @param x A largeList object created by \code{\link{getList}}.
#' @param index  \code{NULL}, a numeric vector or a character vecter.
#' @param value \code{NULL}, a vector or a list.
#' @seealso \code{\link{largeList}} 
#' @export
"[<-.largeList" <- function(x, index = NULL, value) {
  ## if index is null, append value to list object.
  if (is.null(index)) {
    ## vectors are transfered to list since function saveList and modifyInList only support list.
    if (class(value) != "list") {value <- as.list(value)}
    saveList(object = value, file = x$file, append = TRUE)
  ## if index is not null.
  }else{
    ## if value is null, remove elements.
    if (is.null(value)) {
      removeFromList(file = x$file, index = index)
    ## if value is not null, elements are either appended or substitute the existing elements.
    }else{
      ## vectors are transfered to list since function saveList and modifyInList only supports list.
      if (class(value) != "list") {value <- as.list(value)}
      ## if index is of type integer/numeric
      if (class(index) %in% c("numeric","integer")) {
        x_length <- length(x)
        index <- as.integer(index)
        if (length(index) %% length(value) != 0){
          warning("number of items to replace is not a multiple of replacement length")
        }
        to_append_index <- index[index > x_length]
        to_modify_index <- index[index <= x_length]
        if (length(to_append_index) > 0) {
          ## here it should be considered that the number of items to replace is not a
          ## multiple of replacement length.
          to_append_value_index <- which(index > x_length) %% length(value)
          to_append_value_index[to_append_value_index == 0] <- length(value)
          to_append_value <- rep(list(NULL), max(to_append_index) - x_length)
          to_append_value[to_append_index - x_length] <- value[to_append_value_index]
        }
        if (length(to_modify_index) > 0) {
          to_modify_value_index <- which(index <= x_length) %% length(value)
          to_modify_value_index[to_modify_value_index == 0] <- length(value)
          to_modify_value <- value[to_modify_value_index]
        }
      }else
      if  (class(index) %in% c("character")){
        if (length(index) %% length(value) != 0) {
          warning("number of items to replace is not a multiple of replacement length")
        }
        x_name <- names(x)
        to_append_index = index[!(index %in% x_name)]
        to_modify_index = index[index %in% x_name]
        if (length(to_append_index) > 0) {
          to_append_value_index <-  which(!(index %in% x_name)) %% length(value)
          to_append_value_index[to_append_value_index == 0] <- length(value)
          to_append_value <- value[to_append_value_index]
          names(to_append_value) <- index[to_append_index]
        }
        if (length(to_modify_index) > 0) {
          to_modify_value_index <- which((index %in% x_name))%% length(value)
          to_modify_value_index[to_modify_value_index == 0] <- length(value)
          to_modify_value <- value[to_modify_value_index]
        }
      }else{
        stop("invalid index type")
      }
      if (length(to_append_index) > 0) { saveList(object = to_append_value, file = x$file, append = TRUE)}
      if (length(to_modify_index) > 0) { modifyInList(file = x$file, index = to_modify_index, object = to_modify_value)}
    }
  }
  x <- getList(x$file)
  invisible(x)
}

#' Overload of operator [[]]<-.
#' @details It behaviours the same as a normal list object. If index is not provided, the list file
#' binding with the largeList object will be truncated and elements in
#' value will be saved to the list file. If value is \code{NULL}, element with given index 
#' will be removed.
#' 
#' @param x A largeList object created by \code{\link{getList}}.
#' @param index  \code{NULL}, a numeric vector or a character vecter.
#' @param value \code{NULL}, a vector or a list.
#' @seealso \code{\link{largeList}} 
#' @export
"[[<-.largeList" <- function(x, index = NULL, value){
  if (length(index) > 1) {stop("subscript out of bounds")}
  if (is.null(index)){
    ## vectors are transfered to list since function saveList and modifyInList only support list.
    if (class(value) != "list") {value <- as.list(value)}
    saveList(object = value, file = x$file, append = FALSE)
  }else{
    x[index] <- value
  }
  x <- getList(x$file)
  invisible(x)
}


#' Overload of function print.
#' @param x A largeList object created by \code{\link{getList}}.
#' @param ... arguments to be passed.
#' @description  If length of list dose not exceed getOption("max.print"), all elements will be printed,
#' otherwise, elements beyond getOption("max.print") will be omitted.
#' @seealso \code{\link{largeList}} 
#' @export
print.largeList <- function(x, ...) {
  x_length = length(x)
  if (x_length <= getOption("max.print")) {
    print(x[])
  }else{
    print(x[1:getOption("max.print")])
    cat(sprintf("[ reached getOption(\"max.print\") -- omitted $d entries ]",
                x_length - getOption("max.print")))
  }
}

#' Overload of function length.
#' @param x A largeList object created by \code{\link{getList}}.
#' @return An integer.
#' @description Get the length of list stored in file.
#' @seealso \code{\link{largeList}} 
#' @export
length.largeList <- function(x) {
  return(getListLength(x$file))
}

#' Overload of function names.
#' @param x A largeList object created by \code{\link{getList}}.
#' @return A character vector.
#' @description Get the names of elements stored in list file.
#' @seealso \code{\link{largeList}} 
#' @export
names.largeList <- function(x) {
  return(getListName(x$file))
}

#' Overload of operator names<-.
#' @details It behaviours the same as a normal list object. 
#' 
#' @param x A largeList object created by \code{\link{getList}}.
#' @param value A character vector.
#' @seealso \code{\link{largeList}} 
#' @export
"names<-.largeList" <- function(x, value) {
  x_length <- length(x)
  if (length(value) > x_length) {
    stop(sprintf("'names' attribute [%d] must be the same length as the vector [%d]", length(value), x$length))  
  }
  if (length(value) <= x_length) {
    modifyNameInList(file = x$file, index = seq_along(value), name = value)
  }
  return(getList(x$file))
}


