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
 *  @file SinogramCreator.h
 */

#ifndef SINOGRAMCREATOR_H
#define SINOGRAMCREATOR_H

#ifdef __CINT__
// when cint is used instead of compiler, override word is not recognized
// nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include "JPetGeomMapping/JPetGeomMapping.h"
#include "JPetHit/JPetHit.h"
#include "JPetUserTask/JPetUserTask.h"
#include "SinogramCreatorTools.h"
#include <string>
#include <vector>

#include "JPetFilterNone.h"
#include "JPetFilterRamLak.h"
#include "JPetRecoImageTools.h"

/**
 * @brief Module creating sinogram from data
 *
 * Input: *.reco.unk.evt
 * Output: *.sino with root diagrams and sinogram *.ppm with calculated sinogram.
 *
 * Module creates 2D x/y sinogram as *.ppm files for further reconstruction, e.g. FBP.
 *
 * It is also possible to split Z coordinate into smaller sections, then for each section sinogram is created.
 * Split is done by dividing Z coordinate into equal sections starting from negative values to positive, assuming that center of Z axis is in 0.
 * It is saved as matrix(fi, r) with values corresponding to number of hits registered with that angle and distance from center,
 * with header needed by ppm P2 file:
 * P2
 * (number of horizontal elements) (number of vertical elements)
 * (maximal value in matrix).
 *
 * It defines 5 user options:
 * - "SinogramCreator_OutFileName_std::string": defines output file name where sinogram is saved
 * - "SinogramCreator_ReconstructionDistanceAccuracy_float": (in cm) defines maximal round value for distance, in cm, e.g. 0.1 means 1 bin in sinogram
 * corresponds to 0.1 cm in reality
 * - "SinogramCreator_SinogramZSplitNumber_int": defines number of splits around "z" coordinate
 * - "SinogramCreator_ScintillatorLenght_float": defines scintillator lenght in "z" coordinate
 */
class SinogramCreator : public JPetUserTask {
public:
  explicit SinogramCreator(const char* name);
  virtual ~SinogramCreator();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  int getSplitRangeNumber(float firstZ, float secondZ);
  int getSplitRangeNumber(float z);
  void saveResult(const JPetRecoImageTools::SparseMatrix& result, const std::string& outputFileName);
  bool analyzeHits(const JPetHit& firstHit, const JPetHit& secondHit);
  bool analyzeHits(const float firstX, const float firstY, const float firstZ, const float firstTOF, const float secondX, const float secondY,
                   const float secondZ, const float secondTOF);
  int getSinogramSlice(float firstX, float firstY, float firstZ, float firstTOF, float secondX, float secondY, float secondZ, float secondTOF);
  float getTOFRescaleFactor(float x_diff, float y_diff, float z_diff);
  int maxValueInSinogram(const JPetRecoImageTools::SparseMatrix& sinogram);

  JPetRecoImageTools::SparseMatrix** fSinogram = nullptr;
  JPetRecoImageTools::Matrix2DTOF* fTOFInformation;

  const int kReconstructionMaxAngle = 180;
  int fZSplitNumber = 1;
  int fMaxDistanceNumber = 0;
  std::vector<std::pair<float, float>> fZSplitRange;

  int* fMaxValueInSinogram = nullptr; // to fill later in output file
  int* fCurrentValueInSinogram = nullptr;

  float fMaxReconstructionLayerRadius = 0.f;    // in cm
  float fReconstructionDistanceAccuracy = 0.1f; // in cm, 1mm accuracy, maximal accuracy: 0.001f
  float fScintillatorLenght = 50.0f;            // in cm
  bool fEnableObliqueLORRemapping = false;      // enable remapping LORs to correct sinogram slices based on TOF value
  bool fEnableKDEReconstruction = false;        // enable saving extra TOF information
  std::vector<int> fReconstructSliceNumbers;    // reconstruct only slices that was given in userParams

  const float kEPSILON = 0.0001f;

private:
  SinogramCreator(const SinogramCreator&) = delete;
  SinogramCreator& operator=(const SinogramCreator&) = delete;

  void setUpOptions();

  const std::string kOutFileNameKey = "SinogramCreator_OutFileName_std::string";
  const std::string kReconstructionDistanceAccuracy = "SinogramCreator_ReconstructionDistanceAccuracy_float";
  const std::string kZSplitNumber = "SinogramCreator_SinogramZSplitNumber_int";
  const std::string kScintillatorLenght = "SinogramCreator_ScintillatorLenght_float";
  const std::string kEnableObliqueLORRemapping = "SinogramCreator_EnableObliqueLORRemapping_bool";
  const std::string kEnableTOFReconstrution = "SinogramCreator_EnableKDEReconstruction_bool";
  const std::string kReconstructSliceNumbers = "SinogramCreator_ReconstructSliceNumbers_std::vector<int>";
  std::string fOutFileName = "sinogram";
};

#endif /*  !SINOGRAMCREATOR_H */
