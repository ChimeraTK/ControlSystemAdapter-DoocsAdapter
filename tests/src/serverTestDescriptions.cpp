// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestDescriptions

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

extern const char* object_name;
#include "DoocsAdapter.h"
#include "ExtendedTestApplication.h"
#include "serverBasedTestTools.h"

#include <ChimeraTK/MappedImage.h>

#include <doocs-server-test-helper/ThreadedDoocsServer.h>

#include <D_history.h>

#include <string>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

// Dedicated application adding process variables that carry unit/description from the application. This is used
// only by this test so that the shared ExtendedTestApplication (used by many other tests) is not modified.
struct DescriptionTestApplication : public ExtendedTestApplication {
  using ExtendedTestApplication::ExtendedTestApplication;

  void initialise() override {
    // Process variables carrying unit/description from the application, used to test the default
    // "description from app" behavior (case a). The unit/description must be given at creation time so that
    // both the control-system and the device side of the process variable carry the metadata.
    appDescScalar = _processVariableManager->createProcessArray<float>(
        ChimeraTK::SynchronizationDirection::deviceToControlSystem, "FLOAT/APP_DESC_SCALAR", 1, "appUnit", "appDesc");
    appDescScalar->write();

    appDescArray =
        _processVariableManager->createProcessArray<float>(ChimeraTK::SynchronizationDirection::deviceToControlSystem,
            "FLOAT/APP_DESC_ARRAY", 4, "arrayAppUnit", "arrayAppDesc");
    appDescArray->write();

    // start/increment scalars carrying units, used to test the x-axis unit taken over from the
    // <startSource>/<incrementSource> when description_from_app is true.
    appStartScalar = _processVariableManager->createProcessArray<float>(
        ChimeraTK::SynchronizationDirection::deviceToControlSystem, "FLOAT/APP_START", 1, "startUnit", "startDesc");
    appStartScalar->write();
    appIncrementScalar = _processVariableManager->createProcessArray<float>(
        ChimeraTK::SynchronizationDirection::deviceToControlSystem, "FLOAT/APP_INCREMENT", 1, "incUnit", "incDesc");
    appIncrementScalar->write();
    appDescImage = _processVariableManager->createProcessArray<uint8_t>(
        ChimeraTK::SynchronizationDirection::deviceToControlSystem, "UINT8/IMAGE", 1000, "", "app image desc");
    // create some valid image - we don't care about content
    OneDRegisterAccessor<uint8_t> imAcc(appDescImage);
    MappedImage mi{imAcc};
    mi.setShape(1, 1, ImgFormat::Gray8);
    appDescImage->write();

    // MUST come last
    ExtendedTestApplication::initialise();
  }

  ChimeraTK::ProcessArray<float>::SharedPtr appDescScalar;
  ChimeraTK::ProcessArray<float>::SharedPtr appDescArray;
  ChimeraTK::ProcessArray<float>::SharedPtr appStartScalar;
  ChimeraTK::ProcessArray<float>::SharedPtr appIncrementScalar;
  ChimeraTK::ProcessArray<uint8_t>::SharedPtr appDescImage;
};

// custom fixture using the dedicated DescriptionTestApplication
struct GlobalFixture {
  GlobalFixture() {
    rpcNo = server.rpcNo();
    bpn = server.bpn();
    ChimeraTK::DoocsAdapter::waitUntilInitialised();
    DescriptionTestApplication::initialiseManualLoopControl();
  }
  ~GlobalFixture() { DescriptionTestApplication::releaseManualLoopControl(); }
  static DescriptionTestApplication referenceTestApplication;
  static std::string rpcNo;
  static std::string bpn;
  ThreadedDoocsServer server{boost::unit_test::framework::master_test_suite().p_name.value + ".conf",
      boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv,
      doocsAdapter.createServer()};
};
DescriptionTestApplication GlobalFixture::referenceTestApplication{BOOST_STRINGIZE(BOOST_TEST_MODULE)};
std::string GlobalFixture::rpcNo;
std::string GlobalFixture::bpn;
BOOST_GLOBAL_FIXTURE(GlobalFixture);

/**********************************************************************************************************************/

// Helper: check the .DESC/.EGU sub-properties of the D_hist of a history-enabled scalar/D_iiii/D_ifff.
// If expectedDesc/expectedUnit are non-empty the corresponding sub-property must carry that value and be read-only.
// If they are empty neither is forced; they must not be read-only.
static void checkHistMetadata(std::string const& propertyAddress, D_hist* hist, const std::string& expectedDesc,
    const std::string& expectedUnit) {
  auto* location = getLocationFromPropertyAddress(propertyAddress);
  location->lock();

  auto* desc = dynamic_cast<D_string*>(hist->get_p_prop(3));
  auto* egu = dynamic_cast<D_plotinfo*>(hist->get_p_prop(2));
  if(!expectedDesc.empty()) {
    BOOST_REQUIRE(desc != nullptr);
    BOOST_CHECK_EQUAL(desc->get_value(), expectedDesc);
    BOOST_CHECK(desc->get_access() == ACCESS_RO);
  }
  else {
    // description was not forced: the sub-properties must remain writeable (control-system controlled).
    BOOST_REQUIRE(desc != nullptr);
    BOOST_CHECK_MESSAGE(desc->get_access() == ACCESS_RW, "unset .DESC must stay writeable");
  }
  if(!expectedUnit.empty()) {
    BOOST_REQUIRE(egu != nullptr);
    BOOST_CHECK_EQUAL(egu->get_unit(), expectedUnit);
    BOOST_CHECK(egu->get_access() == ACCESS_RO);
  }
  else {
    // unit was not forced: the sub-property must remain writeable (control-system controlled).
    BOOST_REQUIRE(egu != nullptr);
    BOOST_CHECK_MESSAGE(egu->get_access() == ACCESS_RW, "unset .EGU must stay writeable");
  }

  location->unlock();
}

