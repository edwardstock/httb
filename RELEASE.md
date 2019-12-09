# Release notes

## 1.0.0
 - Stable release
 - Refactored code and used c++17 standard (partially)
 - Fixed blocking execution, now using beast executor timer
 - Added methods to remove and clear headers and query parameters
 - Fixed consistency while adding headers: now container checks for existent header, and overwrite value for it, otherwise insert with lowercase name
 - Hidden non-api headers and functions
 - Added boost::optional return type for find_* methods

## 0.4.2
 - Using clang-format for code-style
 - Updated toolbox dependency

## 0.4.1
 - Fixed building parameters for post body
 - Fixed undefined reference for `body_string`

## 0.4.0
 - Added progress callback
 - Added batch request
 - Fixed files downloading
 - Refactoring