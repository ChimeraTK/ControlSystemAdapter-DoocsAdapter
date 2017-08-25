#ifndef CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H
#define CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H

#include <string>
#include <map>
#include <set>
#include <memory>
#include <iostream>
#include <boost/any.hpp>
#include <mtca4u/RegisterPath.h>

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

    template<class T>
    static bool castAndCompare(boost::any const &t1, boost::any const &t2){
      try{
        auto casted1 = boost::any_cast<T>(t1);
        auto casted2 = boost::any_cast<T>(t2);
        // both casts have to succeed
        return casted1==casted2;
      }catch(const boost::bad_any_cast &){
        return false;
      }
    }
    
    static bool compareDoocsTypeDescriptions(boost::any const &description1, boost::any const &description2);

    // PropertyAttributes are used in the property description itself, and
    // as default values (global and in the locations)
    struct PropertyAttributes{
      bool hasHistory;
      bool isWriteable;
      PropertyAttributes(bool hasHistory_ = true, bool isWriteable_=true)
      : hasHistory(hasHistory_), isWriteable(isWriteable_){}
      bool operator==(PropertyAttributes const & other) const{
        return (hasHistory == other.hasHistory
                && isWriteable == other.isWriteable);
      }
    };

    // Common for all properties, the base class to be stored.
    // FIXME: should sort by name to put it into a set?
    struct PropertyDescription{
      std::string location;
      std::string name;
      PropertyDescription(std::string location_="", std::string name_="")
      : location(location_), name(name_){}
      virtual bool operator==(PropertyDescription const & other) const{
        return location==other.location && name==other.name;
      }
      virtual const std::type_info& type() const{
        return typeid(PropertyDescription);
      }
    };

    // Combines property attributes and the base description
    // FIXME: should sort by name to put it into a set?
    struct AutoPropertyDescription:
      public PropertyDescription, public PropertyAttributes{
      mtca4u::RegisterPath source;
     AutoPropertyDescription(mtca4u::RegisterPath const & source_="", std::string location_="", std::string name_="", bool hasHistory_ = true, bool isWriteable_=true)
      : PropertyDescription(location_, name_), PropertyAttributes(hasHistory_, isWriteable_), source(source_){}
      virtual bool operator==(PropertyDescription const & other) const override{
        if (other.type() == typeid(AutoPropertyDescription)){
          auto casted_other = static_cast<AutoPropertyDescription const &>(other);
          return source==casted_other.source && location==other.location && name==other.name && static_cast< const PropertyAttributes *>(this)->operator==(casted_other);
        }else{
          return false;
        }
      }
      virtual const std::type_info& type() const override{
        return typeid(AutoPropertyDescription);
      }
    };

    // This is the per location information which are used as default for the properties in this
    // location
    struct LocationInfo: public PropertyAttributes{
      bool useHasHistoryDefault;
      bool useIsWriteableDefault;
      LocationInfo(bool useHasHistoryDefault_=false,  bool useIsWriteableDefault_=false)
        : useHasHistoryDefault(useHasHistoryDefault_), useIsWriteableDefault(useIsWriteableDefault_){
      }
    };
    
    std::map< std::string, std::shared_ptr<PropertyDescription> > getPropertiesInLocation(std::string location) const;
    std::map< std::string, std::shared_ptr<PropertyDescription> > const & getAllProperties() const;

    VariableMapper(VariableMapper &)=delete;
    void operator=(VariableMapper const &)=delete;

    void directImport(std::set< std::string > inputVariables);
    
    // empty the created mapping 
    void clear();

    /// printing the map is useful for debugging
    void print(std::ostream & os = std::cout) const;
    
    /// Function to get a bool out of the texts true/True/TRUE/false/False/FALSE/1/0.
    /// Note on the input: we intentionally make a copy because we modify it inside.
    static bool evaluateBool(std::string txt);

    /// Loop through the sub nodes and take the string from the first non-empty
    /// which can be casted to a string-node
    static std::string getContentString(xmlpp::Node const *node);
     
  protected:
    VariableMapper()=default;

    std::set< std::string > _inputVariables;
  
    void processLocationNode(xmlpp::Node const * locationNode);
    void processPropertyNode(xmlpp::Node const * propertyNode, std::string locationName);
    void processImportNode(xmlpp::Node const * importNode, std::string importLocationName=std::string());

    void import(std::string importSource, std::string importLocationName, std::string directory="");
    bool getHasHistoryDefault(std::string const & locationName);
    bool getIsWriteableDefault(std::string const & locationName);
   
    std::map<std::string, LocationInfo> _locationDefaults;
    PropertyAttributes _globalDefaults;

    // PropertyDescriptions, sorted by input, i.e. the ChimeraTK PV name
    std::map<std::string, std::shared_ptr<PropertyDescription> > _inputSortedDescriptions;

    /// An internal helper function to abbreviate the syntax
    bool nodeIsWhitespace(const xmlpp::Node* node);
  };

} // namespace ChimeraTK

#endif // CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H
