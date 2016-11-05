# largeList: Serialization Interface for Large List Objects

## Description
Functions to write or append a R list to a file, read, modify or remove elements from it without restoring the whole list.

## How it works
R objects will be serialized with an uncompressed/ compressed (zlib, default level) non-ascii little-endian format, which is similar to saveRDS. Two ordered tables are created at the end of data for quick lookups, one for indices and one for element names. Notice that, all the names will be truncated to 16 characters. 

Given indices or names of elements, positions will be directly extracted or extracted via binary search within the name-position table. Then required elements are located and unserialized. Therefore it will not restore the whole list into memory. 

## Limits of current version
In the current version, only basic data types are supported, including NULL, integer, 
numeric, character, complex, raw, logic, factor, list, matrix, array and data.frame. 
Types like function, data.table are not supported. 

The supported maximum size of R objects is 2^31 -1, the supported maximum file 
size is 2^63 -1 bytes. 

# Functions
* **saveList:**   Save or append elements to a list file.
* **readList:**  Get elements from a list file.
* **removeFromList:** Remove elements from a list file.
* **modifyInList:** Modify elements in a list file.
* **modifyNameInList:** Modify names of elements in a list file.
* **getListName:** Get number of elements in a list file.
* **getListLength:** Get names of elements in a list file.

With overloads of operators, list objects stored in files can be manipulated as simply as the normal list objects in R.

* **getList:** Bind a R object with a list file.
* **\[.largeList:** Get elements.
* **\[\[.largeList:** Get element.
* **$.largeList:** Get element, same as **\[\[.largeList**, no partial matching.
* **\[<-.largeList:** If index provided, it modifies, appends or removes the elements with given indices, otherwise it appends value to list. 
* **\[\[<-.largeList:** If index provided, it modifies, appends or removes the element with given index, otherwise it saves value to list. 
* **$<-.largeList:** Same as **\[\[<-.largeList**, no partial matching.
* **length.largeList:** Get length of list stored in file.
* **length<-.largeList:** Set length of list stored in file.
* **names.largeList:** Get names of elements stored in file.
* **names<-.largeList:** Set names of elements stored in file.
