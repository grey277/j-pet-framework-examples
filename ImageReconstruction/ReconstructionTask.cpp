/**
 *  @copyright Copyright 2017 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  @file ReconstructionTask.cpp
 */

#include "ReconstructionTask.h"

using namespace jpet_options_tools;

ReconstructionTask::ReconstructionTask(const char* name) : JPetUserTask(name) {}

ReconstructionTask::~ReconstructionTask() {}

bool ReconstructionTask::init()
{
  setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");
  fSinogram = JPetSinogramType::readMapFromFile(fInFileName, "Sinogram");
  if (!fSinogram)
  {
    return false;
  }
  return true;
}

bool ReconstructionTask::exec() { return true; }

bool ReconstructionTask::terminate()
{
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW1D;
  const auto& sinogram = fSinogram->getSinogram();
  unsigned int zSplitNumber = fSinogram->getZSplitNumber();
  JPetRecoImageTools::FilteredBackProjectionWeightingFunction weightFunction;
  static std::map<std::string, int> reconstructionNameToWeight{{"FBP", ReconstructionTask::kWeightingType::kFBP},
                                                               {"TOFFBP", ReconstructionTask::kWeightingType::kTOFFBP}};

  switch (reconstructionNameToWeight[fReconstructionName])
  {
  case ReconstructionTask::kWeightingType::kFBP:
    weightFunction = JPetRecoImageTools::FBPWeight;
    break;
  case ReconstructionTask::kWeightingType::kTOFFBP:
    weightFunction = JPetRecoImageTools::FBPTOFWeight;
    break;
  default:
    ERROR("Could not find reconstruction name: " + fReconstructionName + ", using FBP.");
    weightFunction = JPetRecoImageTools::FBPWeight;
    break;
  }

  static std::map<std::string, int> filterNameToFilter{{"None", ReconstructionTask::kFilterType::kFilterNone},
                                                       {"Cosine", ReconstructionTask::kFilterType::kFilterCosine},
                                                       {"Hamming", ReconstructionTask::kFilterType::kFilterHamming},
                                                       {"Hann", ReconstructionTask::kFilterType::kFilterHann},
                                                       {"Ridgelet", ReconstructionTask::kFilterType::kFilterRidgelet},
                                                       {"SheppLogan", ReconstructionTask::kFilterType::kFilterSheppLogan}};

  for (unsigned int i = 0; i < zSplitNumber; i++)
  { // loop throught Z slices
    int sliceNumber = i - (zSplitNumber / 2);
    if (!fReconstructSliceNumbers.empty()) // if there we want to reconstruct only selected z slices, skip others
      if (std::find(fReconstructSliceNumbers.begin(), fReconstructSliceNumbers.end(), sliceNumber) == fReconstructSliceNumbers.end())
        continue;
    for (float cutOffValue = fCutOffValueBegin; cutOffValue <= fCutOffValueEnd; cutOffValue += fCutOffValueStep)
    {
      JPetFilterInterface* filter;

      switch (filterNameToFilter[fFilterName])
      {
      case ReconstructionTask::kFilterType::kFilterNone:
        filter = new JPetFilterNone(cutOffValue);
        break;
      case ReconstructionTask::kFilterType::kFilterCosine:
        filter = new JPetFilterCosine(cutOffValue);
        break;
      case ReconstructionTask::kFilterType::kFilterHamming:
        filter = new JPetFilterHamming(cutOffValue);
        break;
      case ReconstructionTask::kFilterType::kFilterHann:
        filter = new JPetFilterHann(cutOffValue);
        break;
      case ReconstructionTask::kFilterType::kFilterRidgelet:
        filter = new JPetFilterRidgelet(cutOffValue);
        break;
      case ReconstructionTask::kFilterType::kFilterSheppLogan:
        filter = new JPetFilterSheppLogan(cutOffValue);
        break;
      default:
        ERROR("Could not find filter: " + fFilterName + ", using JPetFilterNone.");
        filter = new JPetFilterNone(cutOffValue);
        break;
      }

      JPetSinogramType::Matrix3D filtered;
      int tofID = 0;
      for (auto& tofWindow : sinogram[i]) // filter sinogram in each TOF-windows(for FBP in single timewindow)
      {
        filtered[tofWindow.first] = JPetRecoImageTools::FilterSinogram(f, *filter, tofWindow.second);
        tofID++;
      }

      JPetSinogramType::SparseMatrix result =
          JPetRecoImageTools::backProject(filtered, fSinogram->getReconstructionDistanceAccuracy(), fSinogram->getTOFWindowSize(), fLORTOFSigma,
                                          weightFunction, JPetRecoImageTools::rescale, 0, 10000);

      SinogramCreatorTools::saveResult(result, fOutFileName + "reconstruction_with_" + fReconstructionName + "_" + fFilterName + "_CutOff_" + std::to_string(cutOffValue) +
                             "_slicenumber_" + std::to_string(sliceNumber) + ".ppm");
      delete filter;
    }
  }
  return true;
}

void ReconstructionTask::setUpOptions()
{
  auto opts = getOptions();
  if (isOptionSet(opts, kInFileNameKey))
  {
    fInFileName = getOptionAsString(opts, kInFileNameKey);
  }
  if (isOptionSet(opts, kOutFileNameKey))
  {
    fOutFileName = getOptionAsString(opts, kOutFileNameKey);
  }
  if (isOptionSet(opts, kReconstructSliceNumbers))
  {
    fReconstructSliceNumbers = getOptionAsVectorOfInts(opts, kReconstructSliceNumbers);
  }
  if (isOptionSet(opts, kFilterCutOffValueBegin))
  {
    fCutOffValueBegin = getOptionAsFloat(opts, kFilterCutOffValueBegin);
  }
  if (isOptionSet(opts, kFilterCutOffValueEnd))
  {
    fCutOffValueEnd = getOptionAsFloat(opts, kFilterCutOffValueEnd);
  }
  if (isOptionSet(opts, kFilterCutOffValueStep))
  {
    fCutOffValueStep = getOptionAsFloat(opts, kFilterCutOffValueStep);
  }
  if (isOptionSet(opts, kFilterAlpha))
  {
    fFilterAlphaValue = getOptionAsFloat(opts, kFilterAlpha);
  }
  if (isOptionSet(opts, kFilterName))
  {
    fFilterName = getOptionAsString(opts, kFilterName);
  }
  if (isOptionSet(opts, kReconstructionName))
  {
    fReconstructionName = getOptionAsString(opts, kReconstructionName);
  }
}
