getListLength <- function(file) {
    .Call('getListLength', PACKAGE = 'largeList', file)
}

getListName <- function(file) {
    .Call('getListName', PACKAGE = 'largeList', file)
}

readList <- function(file, index = NULL) {
    .Call('readList', PACKAGE = 'largeList', file, index)
}

removeFromList <- function(file, index) {
    res <- .Call('removeFromList', PACKAGE = 'largeList', file, index)
}

saveList <- function(object, file, append = FALSE) {
    res <- .Call('saveList', PACKAGE = 'largeList', object, file, append)
}

