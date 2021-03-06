% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/operatorOverload.R
\name{[<-.largeList}
\alias{[<-.largeList}
\title{Overload of operator []<-.}
\usage{
\method{[}{largeList}(x, index = NULL) <- value
}
\arguments{
\item{x}{A largeList object created by \code{\link{getList}}.}

\item{index}{\code{NULL} or a numeric, logical, character vector.}

\item{value}{\code{NULL}, a vector or a list.}
}
\description{
Overload of operator []<-.
}
\details{
It behaviours the same as a normal list object. If \code{index} is not provided, elements in
value will be appended to the list file. If \code{value} is \code{NULL}, elements with given indices 
will be removed.
}
\examples{
largelist_object <- getList("example.llo", truncate = TRUE)
largelist_object[] <- list("A" = 1, "B" = 2)  ## append list to the list file
largelist_object[] <- list("C" = 3, "D" = 4)  ## append list to the list file
largelist_object[1] <- NULL ## remove first element
largelist_object["B"] <- NULL ## remove element with name "B"
largelist_object[c("C","D")] <- c(5, 6) ## change value
largelist_object[2] <- 5 ## change value
largelist_object[c(4, 5)] <- list(6, 7) ## append 6, 7 to 4th, 5th position and NULL to 3rd position
}
\seealso{
\code{\link{largeList}}
}
