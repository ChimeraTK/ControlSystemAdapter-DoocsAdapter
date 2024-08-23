// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE DoocsImageTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h
#include "DoocsImage.h"

#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>
#include <ChimeraTK/Device.h>
#include <ChimeraTK/TypeChangingDecorator.h>

#include <boost/test/unit_test.hpp>

#include <limits>
#include <sstream>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

BOOST_AUTO_TEST_SUITE(DoocsImageTestSuite)

struct DeviceFixture {
  const size_t arraySize = 400000;
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers;
  boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> deviceVariable;
  boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> controlSystemVariable;

  DeviceFixture() {
    pvManagers = createPVManager();
    boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
    boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

    deviceVariable = devManager->createProcessArray<uint8_t>(
        SynchronizationDirection::deviceToControlSystem, "fromDeviceVariable", arraySize);
    controlSystemVariable = csManager->getProcessArray<uint8_t>("fromDeviceVariable");
  }
};

BOOST_FIXTURE_TEST_CASE(testMappedDoocsImage, DeviceFixture) {
  // this test shows MappedImage usage

  ChimeraTK::OneDRegisterAccessor acc(deviceVariable);
  MappedImage A0(acc);
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
  std::vector<uint8_t> expectedData = {8, 0, 7, 0, 6, 0, 5, 0, 4, 0, 3, 0, 2, 0, 1, 0};

  MappedDoocsImg A(acc, MappedDoocsImg::InitData::No);
  IMH headerOut;
  unsigned char* imgData = A.asDoocsImg(&headerOut);
  BOOST_CHECK(headerOut.aoi_height == (int)h);
  BOOST_CHECK(headerOut.aoi_width == (int)w);
  for(unsigned i = 0; i < w * h * 2; i++) {
    BOOST_CHECK(imgData[i] == expectedData[i]);
  }
}

// generate a test image in given byteArray, which should already have required size
void generateImage(ChimeraTK::OneDRegisterAccessor<uint8_t>& acc) {
  ChimeraTK::MappedImage im(acc, MappedDoocsImg::InitData::Yes);
  unsigned w = 20, h = 10;
  im.setShape(w, h, ImgFormat::Gray16);
  ChimeraTK::ImgHeader* imh = im.header();
  imh->effBitsPerPixel = 14;
  imh->frame = 3;

  auto imv = im.interpretedView<uint16_t>();
  for(unsigned x = 0; x < w; x++) {
    unsigned x1 = (x + 5) % w;
    for(unsigned y = 0; y < h; y++) {
      imv(x, y) = x1 * x1 - y * y;
    }
  }
}

BOOST_FIXTURE_TEST_CASE(fromDeviceTest, DeviceFixture) {
  const size_t arraySizeCheck = 400;

  DoocsUpdater updater;

  // EqFct = NULL, not required for our purpose
  DoocsImage doocsImage(nullptr, "someName", controlSystemVariable, updater);

  // generate an image and send it via the device side
  ChimeraTK::OneDRegisterAccessor acc(deviceVariable);
  generateImage(acc);
  deviceVariable->write();

  // everything should still be 0 on the CS side
  std::vector<uint8_t>& csVector = controlSystemVariable->accessChannel(0);
  for(size_t i = 0; i < arraySizeCheck; ++i) {
    BOOST_CHECK(csVector[i] == 0);
  }
  // image data not yet set
  BOOST_CHECK(doocsImage.value() == nullptr);

  updater.update();

  // The actual vector buffer has changed. We have to get the new reference.
  csVector = controlSystemVariable->accessChannel(0);
  auto deviceVector = deviceVariable->accessChannel(0);
  for(size_t i = 0; i < arraySizeCheck; ++i) {
    BOOST_CHECK_EQUAL(csVector[i], deviceVector[i]);
    // note, this works only if the image format is not changed by mapping to DOOCS
    BOOST_CHECK_EQUAL(doocsImage.value()[i], deviceVector[i + sizeof(ImgHeader)]);
  }
}

BOOST_AUTO_TEST_SUITE_END()