/**********************************************************************************************************************/

// Helper: check manual .DESC/.EGU sub-properties (arrays, strings, non-history scalars).
static void checkManualMetadata(
    std::string const& propertyAddress, const std::string& expectedDesc, const std::string& expectedUnit) {
  auto* desc = getDoocsProperty<D_string>(propertyAddress + ".DESC");
  auto* location = getLocationFromPropertyAddress(propertyAddress);
  location->lock();

  if(!expectedDesc.empty()) {
    BOOST_CHECK_EQUAL(desc->get_value(), expectedDesc);
    BOOST_CHECK(desc->get_access() == ACCESS_RO);
  }
  if(!expectedUnit.empty()) {
    auto* egu = getDoocsProperty<D_plotinfo>(propertyAddress + ".EGU");
    BOOST_REQUIRE(egu != nullptr);
    BOOST_CHECK_EQUAL(egu->get_unit(), expectedUnit);
    BOOST_CHECK(egu->get_access() == ACCESS_RO);
  }

  location->unlock();
}

/**********************************************************************************************************************/

// Helper: read the unit string of a D_spectrum/D_xy axis via plot_x_value/plot_y_value, like the existing
// testXyMetadata does.
static std::string readPlotUnit(D_fct* property, bool xAxis) {
  int i1;
  float f1, f2;
  time_t tmp;
  char buf[255];
  if(auto* spectrum = dynamic_cast<D_spectrum*>(property)) {
    if(xAxis) {
      spectrum->plot_x_value(&i1, &f1, &f2, &tmp, buf, sizeof(buf));
    }
    else {
      spectrum->plot_y_value(&i1, &f1, &f2, &tmp, buf, sizeof(buf));
    }
  }
  else if(auto* xy = dynamic_cast<D_xy*>(property)) {
    if(xAxis) {
      xy->plot_x_value(&i1, &f1, &f2, &tmp, buf, sizeof(buf));
    }
    else {
      xy->plot_y_value(&i1, &f1, &f2, &tmp, buf, sizeof(buf));
    }
  }
  return std::string(buf);
}

/**********************************************************************************************************************/

// description/unit taken over from the process variable for a history scalar
BOOST_AUTO_TEST_CASE(testFromAppScalar) {
  std::cout << "testFromAppScalar" << std::endl;
  const std::string propertyAddress{"//DESC/APP_SCALAR"};
  auto* scalar = getDoocsProperty<D_float>(propertyAddress);
  BOOST_REQUIRE(scalar->get_histPointer() != nullptr);
  checkHistMetadata(propertyAddress, scalar->get_histPointer(), "appDesc", "appUnit");
}

/**********************************************************************************************************************/

// description/unit taken over from the process variable for an array (manual fallback)
BOOST_AUTO_TEST_CASE(testFromAppArray) {
  std::cout << "testFromAppArray" << std::endl;
  checkManualMetadata("//DESC/APP_ARRAY", "arrayAppDesc", "arrayAppUnit");
}

/**********************************************************************************************************************/

// explicit XML <description>/<unit> on a history scalar
BOOST_AUTO_TEST_CASE(testXmlScalar) {
  std::cout << "testXmlScalar" << std::endl;
  const std::string propertyAddress{"//DESC/XML_SCALAR"};
  auto* scalar = getDoocsProperty<D_double>(propertyAddress);
  BOOST_REQUIRE(scalar->get_histPointer() != nullptr);
  checkHistMetadata(propertyAddress, scalar->get_histPointer(), "scalar xml desc", "scalarXmlUnit");
}

/**********************************************************************************************************************/

// explicit XML <description>/<unit> on an array (manual fallback)
BOOST_AUTO_TEST_CASE(testXmlArray) {
  std::cout << "testXmlArray" << std::endl;
  checkManualMetadata("//DESC/XML_ARRAY", "array xml desc", "arrayXmlUnit");
}

/**********************************************************************************************************************/

// explicit XML <description>/<unit> on a string scalar (manual fallback)
BOOST_AUTO_TEST_CASE(testXmlString) {
  std::cout << "testXmlString" << std::endl;
  checkManualMetadata("//DESC/XML_STRING", "string xml desc", "stringXmlUnit");
}

