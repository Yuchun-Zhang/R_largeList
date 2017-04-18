# largeList: Serialization Interface for Large List Objects

## Description
Functions to write or append a R list to a file, read, modify or remove elements from it without restoring the whole list.

## Installation of CRAN Version

[![CRAN_Status_Badge][cran_version_badges]][cran_link]
[![Build Status][build_status_badges]][build_status_link]
[![Downloads from the RStudio CRAN mirror][cran_download_badges]][cran_link]


You can install the released version from [CRAN][cran_link].

```R
install.packages("largeList")
```

## Development

[![Build Status][build_status_badges_dev]][build_status_link]


The latest version of package is under development at [GitHub][github_dev] in branch
'v0.3.1'. You may install it with **devtools** by

```R
devtools::install_github("Yuchun-Zhang/R_largeList", ref = "v0.3.2", subdir = "largeList")
```

## Get Started

[Package vignettes][largeList_vignettes]
provides a quick demonstration for the basic usage of main functions.


# Change Log

### v0.3.2
* [f] Remove predefined platform specified directories in testing part.

### v0.3.1
* [u] Improve algorithm for new position calculations in function removeFromList and modifyInList.
* [+] Add progress output to console if estimated processing time > 5s in function saveList, readList, removeFromList and modifyInList.
* [+] Add user interrupt detection.
* [f] Add include cstring library to largeList.h for function std::memcpy in old environments.
* [+] Implemented memory slots to avoid frequent malloc and free.
* [f] Fix problem of slow performance of mkChar.
* [u] Rewrite error handling parts.

### v0.3.0
* [u] Refactor code into OOP.
* [f] Fix memory leak.
* [+] Add compression options.
* [+] Add random list generator for testing.
* [f] Fix library problems for windows. 
* [u] Change serialization head, not compatible with v0.2.0

### v0.2.0
* [+] Add functions modifyInList, modifyNameInList.
* [+] Add operator overloadings, which provide similar syntax as manipulating normal list objects in R.
* [f] Fix problem with empty names.
* [f] Fix fclose bug in modifyInList.
* [u] Change serialization format, not compatible with v0.1.0

### v0.1.0
* [+] First version, including saveList, readList, removeFromList, getListLength and getListName functions.


## License

The R package **largeList** is free software and comes with ABSOLUTELY NO WARRANTY.
You can redistribute it and/or modify it under the terms of the GNU General Public License,
see the [GNU General Public License][gnu_license] for details.


[cran_version_badges]: http://www.r-pkg.org/badges/version/largeList
[cran_link]: https://CRAN.R-project.org/package=largeList
[build_status_badges]: https://travis-ci.org/Yuchun-Zhang/R_largeList.svg?branch=master
[build_status_badges_dev]: https://travis-ci.org/Yuchun-Zhang/R_largeList.svg?branch=v0.3.2
[build_status_link]: https://travis-ci.org/Yuchun-Zhang/R_largeList
[cran_download_badges]: http://cranlogs.r-pkg.org/badges/largeList
[github_dev]: https://github.com/Yuchun-Zhang/R_largeList/tree/v0.3.2
[largeList_vignettes]: https://cran.r-project.org/web/packages/largeList/vignettes/intro_largeList.html
[gnu_license]: http://www.gnu.org/licenses/