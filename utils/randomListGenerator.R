randomNumeric <- function(length = NULL, attr = TRUE){
  if (is.null(length)) {
    length = sample(500:1000, 1)
  }
  res <- runif(length, -1000, +1000)
  if (attr) {res <- addAttribute(res)}
  return(res)
}

randomInteger <- function(length = NULL, attr = TRUE){
  if (is.null(length)) {
    length = sample(500:1000, 1)
  }
  res <- sample(-1000:1000, length, replace = T)
  if (attr) {res <- addAttribute(res)}
  return(res)
}

randomLogical <- function(length = NULL, attr = TRUE){
  if (is.null(length)) {
    length = sample(500:1000, 1)
  }
  res <- sample(c(TRUE,FALSE),length, replace = T)
  if (attr) { res <- addAttribute(res)}
  return(res)
}

randomCharacter <- function(length = NULL, attr = TRUE)
{
  if (is.null(length)) {
    length = sample(500:1000, 1)
  }
  randomString <- c(1:length)                  # initialize vector
  for (i in 1:length)
  {
    str_length = sample(1:20, 1)
    randomString[i] <- paste(sample(c(0:9, letters, LETTERS), str_length, replace = TRUE), collapse = "")
  } 
  if (attr) { randomString <- addAttribute(randomString)}
  return(randomString)
}

randomRaw <- function(length = NULL, attr = TRUE){
  if (is.null(length)) {
    length = sample(500:1000, 1)
  }
  res <- as.raw(sample(0:(16*16 - 1), length, replace = T))
  if (attr) { res <- addAttribute(res) }
  return(res)
}

randomComplex <- function(length = NULL, attr = TRUE) {
  if (is.null(length)) {
    length = sample(500:1000, 1)
  }
  z <- complex(real = runif(length, -1000, 1000), 
               imaginary =  runif(length, -1000, 1000))
  if (attr) { z <- addAttribute(z) }
  return(z)
}

randomBasic <- function(length = NULL, attr = TRUE) {
  if (is.null(length)) {
    length = sample(500:1000, 1)
  }
  type = sample(c("numeric","integer","raw","complex","character","logic"),1)
  switch(type,
         "numeric" = {res <- randomNumeric(length, attr)},
         "integer" = {res <- randomInteger(length, attr)},
         "raw" = {res <- randomRaw(length, attr)},
         "complex" = {res <- randomComplex(length, attr)},
         "character" = {res <- randomCharacter(length, attr)},
         "logic" = {res <- randomLogical(length, attr)}
  )
  return(res)
}

randomBasicWithoutRaw <- function(length = NULL, attr = TRUE) {
  if (is.null(length)) {
    length = sample(500:1000, 1)
  }
  type = sample(c("numeric","integer","complex","character","logic"),1)
  switch(type,
         "numeric" = {res <- randomNumeric(length, attr)},
         "integer" = {res <- randomInteger(length, attr)},
         "complex" = {res <- randomComplex(length, attr)},
         "character" = {res <- randomCharacter(length, attr)},
         "logic" = {res <- randomLogical(length, attr)}
  )
  return(res)
}

randomFactor <- function(length = NULL, attr = TRUE){
  if (is.null(NULL)) {res <- randomBasicWithoutRaw()} else {
    length = sample(500:1000, 1)
    res <- randomBasicWithoutRaw(length)}
  res <- as.factor(res)
  if (attr) {res <- addAttribute(res) }
  return(res)
}

randomMatrix <- function(m = NULL, n = NULL, attr = TRUE) {
  if (is.null(m)) {
    m = sample(10:20, 1)
  }
  if (is.null(n)) {
    n = sample(10:20, 1)
  }
  res <- matrix(randomBasic(m * n), nrow = m)
  if (attr) {res <- addAttribute(res) }
  return(res)
}

randomData.frame <- function(m = NULL, n = NULL, attr = TRUE) {
  if (is.null(m)) {
    m = sample(10:20, 1)
  }
  if (is.null(n)) {
    n = sample(10:20, 1)
  }
  res <- data.frame(randomBasic(m))
  for (i in 1:n) {
    res <- cbind(res, randomBasic(m))
  }
  if (attr) { res <- addAttribute(res) }
  return(res)
}

randomArray <- function() {
  dim <- sample(5:10,sample(1:4 ,1))
  res <- array(data = randomBasic(prod(dim)), dim = dim)
  res <- addAttribute(res)
  return(res)
}

randomList <- function(length = NULL, depth = 2, attr = TRUE){
  if (is.null(length)) {
    length = sample(500:1000, 1)
  }
  l <- list()
  for (i in 1:length) {
    if (depth > 0) {
      type = sample(c("numeric","integer","raw","complex","character","logic", "matrix",
                      "factor", "array", "data.frame", "null", rep("list",10)),1)
    } else {
      type = sample(c("numeric","integer","raw","complex","character","logic", "matrix",
                      "factor", "array", "data.frame", "null"),1)
    }

    switch(type,
           "numeric" = {res <- randomNumeric()},
           "integer" = {res <- randomInteger()},
           "raw" = {res <- randomRaw()},
           "complex" = {res <- randomComplex()},
           "character" = {res <- randomCharacter()},
           "logic" = {res <- randomLogical()},
           "factor" = {res <- randomFactor()},
           "matrix" = {res <- randomMatrix()},
           "array" = {res <- randomArray()},
           "data.frame" = {res <- randomData.frame()},
           "null" = {res <- NULL},
           "list" = {res <- randomList(length = 5, depth = depth - 1)}
    )
    l[i] <- list(res)
  }
  names(l) <- randomCharacter(length)
  if (attr) { l <- addAttribute(l) }
  return(l)
}

addAttribute <- function(x) {
  times <- sample(c(rep(0,5),1:5), 1)
  if (times > 0) {
    for (i in 1:times) {
      name <- randomCharacter(length = 1, attr = FALSE)
      attribute <- randomBasic(length = 10, attr = FALSE)
      attr(x, name) <- attribute
    }
  }
  return(x)
}