/**********************************************************************************************************************/

// description_from_app=false -> not forced, sub-properties remain writeable (control-system controlled)
BOOST_AUTO_TEST_CASE(testNotFromApp) {
  std::cout << "testNotFromApp" << std::endl;
  const std::string propertyAddress{"//DESC/CTRL_SCALAR"};
  auto* scalar = getDoocsProperty<D_int>(propertyAddress);
  BOOST_REQUIRE(scalar->get_histPointer() != nullptr);
  // neither description nor unit from app nor from XML -> nothing forced, sub-properties stay writeable
  checkHistMetadata(propertyAddress, scalar->get_histPointer(), "", "");
}

/**********************************************************************************************************************/

// explicit XML <description>/<unit> on D_iiii (never from app)
BOOST_AUTO_TEST_CASE(testXmlIiii) {
  std::cout << "testXmlIiii" << std::endl;
  const std::string propertyAddress{"//DESC/XML_IIII"};
  auto* iiii = getDoocsProperty<D_iiii>(propertyAddress);
  BOOST_REQUIRE(iiii->get_histPointer() != nullptr);
  checkHistMetadata(propertyAddress, iiii->get_histPointer(), "iiii xml desc", "iiiiXmlUnit");
}

/**********************************************************************************************************************/

// D_iiii with no XML description/unit and no ignore flag -> nothing forced (.DESC/.EGU stay writeable)
BOOST_AUTO_TEST_CASE(testCtrlIiii) {
  std::cout << "testCtrlIiii" << std::endl;
  const std::string propertyAddress{"//DESC/CTRL_IIII"};
  auto* iiii = getDoocsProperty<D_iiii>(propertyAddress);
  BOOST_REQUIRE(iiii->get_histPointer() != nullptr);
  checkHistMetadata(propertyAddress, iiii->get_histPointer(), "", "");
}

/**********************************************************************************************************************/

// D_ifff with no XML description/unit and no ignore flag -> nothing forced (.DESC/.EGU stay writeable)
BOOST_AUTO_TEST_CASE(testCtrlIfff) {
  std::cout << "testCtrlIfff" << std::endl;
  const std::string propertyAddress{"//DESC/CTRL_IFFF"};
  auto* ifff = getDoocsProperty<D_ifff>(propertyAddress);
  BOOST_REQUIRE(ifff->get_histPointer() != nullptr);
  checkHistMetadata(propertyAddress, ifff->get_histPointer(), "", "");
}

/**********************************************************************************************************************/

// explicit XML <description>/<unit axis=x|y> on a D_spectrum
BOOST_AUTO_TEST_CASE(testXmlSpectrum) {
  std::cout << "testXmlSpectrum" << std::endl;
  const std::string propertyAddress{"//DESCSPECTRUM/XML_SPECTRUM"};
  auto* spectrum = getDoocsProperty<D_spectrum>(propertyAddress);
  auto* location = getLocationFromPropertyAddress(propertyAddress);

  location->lock();
  BOOST_CHECK_EQUAL(spectrum->description(), "spectrum xml desc");
  BOOST_CHECK_EQUAL(readPlotUnit(spectrum, true), "spectrumXUnit");
  BOOST_CHECK_EQUAL(readPlotUnit(spectrum, false), "spectrumYUnit");
  location->unlock();
}

/**********************************************************************************************************************/

// spectrum x-unit taken from <incrementSource> (wins over <startSource>) when description_from_app=true
BOOST_AUTO_TEST_CASE(testFromAppSpectrumXUnit) {
  std::cout << "testFromAppSpectrumXUnit" << std::endl;
  const std::string propertyAddress{"//DESCSPECTRUM/APP_SPECTRUM"};
  auto* spectrum = getDoocsProperty<D_spectrum>(propertyAddress);
  auto* location = getLocationFromPropertyAddress(propertyAddress);

  location->lock();
  BOOST_CHECK_EQUAL(readPlotUnit(spectrum, true), "incUnit");
  location->unlock();
}

/**********************************************************************************************************************/

// explicit XML <description>/<unit axis=x|y> on a D_xy
BOOST_AUTO_TEST_CASE(testXmlXy) {
  std::cout << "testXmlXy" << std::endl;
  const std::string propertyAddress{"//DESCXY/XML_XY"};
  auto* xy = getDoocsProperty<D_xy>(propertyAddress);
  auto* location = getLocationFromPropertyAddress(propertyAddress);

  location->lock();
  BOOST_CHECK_EQUAL(xy->description(), "xy xml desc");
  BOOST_CHECK_EQUAL(readPlotUnit(xy, true), "xyXUnit");
  BOOST_CHECK_EQUAL(readPlotUnit(xy, false), "xyYUnit");
  location->unlock();
}

/**********************************************************************************************************************/

// explicit XML <description> on a D_imagec
BOOST_AUTO_TEST_CASE(testXmlImage) {
  std::cout << "testXmlImage" << std::endl;
  const std::string propertyAddress{"//DESCIM/IM"};

  checkManualMetadata(propertyAddress, "image xml desc", "");
}
