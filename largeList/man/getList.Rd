% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/operatorOverload.R
\name{getList}
\alias{getList}
\title{Create a R object and bind with file.}
\usage{
getList(file, compress = TRUE, verbose = FALSE, truncate = FALSE)
}
\arguments{
\item{file}{Name of file}

\item{compress}{\code{TRUE/FALSE} Use compression for elements or not.}

\item{verbose}{\code{TRUE/FALSE} Print extra info}

\item{truncate}{\code{TRUE/FALSE} Truncate the file or not.}
}
\value{
A R object of class "largeList"
}
\description{
Create a R object and bind with file.
}
\details{
Create a R object of class "largeList" and bind it with a file.
If the file exists, the R object will be bound to the file.
If the file does not exist or \code{truncate == TRUE}, an empty list will be written into 
the given file and then bind it with R object. Later the R object can be used as a normal
list and all the manipulation will be done within the file binding to it.
}
\examples{
largelist_object <- getList("example.llo", verbose = TRUE, truncate = TRUE)
}
\seealso{
\code{\link{largeList}}
}
