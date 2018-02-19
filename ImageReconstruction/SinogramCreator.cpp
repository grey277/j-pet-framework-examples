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
 *  @file SinogramCreator.cpp
 */

#include "SinogramCreator.h"
#include <TH2F.h>
using namespace jpet_options_tools;

SinogramCreator::SinogramCreator(const char* name) : JPetUserTask(name) {}

SinogramCreator::~SinogramCreator() {}

bool SinogramCreator::init()
{
  setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  getStatistics().createHistogram(new TH2I("reconstuction_histogram",
                                  "reconstuction_histogram",
                                  std::ceil(fReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1, -fReconstructionLayerRadius, fReconstructionLayerRadius,
                                  std::ceil((fReconstructionEndAngle - fReconstructionStartAngle) / fReconstructionAngleStep), fReconstructionStartAngle, fReconstructionEndAngle));

  return true;
}

bool SinogramCreator::exec()
{
  unsigned int currentValueInSinogram = 0; // holds current bin value of sinogram
  float reconstructionAngleDiff = std::abs(fReconstructionEndAngle - fReconstructionStartAngle); // should be always positive

  unsigned int maxThetaNumber = std::ceil(reconstructionAngleDiff / fReconstructionAngleStep);
  unsigned int maxDistanceNumber = std::ceil(fReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1;
  if (fSinogram == nullptr) {
    fSinogram = new SinogramResultType(maxDistanceNumber, (std::vector<unsigned int>(maxThetaNumber)));
  }
  if (const auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++) {
      auto event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](static_cast<int>(i)));
      auto hits = event.getHits();
      if (hits.size() == 2) {
        const auto& firstHit = hits[0];
        const auto& secondHit = hits[1];
        if (checkLayer(firstHit) && checkLayer(secondHit)) {
          for (float theta = fReconstructionStartAngle; theta < fReconstructionEndAngle; theta += fReconstructionAngleStep) {
            float x = fReconstructionLayerRadius * std::cos(theta * (M_PI / 180.f)); // calculate x,y positon of line with theta angle from line (0,0) = theta
            float y = fReconstructionLayerRadius * std::sin(theta * (M_PI / 180.f));
            std::pair<float, float> intersectionPoint = SinogramCreatorTools::lineIntersection(std::make_pair(-x, -y), std::make_pair(x, y),
                std::make_pair(firstHit.getPosX(), firstHit.getPosY()), std::make_pair(secondHit.getPosX(), secondHit.getPosY())); //find intersection point
            if (intersectionPoint.first != std::numeric_limits<float>::max() && intersectionPoint.second != std::numeric_limits<float>::max()) { // check is there is intersection point
              float distance = SinogramCreatorTools::length2D(intersectionPoint.first, intersectionPoint.second);
              if (distance >= fReconstructionLayerRadius) // if distance is greather then our max reconstuction layer radius, it cant be placed in sinogram
                continue;
              if (intersectionPoint.first < 0.f)
                distance = -distance;
              int distanceRound = std::floor((fReconstructionLayerRadius / fReconstructionDistanceAccuracy)
                                             + fReconstructionDistanceAccuracy)
                                  + std::floor((distance / fReconstructionDistanceAccuracy) + fReconstructionDistanceAccuracy); //clever way of rounding to nearest multipicity of accuracy digit and change it to int
              int thetaNumber = std::round(theta / fReconstructionAngleStep);                                                   // round because of floating point
              currentValueInSinogram = ++ fSinogram->at(distanceRound).at(thetaNumber); // add to sinogram
              if (currentValueInSinogram >= fMaxValueInSinogram)
                fMaxValueInSinogram = currentValueInSinogram; // save max value of sinogram
              getStatistics().getObject<TH2I>("reconstuction_histogram")->Fill(distance, theta); //add to histogram
            }
          }
        }
      }
    }
  } else {
    ERROR("Returned event is not TimeWindow");
    return false;
  }
  return true;
}

bool SinogramCreator::checkLayer(const JPetHit& hit)
{
  return hit.getBarrelSlot().getLayer().getID() == 1;
}

bool SinogramCreator::terminate()
{
  std::ofstream res(fOutFileName);
  res << "P2" << std::endl;
  res << (*fSinogram)[0].size() << " " << fSinogram->size() << std::endl;
  res << fMaxValueInSinogram << std::endl;
  for (unsigned int i = 0; i < fSinogram->size(); i++) {
    for (unsigned int j = 0; j < (*fSinogram)[0].size(); j++) {
      res << (*fSinogram)[i][j] << " ";
    }
    res << std::endl;
  }
  res.close();
  return true;
}

void SinogramCreator::setUpOptions()
{
  auto opts = getOptions();
  if (isOptionSet(opts, kOutFileNameKey)) {
    fOutFileName = getOptionAsString(opts, kOutFileNameKey);
  }

  if (isOptionSet(opts, kReconstructionLayerRadiusKey)) {
    fReconstructionLayerRadius = getOptionAsFloat(opts, kReconstructionLayerRadiusKey);
  }

  if (isOptionSet(opts, kReconstructionStartAngle)) {
    fReconstructionStartAngle = getOptionAsFloat(opts, kReconstructionStartAngle);
  }

  if (isOptionSet(opts, kReconstructionEndAngle)) {
    fReconstructionEndAngle = getOptionAsFloat(opts, kReconstructionEndAngle);
  }

  if (isOptionSet(opts, kReconstructionDistanceAccuracy)) {
    fReconstructionDistanceAccuracy = getOptionAsFloat(opts, kReconstructionDistanceAccuracy);
  }

  if (isOptionSet(opts, kReconstructionAngleStep)) {
    fReconstructionAngleStep = getOptionAsFloat(opts, kReconstructionAngleStep);
  }
}