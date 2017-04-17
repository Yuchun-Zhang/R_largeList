## ---- echo = FALSE, message = FALSE--------------------------------------
library(largeList)
knitr::opts_chunk$set(
  comment = "#",
    error = FALSE,
     tidy = FALSE,
    cache = FALSE,
 collapse = TRUE)

## ------------------------------------------------------------------------
# save list_1 to a new file called example.llo using compression.
list_1 <- list("A" = c(1,2),
               "B" = "abc", 
               list(1, 2))
saveList(object = list_1, file = "example.llo", append = FALSE, compress = TRUE)

# append list_2 to the existing file example.llo, compress option will be extracted from the file.
list_2 <-  list("C" = data.frame(col_1 = 1:2, col_2 = 3:4),
                "D" = matrix(0, nrow = 2, ncol = 2))
saveList(object = list_2, file = "example.llo", append = TRUE)

## ------------------------------------------------------------------------
# all elements
list_read <- readList(file = "example.llo")

# by numeric indices
list_read <- readList(file = "example.llo", index = c(1, 3))

# by names
list_read <- readList(file = "example.llo", index = c("A", "B"))

# by logical indices
list_read <- readList(file = "example.llo", index = c(T, F, T, F, T))

## ------------------------------------------------------------------------
# copy the file
file.copy(from = "example.llo", to = "example_remove.llo")

# by numeric indices
removeFromList(file = "example_remove.llo", index = c(2))

# by names
removeFromList(file = "example_remove.llo", index = c("A", "D"))

# by logical indices
removeFromList(file = "example_remove.llo", index = c(T, F))

# remove file
file.remove("example_remove.llo")

## ------------------------------------------------------------------------
# copy the file
file.copy(from = "example.llo", to = "example_modify.llo")

# by numeric indices
modifyInList(file = "example_modify.llo", index = c(1, 2), object = list("AA", "BB"))

# by names
modifyInList(file = "example_modify.llo", index = c("C","D"), object = list("C","D"))

# by logical indices
modifyInList(file = "example_modify.llo", index = c(T, F), object = list(1, 2))

# remove file
file.remove("example_modify.llo")

## ------------------------------------------------------------------------
# copy the file
file.copy(from = "example.llo", to = "example_modify_name.llo")

# by numeric indices
modifyNameInList(file = "example_modify_name.llo", index = c(1, 2), name = c("new_name_A", "new_name_B"))

# by logical indices
modifyNameInList(file = "example_modify_name.llo", index = c(T, F), name = c("new_name_C", "new_name_D"))

# remove file
file.remove("example_modify_name.llo")

## ------------------------------------------------------------------------
getListName("example.llo")

## ------------------------------------------------------------------------
getListLength("example.llo")

# remove file
file.remove("example.llo")

## ------------------------------------------------------------------------
# by setting truncate == TRUE, file will be truncated if exists.
largelist_object <- getList("example.llo", verbose = TRUE, truncate = TRUE)

# by setting truncate == FALSE, it will bind to existing file.
largelist_object <- getList("example.llo", verbose = TRUE, truncate = FALSE)

## ------------------------------------------------------------------------
# save list
largelist_object[[]] <- list("A" = 1, "B" = 2)

# append list
largelist_object[] <- list("C" = 3, "D" = 4)

## ------------------------------------------------------------------------
# For print just use largelist_object, for assignment, use largelist_object[]
largelist_object
object_copy <- largelist_object[]

# by numeric indices
largelist_object[c(1,2)]
largelist_object[[1]]

# by names
largelist_object[c("A", "E")]
largelist_object[["A"]]

# by logical indices
largelist_object[c(T, F)]

## ------------------------------------------------------------------------
# by numeric indices
largelist_object[1] <- NULL

# by names
largelist_object["B"] <- NULL

# by logical indices
largelist_object[c(T,F)] <- NULL

## ------------------------------------------------------------------------
largelist_object[[]] <- list("A" = 1, "B" = 2, "C" = 3, "D" = 4)

# by numeric indices
largelist_object[c(1, 5)] <- list(1, "E" = 5)

# by names
largelist_object[c("C","F")] <- c(5, 7)

# by logical indices
largelist_object[c(T, F)] <- c(8)

print(largelist_object)

## ------------------------------------------------------------------------
largelist_object[[]] <- list("A" = 1, "B" = 2)
# get names 
names(largelist_object)

# modify names
names(largelist_object)[c(1, 2)] <- c("AA", "BB")
names(largelist_object)[c(F, T)] <- c("DD")

print(largelist_object)

## ------------------------------------------------------------------------
largelist_object[[]] <- list("A" = 1, "B" = 2)
# maximal number to print can be changed by setting option largeList.max.print.
print(largelist_object)

length(largelist_object)

head(largelist_object)

tail(largelist_object)

# remove object and file
rm(largelist_object)
file.remove("example.llo")

