#ifndef _DOOCS_ADAPTER_SPLIT_STRING_AT_FIRST_SLASH_
#define _DOOCS_ADAPTER_SPLIT_STRING_AT_FIRST_SLASH_

namespace ChimeraTK {

  /** Split the given process variable name into location and property name
   *  @todo TODO rename this function! */
  inline std::pair<std::string, std::string> splitStringAtFirstSlash(std::string input) {
    // find first slash
    auto slashPosition = input.find_first_of("/");
    if(slashPosition == 0) { // ignore leading slash
      input = input.substr(1);
      slashPosition = input.find_first_of("/");
    }
    // no slash found: return empty location name
    if(slashPosition == std::string::npos) {
      return std::make_pair(std::string(), input);
    }
    // split at first slash into location name and property name
    auto locationName = input.substr(0, slashPosition);
    auto propertyName = input.substr(slashPosition + 1);
    // replace any remaining slashes in property name with dots
    while((slashPosition = propertyName.find_first_of("/")) != std::string::npos) {
      propertyName[slashPosition] = '.';
    }
    return std::make_pair(locationName, propertyName);
  }

} // namespace ChimeraTK

#endif // _DOOCS_ADAPTER_SPLIT_STRING_AT_FIRST_SLASH_
