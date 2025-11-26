%module BoardWriter
%include "std_string.i"
%include "std_vector.i"
%include "stdint.i"
namespace std {
    %template(UintVector) vector<uint32_t>;
}

 %{
#include "BoardWriter.hh"
 %}


%include "BoardWriter.hh"



