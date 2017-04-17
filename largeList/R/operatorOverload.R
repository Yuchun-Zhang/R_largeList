#' Create a R object and bind with file.
#' 
#' @details 
#' Create a R object of class "largeList" and bind it with a file.
#' If the file exists, the R object will be bound to the file.
#' If the file does not exist or \code{truncate == TRUE}, an empty list will be written into 
#' the given file and then bind it with R object. Later the R object can be used as a normal
#' list and all the manipulation will be done within the file binding to it.
#' @param file Name of file 
#' @param compress \code{TRUE/FALSE} Use compression for elements or not.
#' @param verbose \code{TRUE/FALSE} Print extra info
#' @param truncate \code{TRUE/FALSE} Truncate the file or not.
#' @return A R object of class "largeList"
#' @seealso \code{\link{largeList}}
#' @examples 
#' largelist_object <- getList("example.llo", verbose = TRUE, truncate = TRUE)
#' @export
#' 
getList <- function(file, compress = TRUE, verbose = FALSE, truncate = FALSE){
  if (file.exists(file) && truncate == FALSE) {
    if (verbose == TRUE) {cat(sprintf("file exists, file head and version will be examed."))}
    .Call(C_checkFileAndVersionExternal, file)
    list_object <- list()
    attr(list_object, "largeList_file") <- file
    class(list_object) <- "largeList"
    return(list_object)
  }else{
    if (verbose == TRUE) {cat(sprintf("file does not exist, create an empty list and store into the file."))}
    saveList(object = list(), file = file, append = FALSE, compress = compress)
    list_object <- list()
    attr(list_object, "largeList_file") <- file
    class(list_object) <- "largeList"
    return(list_object)
  }
}

#' Overload of operator [].
#' @details It behaviours the same as a normal list object.
#' @param x A largeList object created by \code{\link{getList}}.
#' @param index  A numeric, logical or character vector.
#' @return A list
#' @seealso \code{\link{largeList}} 
#' @examples
#' largelist_object <- getList("example.llo", truncate = TRUE)
#' largelist_object[[]] <- list("A" = 1, "B" = 2, "C" = 3)  ## assign list to the list file
#' largelist_object[c(1, 2)] ## get list("A" = 1, "B" = 2)
#' largelist_object[c(TRUE, FALSE, TRUE)] ## get list("A" = 1, "C" = 3)
#' largelist_object[c("A", "C")] ## get list("A" = 1, "C" = 3)
#' @export
"[.largeList" <- function(x, index = NULL) {
  res <- readList(file = attr(x, "largeList_file"), index = index)
  return(res)
}

#' Overload of operator [[]].
#' @details It behaviours the same as a normal list object.
#' @param x A largeList object created by \code{\link{getList}}.
#' @param index  A numeric or character vector of length 1.
#' @return A R object.
#' @examples
#' largelist_object <- getList("example.llo", truncate = TRUE)
#' largelist_object[[]] <- list("A" = 1, "B" = 2, "C" = 3)  ## assign list to the list file
#' largelist_object[[1]] ## get 1
#' largelist_object[["B"]] ## get 2
#' @seealso \code{\link{largeList}} 
#' @export
"[[.largeList" <- function(x, index = NULL) {
  if (is.null(index)) {stop("invalid subscript type 'symbol'")}
  if (length(index) > 1) {stop("subscript out of bounds")}
  if (class(index)[1] %in% c("integer","numeric") && index > length(x)) {
    stop("subscript out of bounds")
  }
  if (class(index)[1] %in% c("integer","numeric") && index <= 0) {
    stop("attempt to select less than one element")
  }
  res <- readList(file = attr(x, "largeList_file"), index = index)
  return(res[[1]])
}

#' Overload of operator $.
#' @details It behaviours different from the list object in R. Here \code{x$name} is equivalent to
#' \code{x[["name"]]}, \strong{no partial matching}.
#' @param x A largeList object created by \code{\link{getList}}.
#' @param index A character vector of length 1.
#' @return A R object.
#' @examples
#' largelist_object <- getList("example.llo", truncate = TRUE)
#' largelist_object[[]] <- list("AA" = 1, "B" = 2, "C" = 3)  ## assign list to the list file
#' largelist_object$B ## get 2
#' largelist_object$A ## get NULL, not 1 from "AA" since no partial matching happens.
#' @seealso \code{\link{[[.largeList}} \code{\link{largeList}} 
#' @export
"$.largeList" <- function(x, index) {
  return(x[[index]])  
}

