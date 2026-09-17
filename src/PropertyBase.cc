// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "PropertyBase.h"

#include "DoocsAdapter.h"
#include "DoocsUpdater.h"

namespace ChimeraTK {

  /********************************************************************************************************************/

  PropertyBase::PropertyBase(
      std::string doocsPropertyName, DoocsUpdater& updater, DataConsistencyGroup::MatchingMode matchingMode)
  : _consistencyGroup(matchingMode), _doocsPropertyName(std::move(doocsPropertyName)), _doocsUpdater(updater) {}

  /********************************************************************************************************************/

  void PropertyBase::setDescription(const std::string& desc) {
    _description = desc;
    _hasDescription = true;
  }

  /********************************************************************************************************************/

  void PropertyBase::setAxis(const Axis& axis, char name) {
    assert(name == 'x' || name == 'y');
    _hasUnits = true;
    _axes['y']; // ensure axis y exists
    _axes[name] = axis;
  }

  /********************************************************************************************************************/

  void PropertyBase::resolveDescriptionAndUnits(const PropertyDescription& propertyDescription,
      const std::string& autoDescription, const std::string& autoXUnit, const std::string& autoYUnit) {
    // Resolve description/unit and decide which manual .DESC/.EGU sub-properties are wanted.
    //
    // "Wanted" rule: a description/unit is NOT wanted when its source is static (explicit XML <description>/<unit>,
    // or the process-variable metadata when description_from_app is true) AND the resolved text is empty. It IS
    // wanted when the text is non-empty, or when the source is dynamic (description_from_app==false, i.e. a value
    // may be written later by the control system / .conf file), in which case the sub-property must still exist to
    // receive it. Only *static & empty* is skipped, matching the factory's "do not create empty units" optimisation.
    //
    // Note: _hasDescription/_hasUnits (leading to a value being forced read-only in applyDescriptionUnits()) are
    // only set when there is real content. For dynamic & currently empty, the sub-property is created but stays
    // writeable.

    // --- description ---
    bool wantDesc = false;
    if(propertyDescription.description.has_value()) {
      // static XML description; explicit XML wins over "from app"
      if(!propertyDescription.description.value().empty()) {
        setDescription(propertyDescription.description.value());
        wantDesc = true;
      }
      // static & empty -> not wanted
    }
    else if(propertyDescription.descriptionFromApp) {
      // static process-variable metadata is the default source; empty -> not wanted
      if(!autoDescription.empty()) {
        setDescription(autoDescription);
        wantDesc = true;
      }
    }
    else {
      // dynamic control-system source: the .DESC sub-property must exist for later CS/.conf values, but nothing is
      // forced (stays writeable).
      wantDesc = true;
    }

    // --- units (per axis) ---
    // The XML axis label/geometry always wins; otherwise, if description_from_app is not ignored (i.e. not dynamic),
    // fill in the auto units. wantEgu tracks whether the .EGU sub-property (y-axis engineering unit) is wanted.
    const bool dynamic = !propertyDescription.descriptionFromApp;
    bool yAxisSet = false;
    bool xAxisSet = false;
    bool wantEgu = false;
    for(auto const& [name, axis] : propertyDescription.axes) {
      setAxis(axis, name);
      xAxisSet = xAxisSet || name == 'x';
      yAxisSet = yAxisSet || name == 'y';
      if(name == 'y' && !axis.label.empty()) {
        wantEgu = true;
      }
    }
    if(!dynamic) {
      // as optimisation, to reduce number of DOOCS properties, do not create units if they are static&empty
      if(!yAxisSet && !autoYUnit.empty()) {
        setAxis(Axis{autoYUnit}, 'y');
        wantEgu = true;
      }
      if(!xAxisSet && !autoXUnit.empty()) {
        setAxis(Axis{autoXUnit}, 'x');
      }
    }
    else {
      // dynamic control-system source: the .EGU sub-property must exist for later CS/.conf values
      wantEgu = true;
    }
    wantEgu = wantEgu && _wantEgu; // e.g. DoocsImage creates no .EGU

    // --- create the manual .DESC/.EGU sub-properties now (before prop_reg_all()) ---
    // If the property natively provides .DESC/.EGU (D_hist, D_spectrum/D_xy API), nothing must be created manually.
    if((wantDesc || wantEgu) && !hasNativeDescriptionUnits()) {
      ensureManualDescEgu(wantDesc, wantEgu);
    }
  }

  /********************************************************************************************************************/

