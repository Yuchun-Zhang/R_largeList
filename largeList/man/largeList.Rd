\name{largeList}
\alias{largeList}
\alias{readList}
\alias{saveList}
\alias{removeFromList}
\alias{getListName}
\alias{getListLength}
\title{Serialization Interface for Large List Objects}
\usage{
saveList(object, file, append = FALSE)
readList(file, index = NULL)
removeFromList(file,index)
getListName(file)
getListLength(file)
}
\arguments{
\item{object}{A list to save or to append.}

\item{file}{The name of the file where the R object is saved to or read from.}

\item{append}{Create a new file or append to an existing file. Default value is \code{FALSE}. 
If append is \code{TRUE} but the file does not exist, an error will be displayed.}

\item{index}{A numeric/integer vector representing indicies of required elements or a character vector representing the names.For readList function, if index is not given, the whole list will be read.}
}
\value{
For \code{saveList}, \code{TRUE} if successfully.

For \code{readList},  a list of required elements.

For \code{removeFromList}, \code{TRUE} if successfully.

For \code{getListName}, a character vector containing names of all elements.

For \code{getListLength}, an integer representing the length of the list

}
\description{
Functions to write or append a R list to a file, read or remove elements from it without restoring the whole list.
}
\details{
In the current version, only basic data types are supported, including NULL, integer, numeric, character, complex, raw, logic, factor, list, matrix, array and data.frame. Types like function, data.table are not supported.

The supported maximum size of R objects is \code{2^31 -1}, the supported maximum file size is \code{2^63 -1} bytes.

Function \code{saveLargeList} uses an uncompressed non-ascii little-endian serialization method which is similar to \code{\link{saveRDS}}. Two ordered tables are created at the end of data for quick lookup, one for sequenced and one for the names. Notice that, all the names will be truncated to 16 characters. The generated files can also be read by \code{\link{readRDS}}, but without names.

Function \code{readLargeList} is provided to fetch required elements without restoring the whole list into memory. Given sequence numbers or names of the elements, positions will be directly extracted 
or extracted via binary search within the corresponding tables. Then required elements are located and unserialized. If for one given names, there exist more then one elements related, the first matched one will be returned. If there is no elements with the name, \code{NULL} will be returned. Files created by \code{\link{saveRDS}} can't be read.

Function \code{removeFromList} removes elements with given indices or names. This function may relocate all the data in the stored file, thus can be very slow! Please consider to call the function batchwise instead of one by one.

Function \code{getListName} can be used to get the names of the stored list.

Function \code{getListLength} returns the length of the stored list. 
}
\examples{
## save to file
list1 <- list("A" = c(1,2), "B" = "abc", list(1, 2, 3))
saveList(list1, "example.llo")
## append to file
list2 <- list("D" = "d", "E" = "e")
saveList(list2, "example.llo", append = TRUE)
## read from file
readList("example.llo")
readList("example.llo", 1)
readList("example.llo",c(1, 3, 5))
readList("example.llo",c("A", "D"))
## remove from list
removeFromList("example.llo",c(2, 3))
removeFromList("example.llo",c("A", "D"))
## get names in list 
getListName("example.llo")
## get length of list
getListLength("example.llo")
}
\author{
Yuchun Zhang
}
\seealso{
\code{\link{readRDS}}
\code{\link{serialize}}
}