#' Overload of operator []<-.
#' @details It behaviours the same as a normal list object. If \code{index} is not provided, elements in
#' value will be appended to the list file. If \code{value} is \code{NULL}, elements with given indices 
#' will be removed.
#' 
#' @param x A largeList object created by \code{\link{getList}}.
#' @param index  \code{NULL} or a numeric, logical, character vector.
#' @param value \code{NULL}, a vector or a list.
#' @seealso \code{\link{largeList}}
#' @examples
#' largelist_object <- getList("example.llo", truncate = TRUE)
#' largelist_object[] <- list("A" = 1, "B" = 2)  ## append list to the list file
#' largelist_object[] <- list("C" = 3, "D" = 4)  ## append list to the list file
#' largelist_object[1] <- NULL ## remove first element
#' largelist_object["B"] <- NULL ## remove element with name "B"
#' largelist_object[c("C","D")] <- c(5, 6) ## change value
#' largelist_object[2] <- 5 ## change value
#' largelist_object[c(4, 5)] <- list(6, 7) ## append 6, 7 to 4th, 5th position and NULL to 3rd position
#' @export
"[<-.largeList" <- function(x, index = NULL, value) {
  .Call(C_checkList, object = list(value))
  ## if index is null, append value to list object.
  if (is.null(index)) {
    ## vectors are transfered to list since function saveList and modifyInList only support list.
    if (!is.list(value)) {value <- as.vector(value, mode = "list")}
    saveList(object = value, file = attr(x, "largeList_file"), append = TRUE)
  ## if index is not null.
  }else{
    ## if value is null, remove elements.
    if (is.null(value)) {
      removeFromList(file = attr(x, "largeList_file"), index = index)
    ## if value is not null, elements are either appended or substitute the existing elements.
    }else{
      ## vectors are transfered to list since function saveList and modifyInList only supports list.
      if (!is.list(value)) {
        value <- as.vector(value, mode = "list")
      }
      x_length = length(x)
      if (class(index)[1] %in% c("numeric","integer") && !all(index < 0, na.rm = T)) {
        temp_list <- list()
        temp_list[index] <- value
        to_append_value <- temp_list[(1:length(temp_list)) > x_length]
        to_modify_index <- unique(index[index <= x_length])
        to_modify_value <- temp_list[to_modify_index]
      }else 
      if (class(index)[1] %in% c("numeric","integer") && all(index < 0, na.rm = T)) {
        to_modify_index <- index
        to_modify_value <- value
        to_append_value <- integer(0)
      }else 
      if  (class(index)[1] %in% c("character")) {
        temp_list <- list()
        temp_list[index] <- value
        to_append_value <- temp_list[!names(temp_list) %in% names(x) | is.na(names(temp_list))]
        to_modify_index <- names(temp_list)[names(temp_list) %in% names(x) & !is.na(names(temp_list))]
        to_modify_value <- temp_list[to_modify_index]
      }else
      if  (class(index)[1] %in% c("logical")) { 
        to_modify_index <- index
        to_modify_value <- value
        to_append_value <- integer(0)
      }else {
        stop("invalid index type")
      }
      if (length(to_append_value) > 0) { 
        saveList(object = to_append_value, 
                 file = attr(x, "largeList_file"), 
                 append = TRUE)
      }
      if (length(to_modify_index) > 0) { 
        modifyInList(file = attr(x, "largeList_file"), 
                     index = to_modify_index, 
                     object = to_modify_value)
      }
    }
  }
  x <- getList(attr(x, "largeList_file"))
  invisible(x)
}

#' Overload of operator [[]]<-.
#' @details It behaviours the same as a normal list object. If index is not provided, the list file
#' binding with the largeList object will be truncated and elements in
#' value will be saved to the list file. If value is \code{NULL}, element with given index 
#' will be removed.
#' 
#' @param x A largeList object created by \code{\link{getList}}.
#' @param index  \code{NULL} or a numeric, character vector with length 1.
#' @param value \code{NULL}, a vector or a list.
#' @seealso \code{\link{largeList}} 
#' @examples
#' largelist_object <- getList("example.llo", truncate = TRUE)
#' largelist_object[[]] <- list("A" = 1, "B" = 2, "C" = 3)  ## assign list to the list file
#' largelist_object[[1]] <- NULL ## remove first element
#' largelist_object[["B"]] <- NULL ## remove element with name "B"
#' largelist_object[["C"]] <- 5 ## change value
#' largelist_object[[2]] <- 5 ## change value
#' largelist_object[[4]] <- 6 ## append 6 to 4th and NULL to 3rd position
#' @export
"[[<-.largeList" <- function(x, index = NULL, value){
  if (length(index) > 1) {stop("subscript out of bounds")}
  if (is.null(index)) {
    ## vectors are transfered to list since function saveList and modifyInList only support list.
    if (!is.list(value)) {value <- as.vector(value, mode = "list")}
    saveList(object = value, 
             file = attr(x, "largeList_file"), 
             append = FALSE, 
             compress = isListCompressed(attr(x, "largeList_file")))
  }else{
    if (class(index)[1] %in% c("integer","numeric") && index <= 0) {
      stop("attempt to select less than one element")
    }
    x[index] <- value
  }
  x <- getList(attr(x, "largeList_file"))
  invisible(x)
}

