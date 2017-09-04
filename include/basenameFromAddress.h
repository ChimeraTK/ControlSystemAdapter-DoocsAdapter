#ifndef _DOOCS_ADAPTER_BASENAME_FROM_ADDRESS_
#define _DOOCS_ADAPTER_BASENAME_FROM_ADDRESS_

namespace ChimeraTK {

  /** Find the last slash and return the subsring behind it.
   */
  inline std::string basenameFromAddress(std::string const & doocsAddress) {
    // find first slash
    auto slashPosition = doocsAddress.rfind("/");
    // no slash found: return the whole string
    if(slashPosition == std::string::npos){
      return doocsAddress;
    }
    return doocsAddress.substr( slashPosition+1 );
  }

}// namespace ChimeraTK

#endif // _DOOCS_ADAPTER_BASENAME_FROM_ADDRESS_