  void PropertyBase::ensureManualDescEgu(bool wantDesc, bool wantEgu) {
    auto* loc = getEqFct();
    auto base = getDfct()->basename();
    // Only create if they do not already exist in the location, to avoid a duplicate registration doocs::Error.
    if(wantDesc && !_manualDesc && loc->find_property(base + ".DESC") == nullptr) {
      _manualDesc = std::make_unique<D_string>(base + ".DESC", loc);
    }
    if(wantEgu && !_manualEgu && loc->find_property(base + ".EGU") == nullptr) {
      _manualEgu = std::make_unique<D_plotinfo>(base + ".EGU", loc);
    }
  }

  /********************************************************************************************************************/

  void PropertyBase::applyDescriptionUnits(D_hist* hist) {
    if(hist) {
      if(_hasDescription) {
        hist->set_description(_description);
        if(auto* desc = dynamic_cast<D_string*>(hist->get_p_prop(3))) {
          desc->set_ro_access(); // .DESC readonly
        }
      }
      if(_hasUnits) {
        const Axis& a = _axes.at('y');
        hist->set_plot_value(a.logarithmic, a.start, a.stop, doocs::Timestamp::now().to_time_t(), a.label.c_str());
        if(auto* egu = dynamic_cast<D_plotinfo*>(hist->get_p_prop(2))) {
          // egu->set_value(a.label);
          egu->set_ro_access(); // .EGU readonly
        }
      }
    }
    else {
      // The manual .DESC/.EGU sub-properties were created in resolveDescriptionAndUnits() BEFORE prop_reg_all()
      // ran (so no reallocation of prop_list_ happens here during auto_init()). Here we only apply the stored
      // description/unit values and mark the sub-properties read-only.
      if(_hasDescription && _manualDesc) {
        _manualDesc->set_value(_description);
        _manualDesc->set_ro_access(); // .DESC readonly
      }
      if(_hasUnits && _manualEgu) {
        // this function is copied from D_history; currently no similar API for D_plotinfo defined.
        auto setPlotValue = [this](int i1, float f1, float f2, time_t tm, const char* com) {
          const char* p;
          USTR val;

          val.i1_data = i1;
          val.f1_data = f1;
          val.f2_data = f2;
          val.tm = tm;

          p = com ? com : "";

          val.str_data.str_data_val = const_cast<char*>(p);
          val.str_data.str_data_len = static_cast<unsigned int>(strlen(p));

          _manualEgu->set_value(&val);
          return 1;
        };
        auto a = _axes.at('y'); // fallback does not happen for x-axis: .XEGU present both for D_spectrum,D_xy
        setPlotValue(a.logarithmic, a.start, a.stop, doocs::Timestamp::now().to_time_t(), a.label.c_str());
        _manualEgu->set_ro_access(); // .EGU readonly
      }
    }
  }

  /********************************************************************************************************************/

  void PropertyBase::registerVariable(TransferElementAbstractor& var, bool update) {
    if(var.isReadable()) {
      auto id = var.getId();
      _consistencyGroup.add(var);
      if(update) {
        _doocsUpdater.addVariable(var, getEqFct(), [this, id] { return updateDoocsBuffer(id); });
      }
      else {
        _doocsUpdater.addVariable(var, getEqFct());
      }
    }
  }

  /********************************************************************************************************************/

  bool PropertyBase::updateConsistency(const TransferElementID& updatedId) {
    // FIXME: A first  implementation is checking the data consistency here. Later this should be
    // before calling this function because calling this function through a function pointer is
    // comparatively expensive.

    // Do not check if update is coming from another DOOCS property mapped to the same variable (ID invalid), since
    // the check would never pass. Such variables cannot use exact data matching anyway, since the update is triggered
    // from the DOOCS write to the other property.
    // Also do not check, if data matching turned off
    if(!updatedId.isValid() || _consistencyGroup.getMatchingMode() == DataConsistencyGroup::MatchingMode::none) {
      _doocsSuccessfullyUpdated = true;
      return true;
    }
    assert(_outputVarForVersionNum);
    TransferElementID compareTo = _outputVarForVersionNum->getId();

    if(!_consistencyGroup.update(updatedId)) {
      // data is not consistent (yet). Don't update the Doocs buffer.
      // check if this will now throw away data and generate a warning
      if(updatedId == compareTo) {
        if(!_doocsSuccessfullyUpdated) {
          ++_nDataLossWarnings;
          if(DoocsAdapter::checkPrintDataLossWarning(_nDataLossWarnings)) {
            const char* reason = (bool)(_macroPulseNumberSource) ?
                " due to failed data matching between value and macro pulse number" :
                " due to failed data matching between values";

            std::cout << "WARNING: Data loss in property " << getEqFct()->name() << "/" << _doocsPropertyName << reason
                      << " (repeated " << _nDataLossWarnings << " times)." << std::endl;
          }
        }
        // Tracking that data is modified is only done for the "compareTo" variable.
        // If we want to detect data loss on all variables in the consistency group, we would have to track this for
        // each variable separately.
        _doocsSuccessfullyUpdated = false;
      }
      return false;
    }
    _doocsSuccessfullyUpdated = true;
    return true;
  }

