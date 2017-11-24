
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
 *  @file ImageReco.cpp
 */

#include "ImageReco.h"
#include <TH3D.h>
#include <TH1I.h>

const int numberOfBins = 100;
const int xRange = 100;
const int yRange = 100;
const int zRange = 50;

const int numberOfHitsInEventHisto = 10;

ImageReco::ImageReco(const char* name) : JPetUserTask(name) { }

ImageReco::~ImageReco() { }

bool ImageReco::init()
{
  getStatistics().createHistogram(new TH3D("hits_pos",
                                  "Reconstructed hit pos",
                                  numberOfBins, -xRange, xRange,
                                  numberOfBins, -yRange, yRange,
                                  numberOfBins, -zRange, zRange));
  getStatistics().createHistogram(new TH1I("number_of_events",
                                  "Number of events with n hits",
                                  numberOfHitsInEventHisto, 0, numberOfHitsInEventHisto));
  return true;
}

bool ImageReco::exec()
{
  if (auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++) {
      auto event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](static_cast<int>(i)));
      getStatistics().getHisto<TH1I>("number_of_events").Fill(event.getHits().size());
      auto numberOfHits = event.getHits().size();
      if (numberOfHits == 1 || numberOfHits <= 0)
        continue;
      else {
        auto hits = event.getHits();
        for (unsigned int i = 0; i < hits.size() - 1; i ++) {
          calculateReconstructedPosition(hits[i], hits[i + 1]);
        }
      }
    }
  }
  return true;
}

bool ImageReco::terminate()
{
  //getStatistics().getHisto<TH3D>("hits_pos").SetShowProjection("yx Col", 100);
  return true;
}

bool ImageReco::calculateReconstructedPosition(const JPetHit& firstHit, const JPetHit& secondHit)
{
  double s1_a = static_cast<double>(firstHit.getSignalA().getTime());
  double s1_b = static_cast<double>(firstHit.getSignalB().getTime());
  double s2_a = static_cast<double>(secondHit.getSignalA().getTime());
  double s2_b = static_cast<double>(secondHit.getSignalB().getTime());

  double t_s1_ab = s1_a - s1_b;
  double t_s2_ab = s2_a - s2_b;

  double s1_z = (t_s1_ab * 11) / 2.0;
  double s2_z = (t_s2_ab * 11) / 2.0;

  double vdx = secondHit.getPosX() - firstHit.getPosX();
  double vdz = s2_z - s1_z;
  double vdy = secondHit.getPosY() - firstHit.getPosY();

  double dd = std::sqrt(vdx * vdx + vdz * vdz + vdy * vdy);

  double mtof_a = 0;
  if (firstHit.getPosY() > secondHit.getPosY()) {
    mtof_a = ((((s1_a + s1_b) / 2.0) - ((s2_a + s2_b) / 2.0)) * 30);
  } else {
    mtof_a = ((((s2_a + s2_b) / 2.0) - ((s1_a + s1_b) / 2.0)) * 30);
  }
  double x, y, z;
  x = firstHit.getPosX() + ((vdx / 2.0) + (vdx / dd * mtof_a));
  y = firstHit.getPosY() + ((vdy / 2.0) + (vdy / dd * mtof_a));
  z = s1_z + ((vdz / 2.0) + (vdz / dd * mtof_a));
  if (x > -xRange && x < xRange && y > -yRange && y < yRange && z > -zRange && z < zRange) {
    getStatistics().getHisto<TH3D>("hits_pos").Fill(x, y, z);
    return true;
  }
  return false;
}