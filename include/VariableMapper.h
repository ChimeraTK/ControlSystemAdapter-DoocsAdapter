#ifndef CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H
#define CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H

#include <string>
#include <map>
#include <set>

namespace ChimeraTK{

  class VariableMapper{
  public:
    static VariableMapper & getInstance();
    void prepareOutput(std::string xmlFile, std::set< std::string > inputVariables);

    // PropertyAttributes are used in the property description itself, and
    // as default values (global and in the locations)
    struct PropertyAttributes{
      bool hasHistory;
      bool isWriteable;
    };

    // extends the PropertyAttributes by a name
    // FIXME: should sort by name to put it into a set?
    struct PropertyDescription:
      public PropertyAttributes{
      std::string location;
      std::string name;
    };

    std::map< std::string, PropertyDescription > getPropertiesInLocation(std::string location);
    std::map< std::string, PropertyDescription > const & getAllProperties();

    VariableMapper(VariableMapper &)=delete;
    void operator=(VariableMapper const &)=delete;
    
  private:
    VariableMapper()=default;

    std::map<std::string, PropertyAttributes> locationDefaults;
    PropertyAttributes globalDefaults;

    // PropertyDescriptions, sorted by input, i.e. the ChimeraTK PV name
    std::map<std::string, PropertyDescription> inputSortedDescriptions;
  };

} // namespace ChimeraTK

#endif // CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H
