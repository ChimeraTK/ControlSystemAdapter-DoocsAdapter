// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

struct ExtendedTestApplication : public ReferenceTestApplication {
  using ReferenceTestApplication::ReferenceTestApplication;

  void initialise() override {
    in["IIII"] = _processVariableManager->createProcessArray<int>(
        ChimeraTK::SynchronizationDirection::controlSystemToDevice, "IIII/TO_DEVICE", 4);
    out["IIII"] = _processVariableManager->createProcessArray<int>(
        ChimeraTK::SynchronizationDirection::deviceToControlSystem, "IIII/FROM_DEVICE", 4);
    out["IIII"]->write();

    // MUST come last
    ReferenceTestApplication::initialise();
  }
  void mainBody() override {
    ReferenceTestApplication::mainBody();

    for(auto& [key, inAcc] : in) {
      auto outAcc = out.at(key);
      if(inAcc->readLatest()) {
        outAcc->accessData(0) = inAcc->accessData(0);
        outAcc->accessData(1) = inAcc->accessData(1);
        outAcc->accessData(2) = inAcc->accessData(2);
        outAcc->accessData(3) = inAcc->accessData(3);
        outAcc->setDataValidity(dataValidity);
        if(outAcc->write(versionNumber.value_or(ChimeraTK::VersionNumber()))) {
          std::cout << "WARNING: Data loss in referenceTestApplication for Process Variables: " << std::endl;
          std::cout << "\t" << inAcc->getName() << std::endl;
        }
      }
    }
  }

  std::map<std::string, ChimeraTK::ProcessArray<int>::SharedPtr> in;
  std::map<std::string, ChimeraTK::ProcessArray<int>::SharedPtr> out;
};