#' Overload of operator $<-.
#' @details x$A <- 1 is equivalent to x[["A"]] <- 1
#' @param x A largeList object created by \code{\link{getList}}.
#' @param index  \code{NULL}, a numeric or character vector with lenght 1.
#' @param value \code{NULL}, a vector or a list.
#' @seealso  \code{\link{[[<-.largeList}} \code{\link{largeList}} 
#' @export
"$<-.largeList" <- function(x, index, value){
  x[[index]] <- value
  return(invisible(x))
}

#' Overload of function print.
#' @param x A largeList object created by \code{\link{getList}}.
#' @param ... arguments to be passed.
#' @description  If length of list dose not exceed getOption("max.print"), all elements will be printed,
#' otherwise, elements beyond getOption("max.print") will be omitted.
#' @seealso \code{\link{largeList}} 
#' @examples
#' largelist_object <- getList("example.llo", truncate = TRUE)
#' largelist_object[[]] <- list("A" = 1, "B" = 2, "C" = 3)  ## assign list to the list file
#' print(largelist_object) ## print to screen
#' largelist_object ## print to screen
#' @export
print.largeList <- function(x, ...) {
  x_length = length(x)
  if (x_length <= getOption("largeList.max.print")) {
    print(x[])
  }else{
    print(x[1:getOption("largeList.max.print")])
    cat(sprintf("[ reached getOption(\"largeList.max.print\") -- omitted %d entries ]",
                x_length - getOption("largeList.max.print")))
  }
}

#' Overload of function length.
#' @param x A largeList object created by \code{\link{getList}}.
#' @return An integer.
#' @description Get the length of list stored in file.
#' @seealso \code{\link{largeList}} 
#' @examples
#' largelist_object <- getList("example.llo", truncate = TRUE)
#' largelist_object[[]] <- list("A" = 1, "B" = 2, "C" = 3)  ## assign list to the list file
#' length(largelist_object) ## get 3
#' @export
length.largeList <- function(x) {
  return(getListLength(attr(x, "largeList_file")))
}

#' Overload of function length<-.
#' @param x A largeList object created by \code{\link{getList}}.
#' @param value An integer number
#' @description Set the length of list stored in file.
#' @seealso \code{\link{largeList}} 
#' @examples
#' largelist_object <- getList("example.llo", truncate = TRUE)
#' largelist_object[[]] <- list("A" = 1, "B" = 2, "C" = 3)  ## assign list to the list file
#' length(largelist_object) <- 5 ## get list("A" = 1, "B" = 2, "C" = 3, NULL, NULL)
#' @export
"length<-.largeList" <- function(x, value) {
  x[value] <- list(NULL)
  return(x)
}

#' Overload of function names.
#' @param x A largeList object created by \code{\link{getList}}.
#' @return A character vector.
#' @description Get the names of elements stored in list file.
#' @seealso \code{\link{largeList}} 
#' @examples
#' largelist_object <- getList("example.llo", truncate = TRUE)
#' largelist_object[[]] <- list("A" = 1, "B" = 2, "C" = 3)  ## assign list to the list file
#' names(largelist_object) ## get c("A", "B", "C")
#' @export
names.largeList <- function(x) {
  return(getListName(attr(x, "largeList_file")))
}

#' Overload of operator names<-.
#' @details It behaviours the same as a normal list object. 
#' 
#' @param x A largeList object created by \code{\link{getList}}.
#' @param value A character vector.
#' @seealso \code{\link{largeList}} 
#' @examples 
#' largelist_object <- getList("example.llo", truncate = TRUE)
#' largelist_object[[]] <- list("A" = 1, "B" = 2, "C" = 3)  ## assign list to the list file
#' names(largelist_object) <- c("AA", "BB", "CC")
#' @export
"names<-.largeList" <- function(x, value) {
  if (is.null(value)) {
    modifyNameInList(file = attr(x, "largeList_file"), index = NULL, name = NULL)
  }else {
    x_length <- length(x)
    if (length(value) > x_length) {
      stop(sprintf("'names' attribute [%d] must be the same length as the vector [%d]", length(value), x_length))  
    }
    if (length(value) <= x_length) {
      modifyNameInList(file = attr(x, "largeList_file"), index = seq_along(value), name = value)
    }    
  }
  return(x)
}


.onLoad <- function(libname, pkgname) {
  options(list(largeList.max.print = 100, largeList.report.progress = TRUE))
}