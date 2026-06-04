#ifndef _DCUTILS_H
#define _DCUTILS_H
#include <iostream>
/** 
\class DCUtils
  \author  L.Mirabito 
  \date March 2010
  \version 1.0

   \brief Old utilities static class

   
*/
class DCUtils {
  public:
/// @brief Convert a string to a vector of strings  according to a separator
/// @param string_list Original string
/// @param v reference to the output vector
/// @param sep string separator (default is , )
static  void String2Vector(std::string string_list,std::vector<std::string> &v,std::string sep=",")
{
 int idx1=0,idx2;
 unsigned int pos;
  do 
    {
      pos = string_list.find(sep,idx1);
      if (pos!=0xFFFFFFFF)
        {
          idx2= pos-1;
        }
      else
        idx2= string_list.length();
      //std::cout <<idx1<<" adding "<<pos<<" "<< string_list.substr(idx1,idx2-idx1+1) <<std::endl;
      v.push_back(string_list.substr(idx1,idx2-idx1+1));
      idx1 =pos+1;
    } while (pos!=0xFFFFFFFF);
}
}; 
#endif
