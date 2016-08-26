#' Overload of operator [].
#' @details It behaviours the same as a normal list object.
#' @param largeListObject A largeList object created by \code{\link{getList}}.
#' @param index  A numeric vector or a character vecter.
#' @return a list
#' @seealso \code{\link{largeList}} 
#' @export
"[.largeList" <- function(largeListObject, index = NULL){
  res <- readList(file =largeListObject$file, index = index)
  return(res)
}

#' Overload of operator [[]].
#' @details It behaviours the same as a normal list object.
#' @param largeListObject A largeList object created by \code{\link{getList}}.
#' @param index  A numeric vector or a character vecter of length 1.
#' @return a R object.
#' @seealso \code{\link{largeList}} 
#' @export
"[[.largeList" <- function(largeListObject, index = NULL){
  if (length(index) > 1) {stop("subscript out of bounds")}
  res <- readList(file = largeListObject$file, index = index)
  return(res[[1]])
}

#' Overload of operator []<-.
#' @details It behaviours the same as a normal list object. If index is not provided, elements in
#' value wiil be appended to the list file. If value is \code{NULL}, elements with given indices 
#' willbe removed.
#' 
#' @param largeListObject A largeList object created by \code{\link{getList}}.
#' @param index  \code{NULL}, a numeric vector or a character vecter.
#' @param value \code{NULL}, a vector or a list.
#' @return a largeList object
#' @seealso \code{\link{largeList}} 
#' @export
"[<-.largeList" <- function(largeListObject, index = NULL, value){
  ## if index is null, append value to list object.
  if (is.null(index)){
    ## vectors are transfered to list since function saveList and modifyInList only support list.
    if (class(value) != "list") {value <- as.list(value)}
    saveList(object = value, file = largeListObject$file, append = TRUE)
  ## if index is not null.
  }else{
    ## if value is null, remove elements.
    if (is.null(value)){
      removeFromList(file = largeListObject$file, index = index)
    ## if value is not null, elements are either appended or substitute the existing elements.
    }else{
      ## vectors are transfered to list since function saveList and modifyInList only supports list.
      if (class(value) != "list") {value <- as.list(value)}
      ## if index is of type integer/numeric
      if (class(index) %in% c("numeric","integer")){
        index <- as.integer(index)
        if (length(index) %% length(value) !=0){
          warning("number of items to replace is not a multiple of replacement length")
        }
        toAppendIndex <- index[index > largeListObject$length]
        toModifyIndex <- index[index <= largeListObject$length]
        if (length(toAppendIndex) >0){
          ## here it should be considered that the number of items to replace is not a 
          ## multiple of replacement length.
          toAppendValueIndex <- which(index > largeListObject$length) %% length(value)
          toAppendValueIndex[toAppendValueIndex == 0] <- length(value)
          toAppendValue <- rep(list(NULL), max(toAppendIndex) - largeListObject$length)
          toAppendValue[toAppendIndex - largeListObject$length] <- value[toAppendValueIndex]
        }
        if (length(toModifyIndex) >0){
          toModifyValueIndex <- which(index <= largeListObject$length) %% length(value)
          toModifyValueIndex[toModifyValueIndex == 0] <- length(value)
          toModifyValue <- value[toModifyValueIndex]
        }
      }else
      if  (class(index) %in% c("character")){
        if (length(index) %% length(value) !=0){
          warning("number of items to replace is not a multiple of replacement length")
        }
        toAppendIndex = index[!(index %in% largeListObject$name)]
        toModifyIndex = index[index %in% largeListObject$name]
        if (length(toAppendIndex) >0){
          toAppendValueIndex <-  which(!(index %in% largeListObject$name))%% length(value)
          toAppendValueIndex[toAppendValueIndex == 0] <- length(value)
          toAppendValue <- value[toAppendValueIndex]
          names(toAppendValue) <- index[toAppendIndex]
        }
        if (length(toModifyIndex) >0){
          toModifyValueIndex <- which((index %in% largeListObject$name))%% length(value)
          toModifyValueIndex[toModifyValueIndex == 0] <- length(value)
          toModifyValue <- value[toModifyValueIndex]
        }
      }else{
        stop("invalid index type")
      }
      if (length(toAppendIndex) > 0){ saveList(object = toAppendValue, file = largeListObject$file, append = TRUE)}
      if (length(toModifyIndex) > 0){ modifyInList(file = largeListObject$file, index = toModifyIndex, object = toModifyValue)}
    }
  }
  return(getList(largeListObject$file))
}

#' Overload of operator [[]]<-.
#' @details It behaviours the same as a normal list object. If index is not provided, the list file
#' binding with the largeList object will be truncated and elements in
#' value wiil be saved to the list file. If value is \code{NULL}, elements with given index 
#' willbe removed.
#' 
#' @param largeListObject A largeList object created by \code{\link{getList}}.
#' @param index  \code{NULL}, a numeric vector or a character vecter.
#' @param value \code{NULL}, a vector or a list.
#' @return a largeList object
#' @seealso \code{\link{largeList}} 
#' @export
"[[<-.largeList" <- function(largeListObject, index = NULL, value){
  if (length(index) > 1) {stop("subscript out of bounds")}
  if (is.null(index)){
    ## vectors are transfered to list since function saveList and modifyInList only support list.
    if (class(value) != "list") {value <- as.list(value)}
    saveList(object = value, file = largeListObject$file, append = FALSE)
  }else{
    largeListObject[index] <- value
  }
  return(getList(largeListObject$file))
}


#' Overload of function print.
#' @param x A largeList object created by \code{\link{getList}}.
#' @param ... arguments to be passed.
#' @description  If length of list dose not exceed getOption("max.print"), all elements will be printed,
#' otherwise, elements beyond getOption("max.print") will be omitted.
#' @seealso \code{\link{largeList}} 
#' @export
print.largeList <- function(x, ...){
  if (x$length <= getOption("max.print")){
    print(x[])
  }else{
    print(x[1:getOption("max.print")])
    cat(sprintf("[ reached getOption(\"max.print\") -- omitted $d entries ]",
                x$length - getOption("max.print")))
  }
}

#' Overload of function length.
#' @param x A largeList object created by \code{\link{getList}}.
#' @description Get the length of list stored in file.
#' @seealso \code{\link{largeList}} 
#' @export
length.largeList <- function(x){
  return(getListLength(x$file))
}

#' Overload of function names.
#' @param x A largeList object created by \code{\link{getList}}.
#' @description Get the names of elements stored in list file.
#' @seealso \code{\link{largeList}} 
#' @export
names.largeList <- function(x){
  return(getListName(x$file))
}

#' Overload of operator names<-.
#' @details It behaviours the same as a normal list object. 
#' 
#' @param x A largeList object created by \code{\link{getList}}.
#' @param value A character vector.
#' @return a largeList object
#' @seealso \code{\link{largeList}} 
#' @export
"names<-.largeList" <- function(x, value){
  if (length(value) > x$length){
    stop(sprintf("'names' attribute [%d] must be the same length as the vector [%d]", length(value), x$length))  
  }
  if (length(value) <= x$length){
    modifyNameInList(file = x$file, index = seq_along(value), name = value)
  }
  return(getList(x$file))
}


