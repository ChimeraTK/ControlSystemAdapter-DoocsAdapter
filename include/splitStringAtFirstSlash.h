#ifndef _DOOCS_ADAPTER_SPLIT_STRING_AT_FIRST_SLASH_
#define _DOOCS_ADAPTER_SPLIT_STRING_AT_FIRST_SLASH_

namespace ChimeraTK{

  inline std::pair< std::string, std::string > splitStringAtFirstSlash(std::string input){
    auto slashPosition = input.find_first_of("/");
    if(slashPosition == 0) {    // ignore leading slash
      input = input.substr(1);
      slashPosition = input.find_first_of("/");
    }
    if (slashPosition == std::string::npos){
      return std::make_pair( std::string(), input);
    }
    return std::make_pair( input.substr( 0, slashPosition) ,
			   input.substr( slashPosition+1 ) );
  }

}

#endif // _DOOCS_ADAPTER_SPLIT_STRING_AT_FIRST_SLASH_
