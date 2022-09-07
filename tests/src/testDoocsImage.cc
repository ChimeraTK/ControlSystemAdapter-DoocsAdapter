// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE DoocsImageTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h
#include "DoocsImage.h"

#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>
#include <ChimeraTK/ControlSystemAdapter/TypeChangingDecorator.h>

#include <boost/test/unit_test.hpp>

#include <limits>
#include <sstream>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

BOOST_AUTO_TEST_SUITE(DoocsImageTestSuite)

BOOST_AUTO_TEST_CASE(testStructMapping) {
  // this test shows an example how to map user-defined opaque structs onto a byte array
  struct AStruct : public OpaqueStructHeader {
    int a = 0;
    float x = 0, y = 0;
    AStruct() : OpaqueStructHeader(typeid(AStruct)) {}
  };
  unsigned len = 100;
  std::vector<unsigned char> buf(len);
  MappedStruct<AStruct> ms(buf.data(), len);
  auto& h = ms.header();
  h.x = 4.;
  BOOST_CHECK(h.totalLength == sizeof(AStruct));

  MappedStruct<AStruct> ms1(buf.data(), len, MappedStruct<AStruct>::InitData::No);
  BOOST_CHECK(ms1.header().x == 4.);
}

BOOST_AUTO_TEST_CASE(testMappedImage) {
  // this test shows MappedImage usage
  MappedImage A0;
  unsigned w = 4, h = 2;
  A0.setShape(w, h, ImgFormat::Gray16);
  auto Av = A0.interpretedView<uint16_t>();
  Av(0, 0) = 8;
  Av(1, 0) = 7;
  Av(2, 0) = 6;
  Av(3, 0) = 5;
  Av(0, 1) = 4;
  Av(1, 1) = 3;
  Av(2, 1) = 2;
  Av(3, 1) = 1;
  float val = Av(2, 0);
  BOOST_CHECK(val == 6);
  std::vector<uint8_t> expectedData = {8, 0, 7, 0, 6, 0, 5, 0, 4, 0, 3, 0, 2, 0, 1, 0};

  MappedDoocsImg A(A0.data(), A0.dataLen(), MappedDoocsImg::InitData::No);
  IMH headerOut;
  unsigned char* imgData = A.asDoocsImg(&headerOut);
  BOOST_CHECK(headerOut.aoi_height == (int)h);
  BOOST_CHECK(headerOut.aoi_width == (int)w);
  for(unsigned i = 0; i < w * h * 2; i++) {
    BOOST_CHECK(imgData[i] == expectedData[i]);
  }
}

// generate a test image in given byteArray, which should already have required size
void generateImage(std::vector<uint8_t>& byteArray) {
  ChimeraTK::MappedImage im(byteArray.data(), byteArray.size(), MappedDoocsImg::InitData::Yes);
  unsigned w = 20, h = 10;
  im.setShape(w, h, ImgFormat::Gray16);
  ChimeraTK::ImgHeader& imh = im.header();
  imh.ebitpp = 14;
  imh.frame = 3;

  auto imv = im.interpretedView<uint16_t>();
  for(unsigned x = 0; x < w; x++) {
    unsigned x1 = (x + 5) % w;
    for(unsigned y = 0; y < h; y++) {
      imv(x, y) = x1 * x1 - y * y;
    }
  }
}

BOOST_AUTO_TEST_CASE(fromDeviceTest) {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  static const size_t arraySize = 400000;
  static const size_t arraySizeCheck = 400;
  typename boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> deviceVariable =
      devManager->createProcessArray<uint8_t>(
          SynchronizationDirection::deviceToControlSystem, "fromDeviceVariable", arraySize);
  typename boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> controlSystemVariable =
      csManager->getProcessArray<uint8_t>("fromDeviceVariable");

  DoocsUpdater updater;

  // EqFct = NULL, not required for our purpose
  DoocsImage doocsImage(nullptr, "someName", controlSystemVariable, updater);

  // generate an image and send it via the device side
  std::vector<uint8_t>& deviceVector = deviceVariable->accessChannel(0);
  generateImage(deviceVector);
  deviceVariable->write();

  // everything should still be 0 on the CS side
  std::vector<uint8_t>& csVector = controlSystemVariable->accessChannel(0);
  for(size_t i = 0; i < arraySizeCheck; ++i) {
    BOOST_CHECK(csVector[i] == 0);
  }
  // image data not yet set
  BOOST_CHECK(doocsImage.value() == 0);

  updater.update();

  // The actual vector buffer has changed. We have to get the new reference.
  csVector = controlSystemVariable->accessChannel(0);
  for(size_t i = 0; i < arraySizeCheck; ++i) {
    BOOST_CHECK_EQUAL(csVector[i], deviceVector[i]);
    // note, this works only if the image format is not changed by mapping to DOOCS
    BOOST_CHECK_EQUAL(doocsImage.value()[i], deviceVector[i + sizeof(ImgHeader)]);
  }
}

BOOST_AUTO_TEST_SUITE_END()
