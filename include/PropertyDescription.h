#ifndef CHIMERATK_DOOCS_ADAPTER_PROPERTY_DESCRIPTION_H
#define CHIMERATK_DOOCS_ADAPTER_PROPERTY_DESCRIPTION_H

#include <string>
//#include <map>
//#include <set>
//#include <memory>
//#include <iostream>
//#include <mtca4u/RegisterPath.h>

///@todo FIXME: Separate these out into individual files

namespace ChimeraTK{

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

    struct SpectrumDescription{
      float start;
      float increment;
      SpectrumDescription(float start_=0.0, float increment_=1.0)
      : start(start_), increment(increment_){}
      bool operator==(SpectrumDescription const & other) const{
        return start==other.start && increment==other.increment;
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
    
} // namespace ChimeraTK

#endif // CHIMERATK_DOOCS_ADAPTER_PROPERTY_DESCRIPTION_H