  /********************************************************************************************************************/

  doocs::Timestamp PropertyBase::getTimestamp() {
    assert(_outputVarForVersionNum);
    doocs::Timestamp timestamp(_outputVarForVersionNum->getVersionNumber().getTime());
    return timestamp;
  }

  /********************************************************************************************************************/

  doocs::Timestamp PropertyBase::correctDoocsTimestamp() {
    doocs::Timestamp timestamp = getTimestamp();
    auto* d_fct = getDfct();
    // Make sure we never send out two absolute identical time stamps. If we would do so, the "watchdog" which
    // corrects inconsistencies in ZeroMQ subscriptions between sender and subscriber cannot detect the inconsistency.
    // This check also enforces that the timestamp is always increasing (never going backwards in time)
    if(timestamp <= d_fct->get_timestamp()) {
      timestamp = d_fct->get_timestamp() + std::chrono::microseconds(1);
    }

    // update global time stamp of DOOCS, but only if our time stamp is newer
    if(get_global_timestamp() < timestamp) {
      set_global_timestamp(timestamp);
    }
    d_fct->set_timestamp(timestamp);
    return timestamp;
  }

  /********************************************************************************************************************/

  void PropertyBase::sendAsync(doocs::Timestamp timestamp) {
    auto* d_fct = getDfct();
    // send data via ZeroMQ if enabled and if DOOCS initialisation is complete
    if(_publishLegacyZMQ && ChimeraTK::DoocsAdapter::isInitialised) {
      dmsg_info info{};
      auto sinceEpoch = timestamp.get_seconds_and_microseconds_since_epoch();
      info.sec = sinceEpoch.seconds;
      info.usec = sinceEpoch.microseconds;
      if(_macroPulseNumberSource.isInitialised()) {
        info.ident = _macroPulseNumberSource;
      }
      else {
        info.ident = 0;
      }
      dmsg_error(&info, d_fct->d_error());
      auto ret = d_fct->send(&info);
      if(ret) {
        std::cout << "ZeroMQ sending failed!!!" << std::endl;
      }
    }
    // _publishAsync is off for scalars, as D_value<T> allready calls publish() in set_value()
    if(_publishAsync && ChimeraTK::DoocsAdapter::isInitialised) {
      // provide all data updates via DOOCS-over-ZeroMQ
      d_fct->publish();
    }
  }

  /********************************************************************************************************************/

  void PropertyBase::updateOthers(bool handleLocking) {
    // Release the location lock before calling callbacks to avoid deadlocks when callbacks lock other locations
    if(handleLocking) {
      getEqFct()->unlock();
    }
    // Invoke all registered callbacks, passing this property as the caller so callbacks can identify the source
    for(const auto& callback : callbacksOnChange()) {
      callback(handleLocking, shared_from_this());
    }
    if(handleLocking) {
      getEqFct()->lock();
    }
  }

  /********************************************************************************************************************/

  void PropertyBase::setMacroPulseNumberSource(const std::string& sourcePath) {
    if(!sourcePath.empty()) {
      auto mpnSource = _doocsUpdater.getMappedProcessVariable<int64_t>(sourcePath);
      if(mpnSource->getNumberOfSamples() != 1) {
        throw ChimeraTK::logic_error("The property '" + mpnSource->getName() +
            "' is used as a macro pulse number source, but it has an array length of " +
            std::to_string(mpnSource->getNumberOfSamples()) + ". Length must be exactly 1");
      }
      if(!mpnSource->isReadable()) {
        throw ChimeraTK::logic_error("The property '" + mpnSource->getName() +
            "' is used as a macro pulse number source, but it is not readable.");
      }
      setMacroPulseNumberSource(mpnSource);
    }
  }

  /********************************************************************************************************************/

