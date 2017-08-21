#ifndef CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H
#define CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H

#include <string>
#include <map>
#include <set>
#include <memory>
#include <iostream>

namespace xmlpp{
  class Node;
}

namespace ChimeraTK{

  class VariableMapper{
  public:
    static VariableMapper & getInstance();
    void prepareOutput(std::string xmlFile, std::set< std::string > inputVariables);

    struct SpectrumDescription{
      float start;
      float increment;
      SpectrumDescription(float start_=0.0, float increment_=1.0)
      : start(start_), increment(increment_){}
      bool operator==(SpectrumDescription const & other) const{
        return start==other.start && increment==other.increment;
      }
    };
    
    // PropertyAttributes are used in the property description itself, and
    // as default values (global and in the locations)
    struct PropertyAttributes{
      bool hasHistory;
      bool isWriteable;
      bool hasSpectrum;
      SpectrumDescription spectrum;
      PropertyAttributes(bool hasHistory_ = true, bool isWriteable_=true, bool hasSpectrum_=true)
      : hasHistory(hasHistory_), isWriteable(isWriteable_), hasSpectrum(hasSpectrum_){}
      bool operator==(PropertyAttributes const & other) const{
        return hasHistory == other.hasHistory
          && isWriteable == other.isWriteable
          && hasSpectrum == other.hasSpectrum
          && spectrum == other.spectrum;
      }
    };

    // extends the PropertyAttributes by a name
    // FIXME: should sort by name to put it into a set?
    struct PropertyDescription:
      public PropertyAttributes{
      std::string location;
      std::string name;
      PropertyDescription(std::string location_="", std::string name_="", bool hasHistory_ = true, bool isWriteable_=true)
        : PropertyAttributes(hasHistory_, isWriteable_), location(location_), name(name_){}
      bool operator==(PropertyDescription const & other) const{
        return location==other.location && name==other.name && static_cast< const PropertyAttributes *>(this)->operator==(other);
      }
    };

    std::map< std::string, PropertyDescription > getPropertiesInLocation(std::string location) const;
    std::map< std::string, PropertyDescription > const & getAllProperties() const;

    VariableMapper(VariableMapper &)=delete;
    void operator=(VariableMapper const &)=delete;

    void directImport(std::set< std::string > inputVariables);
    
    // empty the created mapping 
    void clear();

    /// printing the map is useful for debugging
    void print(std::ostream & os = std::cout) const;
    
    /// Note on the input: we intentionally make a because we modify it inside.
    static bool evaluateBool(std::string txt);

  protected:
    VariableMapper()=default;

    std::set< std::string > _inputVariables;
  
    void processLocationNode(xmlpp::Node const * locationNode);
    void processPropertyNode(xmlpp::Node const * propertyNode, std::string locationName);
    void processImportNode(xmlpp::Node const * importNode, std::string importLocationName=std::string());

    void import(std::string importSource, std::string importLocationName, std::string directory="");
   
    std::map<std::string, PropertyAttributes> _locationDefaults;
    PropertyAttributes _globalDefaults;

    // PropertyDescriptions, sorted by input, i.e. the ChimeraTK PV name
    std::map<std::string, PropertyDescription> _inputSortedDescriptions;

    /// An internal helper function to abbreviate the syntax
    bool nodeIsWhitespace(const xmlpp::Node* node);
  };

} // namespace ChimeraTK

#endif // CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H
