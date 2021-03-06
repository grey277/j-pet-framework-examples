/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file TimeCalibLoader.cpp
 */

#include "TimeCalibLoader.h"
#include "TimeCalibTools.h"
#include "JPetGeomMapping/JPetGeomMapping.h"
#include <JPetParamManager/JPetParamManager.h>

TimeCalibLoader::TimeCalibLoader(const char* name, const char* description):
  JPetTask(name, description)
{
  /**/
}

TimeCalibLoader::~TimeCalibLoader() {}

void TimeCalibLoader::init(const JPetTaskInterface::Options& opts)
{
  auto calibFile =  std::string("timeCalib.txt");
  if (opts.count(fConfigFileParamKey)) {
    calibFile = opts.at(fConfigFileParamKey);
  }
  assert(fParamManager);
  JPetGeomMapping mapper(fParamManager->getParamBank());
  auto tombMap = mapper.getTOMBMapping();
  fTimeCalibration = TimeCalibTools::loadTimeCalibration(calibFile, tombMap);
  if (fTimeCalibration.empty()) {
    ERROR("Time calibration seems to be empty");
  }
}

void TimeCalibLoader::exec()
{
  if (auto oldTimeWindow = dynamic_cast<const JPetTimeWindow* const>(getEvent())) {
    JPetTimeWindow correctedWindow;
    auto newSigChs = oldTimeWindow->getSigChVect();
    for (auto & sigCh : newSigChs) {
      /// Calibration time is ns so we should change it to ps, cause all the time is in ps.
      sigCh.setValue(sigCh.getValue() + 1000. * TimeCalibTools::getTimeCalibCorrection(fTimeCalibration, sigCh.getTOMBChannel().getChannel()));
      correctedWindow.addCh(sigCh);
    }
    correctedWindow.setIndex(oldTimeWindow->getIndex());
    saveTimeWindow(correctedWindow);
  }
}

void TimeCalibLoader::saveTimeWindow(const JPetTimeWindow& window)
{
  assert(fWriter);
  fWriter->write(window);
}

void TimeCalibLoader::terminate()
{
}

void TimeCalibLoader::setWriter(JPetWriter* writer)
{
  fWriter = writer;
}

void TimeCalibLoader::setParamManager(JPetParamManager* paramManager)
{
  fParamManager = paramManager;
}