  void PropertyBase::setMacroPulseNumberSource(
      const boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>>& macroPulseNumberSource) {
    _macroPulseNumberSource.replace(macroPulseNumberSource);

    // we send out updates only when configured data_matching not equal 'none'
    registerVariable(
        _macroPulseNumberSource, _consistencyGroup.getMatchingMode() != DataConsistencyGroup::MatchingMode::none);
  }

  /********************************************************************************************************************/

  void PropertyBase::setIsWriteableSource(const std::string& sourcePath) {
    if(!sourcePath.empty()) {
      auto isWriteableSource = _doocsUpdater.getMappedProcessVariable<ChimeraTK::Boolean>(sourcePath);
      if(isWriteableSource->getNumberOfSamples() != 1) {
        throw ChimeraTK::logic_error(std::format(
            "The property '{}' is used as a is-writeable source, but is not a scalar.", isWriteableSource->getName()));
      }

      if(isWriteableSource->isReadable()) {
        // Readable PV (device-to-CS direction): register with DoocsUpdater to receive updates via the update loop
        _isWriteableSource.replace(isWriteableSource);
        _doocsUpdater.addVariable(_isWriteableSource, getEqFct(), [this] {
          if(bool(_isWriteableSource)) {
            this->getDfct()->set_rw_access();
          }
          else {
            this->getDfct()->set_ro_access();
          }
        });
      }
      else {
        // Write-only PV (CS-to-device direction): cannot use DoocsUpdater since that only handles readable PVs.
        // Instead, register a callback in writeableVariablesWithMultipleProperties so it gets called when another
        // property writes to the same PV (via updateOthers). The callback reads the current value from the caller's
        // DOOCS property using the generic EqData interface.
        assert(!doocsAdapter.writeableVariablesWithMultiplePropertiesIsFinal);
        auto weakSelf = weak_from_this();
        doocsAdapter.writeableVariablesWithMultipleProperties[sourcePath].emplace_back(
            [weakSelf](bool handleLocking, const boost::shared_ptr<PropertyBase>& caller) {
              auto self = weakSelf.lock();
              if(!self) {
                return;
              }
              EqData input, result;
              caller->getDfct()->get(nullptr, &input, &result, caller->getEqFct());
              bool value = result.get_bool();
              if(handleLocking) {
                self->getEqFct()->lock();
              }
              if(value) {
                self->getDfct()->set_rw_access();
              }
              else {
                self->getDfct()->set_ro_access();
              }
              if(handleLocking) {
                self->getEqFct()->unlock();
              }
            });
      }
    }
  }

  /********************************************************************************************************************/

  void PropertyBase::subscribeToSharedPV(const std::string& pvName) {
    // Register a callback that updates this property's DOOCS buffer when another property writes to the shared PV.
    // Uses weak_ptr to avoid preventing destruction of this property.
    auto weakSelf = weak_from_this();
    doocsAdapter.writeableVariablesWithMultipleProperties[pvName].emplace_back(
        [weakSelf](bool handleLocking, const boost::shared_ptr<PropertyBase>& caller) {
          auto self = weakSelf.lock();
          if(!self) {
            return;
          }
          assert(caller != self);

          if(handleLocking) {
            self->getEqFct()->lock();
          }
          self->updateDoocsBuffer({});
          if(handleLocking) {
            self->getEqFct()->unlock();
          }
        });
  }

  /********************************************************************************************************************/

  PVChangeListeners& PropertyBase::callbacksOnChange() {
    if(_callbacksCacheIsFinal) {
      return _callbacksOnChange_cache;
    }

    // Collect all callbacks from PV groups this property is subscribed to. This includes both data-property
    // callbacks (from subscribeToSharedPV) and isWriteableSource callbacks (from setIsWriteableSource).
    _callbacksOnChange_cache.clear();
    _hasOtherDataProperties = false;
    for(auto& [pvName, listeners] : doocsAdapter.writeableVariablesWithMultipleProperties) {
      if(!_sharedPVSubscriptions.contains(pvName)) {
        continue;
      }
      _callbacksOnChange_cache.insert(_callbacksOnChange_cache.end(), listeners.begin(), listeners.end());
      // More than one listener means there are other data properties besides just an isWriteableSource callback
      if(listeners.size() > 1) {
        _hasOtherDataProperties = true;
      }
    }
    if(doocsAdapter.writeableVariablesWithMultiplePropertiesIsFinal) {
      _callbacksCacheIsFinal = true;
    }
    return _callbacksOnChange_cache;
  }

  /********************************************************************************************************************/

} // namespace ChimeraTK
