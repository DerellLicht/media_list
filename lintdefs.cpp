//lint -esym(40, errno)

//lint -e534  Ignoring return value of function
//lint -e713  Loss of precision (arg. no. 2) (unsigned int to long)
//lint -e737  Loss of sign in promotion from int to unsigned long
//lint -e754  local struct member not referenced

//  These two warnings mean that lint wants extern "C" added to file,
//  because it is a .h file, not a .hpp file.
//  This is not an issue that the compilers actually care about.
//lint -e1065  Symbol not declared as "C" conflicts with self
//lint -e1066  Symbol declared as "C" conflicts with self
//lint -e1709  typedef declared as "C" conflicts with self

//  deal with ambiguities in MediaInfoDll.cpp
//lint -esym(526, get_mi_info)  //  symbol not defined

//  unicode warnings
//lint -e559  Size of argument inconsistent with format

//lint -esym(526, _wstat, _wopen)
//lint -esym(628, _wstat, _wopen)
//lint -esym(746, _wstat, _wopen)
//lint -esym(1055, _wstat, _wopen)
