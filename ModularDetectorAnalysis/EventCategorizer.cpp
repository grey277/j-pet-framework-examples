/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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
 *  @file EventCategorizer.cpp
 */

#include <boost/property_tree/json_parser.hpp>

#include "EventCategorizer.h"
#include "EventCategorizerTools.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include <iostream>

using namespace jpet_options_tools;
using namespace std;

EventCategorizer::EventCategorizer(const char* name) : JPetUserTask(name) {}

EventCategorizer::~EventCategorizer() {}

bool EventCategorizer::init()
{
  INFO("Event categorization started.");

  // Reading user parameters
  if (isOptionSet(fParams.getOptions(), k2gThetaDiffParamKey))
  {
    f2gThetaDiff = getOptionAsDouble(fParams.getOptions(), k2gThetaDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", k2gThetaDiffParamKey.c_str(), f2gThetaDiff));
  }

  if (isOptionSet(fParams.getOptions(), k2gTimeDiffParamKey))
  {
    f2gTimeDiff = getOptionAsDouble(fParams.getOptions(), k2gTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", k2gTimeDiffParamKey.c_str(), f2gTimeDiff));
  }

  // Reading TOT cut values
  if (isOptionSet(fParams.getOptions(), kTOTCutAnniMinParamKey))
  {
    fTOTCutAnniMin = getOptionAsDouble(fParams.getOptions(), kTOTCutAnniMinParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kTOTCutAnniMinParamKey.c_str(), fTOTCutAnniMin));
  }

  if (isOptionSet(fParams.getOptions(), kTOTCutAnniMaxParamKey))
  {
    fTOTCutAnniMax = getOptionAsDouble(fParams.getOptions(), kTOTCutAnniMaxParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kTOTCutAnniMaxParamKey.c_str(), fTOTCutAnniMax));
  }

  if (isOptionSet(fParams.getOptions(), kTOTCutDeexMinParamKey))
  {
    fTOTCutDeexMin = getOptionAsDouble(fParams.getOptions(), kTOTCutDeexMinParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kTOTCutDeexMinParamKey.c_str(), fTOTCutDeexMin));
  }

  if (isOptionSet(fParams.getOptions(), kTOTCutDeexMaxParamKey))
  {
    fTOTCutDeexMax = getOptionAsDouble(fParams.getOptions(), kTOTCutDeexMaxParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kTOTCutDeexMaxParamKey.c_str(), fTOTCutDeexMax));
  }

  // Cuts around source position
  if (isOptionSet(fParams.getOptions(), kSourceDistCutXYParamKey))
  {
    fSourceDistXYCut = getOptionAsDouble(fParams.getOptions(), kSourceDistCutXYParamKey);
  }
  else
  {
    WARNING(
        Form("No value of the %s parameter provided by the user. Using default value of %lf.", kSourceDistCutXYParamKey.c_str(), fSourceDistXYCut));
  }

  if (isOptionSet(fParams.getOptions(), kSourceDistCutZParamKey))
  {
    fSourceDistZCut = getOptionAsDouble(fParams.getOptions(), kSourceDistCutZParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kSourceDistCutZParamKey.c_str(), fSourceDistZCut));
  }

  // LOR cuts
  if (isOptionSet(fParams.getOptions(), kLORAngleCutParamKey))
  {
    fLORAngleCut = getOptionAsDouble(fParams.getOptions(), kLORAngleCutParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kLORAngleCutParamKey.c_str(), fLORAngleCut));
  }

  if (isOptionSet(fParams.getOptions(), kLORPosZCutParamKey))
  {
    fLORPosZCut = getOptionAsDouble(fParams.getOptions(), kLORPosZCutParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kLORPosZCutParamKey.c_str(), fLORPosZCut));
  }

  // Source position
  if (isOptionSet(fParams.getOptions(), kSourcePosXParamKey) && isOptionSet(fParams.getOptions(), kSourcePosYParamKey) &&
      isOptionSet(fParams.getOptions(), kSourcePosZParamKey))
  {
    auto x = getOptionAsDouble(fParams.getOptions(), kSourcePosXParamKey);
    auto y = getOptionAsDouble(fParams.getOptions(), kSourcePosYParamKey);
    auto z = getOptionAsDouble(fParams.getOptions(), kSourcePosZParamKey);
    fSourcePos.SetXYZ(x, y, z);
    INFO(Form("Source position is: %lf, %lf, %lf", x, y, z));
  }
  else
  {
    fSourcePos.SetXYZ(0.0, 0.0, 0.0);
    INFO("Source is positioned in (0, 0, 0).");
  }

  // Reading file with constants to property tree
  if (isOptionSet(fParams.getOptions(), kConstantsFileParamKey))
  {
    boost::property_tree::read_json(getOptionAsString(fParams.getOptions(), kConstantsFileParamKey), fConstansTree);
  }

  if (isOptionSet(fParams.getOptions(), kScatterTOFTimeDiffParamKey))
  {
    fScatterTOFTimeDiff = getOptionAsDouble(fParams.getOptions(), kScatterTOFTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kScatterTOFTimeDiffParamKey.c_str(),
                 fScatterTOFTimeDiff));
  }

  // Time variable used as +- axis limits for histograms with time spectra
  if (isOptionSet(fParams.getOptions(), kMaxTimeDiffParamKey))
  {
    fMaxTimeDiff = getOptionAsDouble(fParams.getOptions(), kMaxTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fMaxTimeDiff));
  }

  // Getting bools for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kSaveCalibHistosParamKey))
  {
    fSaveCalibHistos = getOptionAsBool(fParams.getOptions(), kSaveCalibHistosParamKey);
  }

  // Input events type
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  // Initialise hisotgrams
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }

  if (fSaveControlHistos)
  {
    initialiseCalibrationHistograms();
  }
  return true;
}

bool EventCategorizer::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    vector<JPetEvent> events;
    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++)
    {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));

      // Check types of current event
      auto lors =
          EventCategorizerTools::getLORs(event, getStatistics(), fSaveControlHistos, f2gThetaDiff, f2gTimeDiff, fTOTCutAnniMin, fTOTCutAnniMax);
      if (lors.size() > 0)
      {
        events.insert(events.end(), lors.begin(), lors.end());
      }
      // bool is2Gamma = EventCategorizerTools::checkFor2Gamma(event, getStatistics(), fSaveControlHistos, f2gThetaDiff, f2gTimeDiff, fTOTCutAnniMin,
      //                                                       fTOTCutAnniMax, fLORAngleCut, fLORPosZCut, fSourcePos);

      // Select hits for TOF calibration, if making calibraiton
      if (fSaveCalibHistos)
      {
        EventCategorizerTools::selectForTOF(event, getStatistics(), fSaveControlHistos, fTOTCutAnniMin, fTOTCutAnniMax, fTOTCutDeexMin,
                                            fTOTCutDeexMax, fSourcePos);

        EventCategorizerTools::selectForTimeWalk(event, getStatistics(), fSaveControlHistos, f2gThetaDiff, f2gTimeDiff, fTOTCutAnniMin,
                                                 fTOTCutAnniMax, fSourcePos);

        // bool isCollimator =
        //     EventCategorizerTools::collimator2Gamma(event, getStatistics(), fSaveControlHistos, f2gThetaDiff, f2gTimeDiff, fTOTCutAnniMin,
        //                                             fTOTCutAnniMax, fLORAngleCut, fLORPosZCut, fSourcePos, fConstansTree);
      }

      // Selection of other type of events is currently not used
      // bool is3Gamma = EventCategorizerTools::checkFor3Gamma(
      //   event, getStatistics(), fSaveControlHistos
      // );
      // bool isPrompt = EventCategorizerTools::checkForPrompt(
      //   event, getStatistics(), fSaveControlHistos, fTOTCutDeexMin, fTOTCutDeexMax
      // );
      // bool isScattered = EventCategorizerTools::checkForScatter(
      //   event, getStatistics(), fSaveControlHistos, fScatterTOFTimeDiff
      // );

      // JPetEvent newEvent = event;
      // if (is2Gamma)
      // {
      //   newEvent.addEventType(JPetEventType::k2Gamma);
      //   events.push_back(newEvent);
      // }
      // if(is3Gamma) newEvent.addEventType(JPetEventType::k3Gamma);
      // if(isPrompt) newEvent.addEventType(JPetEventType::kPrompt);
      // if(isScattered) newEvent.addEventType(JPetEventType::kScattered);
      //
      // if(fSaveControlHistos){
      //   for(auto hit : event.getHits()){
      //     getStatistics().getHisto2D("All_XYpos")->Fill(hit.getPosX(), hit.getPosY());
      //   }
      // }
    }
    saveEvents(events);
  }
  else
  {
    return false;
  }
  return true;
}

bool EventCategorizer::terminate()
{
  INFO("Event categorization completed.");
  return true;
}

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events)
  {
    fOutputEvents->add<JPetEvent>(event);
  }
}

void EventCategorizer::initialiseHistograms()
{
  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;
  double totUppLimit = 10000000.0;
  double revTOTLimit = 0.000000015;

  // Histograms for 2 gamama events
  getStatistics().createHistogram(new TH1F("2g_tot", "2 gamma event - average TOT scaled", 201, 0.0, totUppLimit));
  getStatistics().getHisto1D("2g_tot")->GetXaxis()->SetTitle("Time over Threshold [ps]");
  getStatistics().getHisto1D("2g_tot")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(new TH2F("2g_tot_scin", "2 gamma event - average TOT scaled per scintillator", maxScinID - minScinID + 1,
                                           minScinID - 0.5, maxScinID + 0.5, 201, 0.0, totUppLimit));
  getStatistics().getHisto2D("2g_tot_scin")->GetXaxis()->SetTitle("Scintillator ID");
  getStatistics().getHisto2D("2g_tot_scin")->GetYaxis()->SetTitle("Time over Threshold [ps]");

  getStatistics().createHistogram(
      new TH2F("2g_tot_z_pos", "2 gamma event - average TOT scaled vs. hit z position", 101, -25.5, 25.5, 201, 0.0, totUppLimit));
  getStatistics().getHisto2D("2g_tot_z_pos")->GetXaxis()->SetTitle("Hit z position [cm]");
  getStatistics().getHisto2D("2g_tot_z_pos")->GetYaxis()->SetTitle("Time over Threshold [ps]");

  getStatistics().createHistogram(new TH1F("2g_revtot", "2 gamma event - reversed TOT", 201, -revTOTLimit, revTOTLimit));
  getStatistics().getHisto1D("2g_revtot")->GetXaxis()->SetTitle("Reversd Time over Threshold [1/ps]");
  getStatistics().getHisto1D("2g_revtot")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(new TH2F("2g_revtot_scin", "2 gamma event - reversed TOT per scintillator", maxScinID - minScinID + 1,
                                           minScinID - 0.5, maxScinID + 0.5, 201, -revTOTLimit, revTOTLimit));
  getStatistics().getHisto2D("2g_revtot_scin")->GetXaxis()->SetTitle("Scintillator ID");
  getStatistics().getHisto2D("2g_revtot_scin")->GetYaxis()->SetTitle("Reversd Time over Threshold [1/ps]");

  getStatistics().createHistogram(
      new TH2F("2g_revtot_z_pos", "2 gamma event - reversed TOT vs. hit z position", 101, -25.5, 25.5, 201, -revTOTLimit, revTOTLimit));
  getStatistics().getHisto2D("2g_revtot_z_pos")->GetXaxis()->SetTitle("Hit z position [cm]");
  getStatistics().getHisto2D("2g_revtot_z_pos")->GetYaxis()->SetTitle("Reversd Time over Threshold [1/ps]");

  getStatistics().createHistogram(new TH1F("2g_tof", "2 gamma event - TOF calculated by convention", 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto1D("2g_tof")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("2g_tof")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH2F("2g_tof_scin", "2 gamma event - TOF calculated by convention per scintillator", maxScinID - minScinID + 1,
                                           minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto2D("2g_tof_scin")->GetXaxis()->SetTitle("Scintillator ID");
  getStatistics().getHisto2D("2g_tof_scin")->GetYaxis()->SetTitle("Time of Flight [ps]");

  getStatistics().createHistogram(new TH2F("2g_tof_z_pos", "2 gamma event - TOF calculated by convention vs. hit z position", 101, -25.5, 25.5, 201,
                                           -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto2D("2g_tof_z_pos")->GetXaxis()->SetTitle("Hit z position [cm]");
  getStatistics().getHisto2D("2g_tof_z_pos")->GetYaxis()->SetTitle("Time of Flight [ps]");

  getStatistics().createHistogram(new TH1F("2g_ab_tdiff", "2 gamma event - hits A-B time difference", 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto1D("2g_ab_tdiff")->GetXaxis()->SetTitle("A-B Signal Time Difference [ps]");
  getStatistics().getHisto1D("2g_ab_tdiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH2F("2g_ab_tdiff_scin", "2 gamma event - hits A-B time difference per scintillator", maxScinID - minScinID + 1,
                                           minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto2D("2g_ab_tdiff_scin")->GetXaxis()->SetTitle("Scintillator ID");
  getStatistics().getHisto2D("2g_ab_tdiff_scin")->GetYaxis()->SetTitle("A-B Signal Time Difference [ps]");

  getStatistics().createHistogram(new TH2F("2g_ab_tdiff_z_pos", "2 gamma event - hits A-B time difference vs. hit z position", 101, -25.5, 25.5, 201,
                                           -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto2D("2g_ab_tdiff_z_pos")->GetXaxis()->SetTitle("Hit z position [cm]");
  getStatistics().getHisto2D("2g_ab_tdiff_z_pos")->GetYaxis()->SetTitle("A-B Signal Time Difference [ps]");

  getStatistics().createHistogram(new TH1F("2g_theta", "2 gamma event - flight vectors theta", 181, -0.5, 180.5));
  getStatistics().getHisto1D("2g_theta")->GetXaxis()->SetTitle("Angle [degree]");
  getStatistics().getHisto1D("2g_theta")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH2F("2g_theta_scin", "2 gamma event - flight vectors theta per scintillator", maxScinID - minScinID + 1,
                                           minScinID - 0.5, maxScinID + 0.5, 181, -0.5, 180.5));
  getStatistics().getHisto2D("2g_theta_scin")->GetXaxis()->SetTitle("Scintillator ID");
  getStatistics().getHisto2D("2g_theta_scin")->GetYaxis()->SetTitle("Angle [degree]");

  getStatistics().createHistogram(
      new TH2F("2g_theta_z_pos", "2 gamma event - flight vectors theta vs. hit z position", 101, -25.5, 25.5, 181, -0.5, 180.5));
  getStatistics().getHisto2D("2g_theta_z_pos")->GetXaxis()->SetTitle("Hit z position [cm]");
  getStatistics().getHisto2D("2g_theta_z_pos")->GetYaxis()->SetTitle("Angle [degree]");

  // Cut stats
  getStatistics().createHistogram(
      new TH1F("cut_stats_none", "Hit pairs before cuts - scintillator occupancy", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5));
  getStatistics().getHisto1D("cut_stats_none")->GetXaxis()->SetTitle("Scintillator ID");
  getStatistics().getHisto1D("cut_stats_none")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
      new TH1F("cut_stats_a1", "Hits after theta cut - scintillator occupancy", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5));
  getStatistics().getHisto1D("cut_stats_a1")->GetXaxis()->SetTitle("Scintillator ID");
  getStatistics().getHisto1D("cut_stats_a1")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
      new TH1F("cut_stats_a2", "Hits after theta cut - scintillator occupancy", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5));
  getStatistics().getHisto1D("cut_stats_a2")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("cut_stats_a2")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(new TH1F("cut_stats_tof", "Hits after time difference cut - scintillator occupancy", maxScinID - minScinID + 1,
                                           minScinID - 0.5, maxScinID + 0.5));
  getStatistics().getHisto1D("cut_stats_tof")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("cut_stats_tof")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
      new TH1F("cut_stats_tot", "Hits after TOT cut - scintillator occupancy", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5));
  getStatistics().getHisto1D("cut_stats_tot")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("cut_stats_tot")->GetYaxis()->SetTitle("Number of Hits");

  // Cut result on other observables
  // TOF cut
  getStatistics().createHistogram(new TH1F("tof_cut_tot", "2 gamma event after TOF cut - average TOT scaled", 201, 0.0, totUppLimit));
  getStatistics().getHisto1D("tof_cut_tot")->GetXaxis()->SetTitle("Time over Threshold [ps]");
  getStatistics().getHisto1D("tof_cut_tot")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
      new TH1F("tof_cut_ab_tdiff", "2 gamma event after TOF cut - hits A-B time difference", 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto1D("tof_cut_ab_tdiff")->GetXaxis()->SetTitle("A-B Signal Time Difference [ps]");
  getStatistics().getHisto1D("tof_cut_ab_tdiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH1F("tof_cut_theta", "2 gamma event after TOF cut - theta between flight vectors", 181, -0.5, 180.5));
  getStatistics().getHisto1D("tof_cut_theta")->GetXaxis()->SetTitle("Angle [degree]");
  getStatistics().getHisto1D("tof_cut_theta")->GetYaxis()->SetTitle("Number of Hit Pairs");

  // After theta angle cut - back to back requirement
  getStatistics().createHistogram(
      new TH1F("theta_cut_tof", "2 gamma event after theta cut - TOF calculated by convention", 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto1D("theta_cut_tof")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("theta_cut_tof")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH1F("theta_cut_tot", "2 gamma event after theta cut - average TOT scaled", 201, 0.0, totUppLimit));
  getStatistics().getHisto1D("theta_cut_tot")->GetXaxis()->SetTitle("Time over Threshold [ps]");
  getStatistics().getHisto1D("theta_cut_tot")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
      new TH1F("theta_cut_ab_tdiff", "2 gamma event after theta cut - hits A-B time difference", 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto1D("theta_cut_ab_tdiff")->GetXaxis()->SetTitle("A-B Signal Time Difference [ps]");
  getStatistics().getHisto1D("theta_cut_ab_tdiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  // After TOT cut
  getStatistics().createHistogram(
      new TH1F("tot_cut_tof", "2 gamma event after TOT cut - TOF calculated by convention", 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto1D("theta_cut_tof")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("theta_cut_tof")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(
      new TH1F("tot_cut_ab_tdiff", "2 gamma event after TOT cut - hits A-B time difference", 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto1D("theta_cut_ab_tdiff")->GetXaxis()->SetTitle("A-B Signal Time Difference [ps]");
  getStatistics().getHisto1D("theta_cut_ab_tdiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH1F("tot_cut_theta", "2 gamma event after TOT cut - theta between flight vectors", 181, -0.5, 180.5));
  getStatistics().getHisto1D("tot_cut_theta")->GetXaxis()->SetTitle("Angle [degree]");
  getStatistics().getHisto1D("tot_cut_theta")->GetYaxis()->SetTitle("Number of Hit Pairs");

  // Events after cut - defined as annihilation event
  getStatistics().createHistogram(new TH1F("ap_tot", "Annihilation pairs average TOT scaled", 201, 0.0, totUppLimit));
  getStatistics().getHisto1D("ap_tot")->GetXaxis()->SetTitle("Time over Threshold [ps]");
  getStatistics().getHisto1D("ap_tot")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(new TH2F("ap_tot_scin", "Annihilation pairs average TOT scaled per scintillator", maxScinID - minScinID + 1,
                                           minScinID - 0.5, maxScinID + 0.5, 201, 0.0, totUppLimit));
  getStatistics().getHisto2D("ap_tot_scin")->GetXaxis()->SetTitle("Scintillator ID");
  getStatistics().getHisto2D("ap_tot_scin")->GetYaxis()->SetTitle("Time over Threshold [ps]");

  getStatistics().createHistogram(
      new TH2F("ap_tot_z_pos", "Annihilation pairs average TOT scaled vs. hit z position", 101, -25.5, 25.5, 201, 0.0, totUppLimit));
  getStatistics().getHisto2D("ap_tot_z_pos")->GetXaxis()->SetTitle("Hit z position [cm]");
  getStatistics().getHisto2D("ap_tot_z_pos")->GetYaxis()->SetTitle("Time over Threshold [ps]");

  getStatistics().createHistogram(new TH1F("ap_revtot", "Annihilation pairs reversed TOT", 201, -revTOTLimit, revTOTLimit));
  getStatistics().getHisto1D("ap_revtot")->GetXaxis()->SetTitle("Reversd Time over Threshold [1/ps]");
  getStatistics().getHisto1D("ap_revtot")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(new TH2F("ap_revtot_scin", "Annihilation pairs reversed TOT per scin", maxScinID - minScinID + 1, minScinID - 0.5,
                                           maxScinID + 0.5, 201, -revTOTLimit, revTOTLimit));
  getStatistics().getHisto2D("ap_revtot_scin")->GetXaxis()->SetTitle("Scintillator ID");
  getStatistics().getHisto2D("ap_revtot_scin")->GetYaxis()->SetTitle("Reversd Time over Threshold [1/ps]");

  getStatistics().createHistogram(
      new TH2F("ap_revtot_z_pos", "Annihilation pairs reversed TOT vs. hit z position", 101, -25.5, 25.5, 201, -revTOTLimit, revTOTLimit));
  getStatistics().getHisto2D("ap_revtot_z_pos")->GetXaxis()->SetTitle("Hit z position [cm]");
  getStatistics().getHisto2D("ap_revtot_z_pos")->GetYaxis()->SetTitle("Reversd Time over Threshold [1/ps]");

  getStatistics().createHistogram(
      new TH1F("ap_ab_tdiff", "Annihilation pairs hits A-B time difference after TOT cut", 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto1D("ap_ab_tdiff")->GetXaxis()->SetTitle("A-B Signal Time Difference [ps]");
  getStatistics().getHisto1D("ap_ab_tdiff")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(new TH2F("ap_ab_tdiff_scin", "Annihilation pairs hits A-B time difference per scintillator",
                                           maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto2D("ap_ab_tdiff_scin")->GetXaxis()->SetTitle("Scintillator ID");
  getStatistics().getHisto2D("ap_ab_tdiff_scin")->GetYaxis()->SetTitle("A-B Signal Time Difference [ps]");

  getStatistics().createHistogram(new TH2F("ap_ab_tdiff_z_pos", "Annihilation pairs hits A-B time difference vs. hit z position", 101, -25.5, 25.5,
                                           201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto2D("ap_ab_tdiff_z_pos")->GetXaxis()->SetTitle("Hit z position [cm]");
  getStatistics().getHisto2D("ap_ab_tdiff_z_pos")->GetYaxis()->SetTitle("A-B Signal Time Difference [ps]");

  getStatistics().createHistogram(new TH1F("ap_tof", "Annihilation pairs Time of Flight", 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto1D("ap_tof")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("ap_tof")->GetYaxis()->SetTitle("Number of Annihilation Pairs");

  getStatistics().createHistogram(new TH2F("ap_tof_scin", "Annihilation pairs Time of Flight per scintillator", maxScinID - minScinID + 1,
                                           minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto2D("ap_tof_scin")->GetXaxis()->SetTitle("Scintillator ID");
  getStatistics().getHisto2D("ap_tof_scin")->GetYaxis()->SetTitle("Time of Flight [ps]");

  getStatistics().createHistogram(
      new TH2F("ap_tof_z_pos", "Annihilation point Time of Flight vs. hit z position", 101, -25.5, 25.5, 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto2D("ap_tof_z_pos")->GetXaxis()->SetTitle("Hit z position [cm]");
  getStatistics().getHisto2D("ap_tof_z_pos")->GetYaxis()->SetTitle("Time of Flight [ps]");

  getStatistics().createHistogram(
      new TH1F("ap_tof_lor_cut", "Annihilation pairs Time of Flight after LOR angle cut", 201, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto1D("ap_tof_lor_cut")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("ap_tof_lor_cut")->GetYaxis()->SetTitle("Number of Annihilation Pairs");

  getStatistics().createHistogram(new TH2F("ap_yx", "YX position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5));
  getStatistics().getHisto2D("ap_yx")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("ap_yx")->GetYaxis()->SetTitle("X position [cm]");

  getStatistics().createHistogram(new TH2F("ap_zx", "ZX position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5));
  getStatistics().getHisto2D("ap_zx")->GetXaxis()->SetTitle("Z position [cm]");
  getStatistics().getHisto2D("ap_zx")->GetYaxis()->SetTitle("X position [cm]");

  getStatistics().createHistogram(new TH2F("ap_zy", "ZY position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5));
  getStatistics().getHisto2D("ap_zy")->GetXaxis()->SetTitle("Z position [cm]");
  getStatistics().getHisto2D("ap_zy")->GetYaxis()->SetTitle("Y position [cm]");

  getStatistics().createHistogram(new TH2F("ap_yx_zoom", "YX position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5));
  getStatistics().getHisto2D("ap_yx_zoom")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("ap_yx_zoom")->GetYaxis()->SetTitle("X position [cm]");

  getStatistics().createHistogram(new TH2F("ap_zx_zoom", "ZX position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5));
  getStatistics().getHisto2D("ap_zx_zoom")->GetXaxis()->SetTitle("Z position [cm]");
  getStatistics().getHisto2D("ap_zx_zoom")->GetYaxis()->SetTitle("X position [cm]");

  getStatistics().createHistogram(new TH2F("ap_zy_zoom", "ZY position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5));
  getStatistics().getHisto2D("ap_zy_zoom")->GetXaxis()->SetTitle("Z position [cm]");
  getStatistics().getHisto2D("ap_zy_zoom")->GetYaxis()->SetTitle("Y position [cm]");

  getStatistics().createHistogram(
      new TH2F("ap_yx_zoom_lor_cut", "YX position of annihilation point after LOR angle cut", 132, -16.5, 16.5, 132, -16.5, 16.5));
  getStatistics().getHisto2D("ap_yx_zoom_lor_cut")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("ap_yx_zoom_lor_cut")->GetYaxis()->SetTitle("X position [cm]");

  getStatistics().createHistogram(
      new TH2F("ap_zx_zoom_lor_cut", "ZX position of annihilation point after LOR angle cut", 132, -16.5, 16.5, 132, -16.5, 16.5));
  getStatistics().getHisto2D("ap_zx_zoom_lor_cut")->GetXaxis()->SetTitle("Z position [cm]");
  getStatistics().getHisto2D("ap_zx_zoom_lor_cut")->GetYaxis()->SetTitle("X position [cm]");

  getStatistics().createHistogram(
      new TH2F("ap_zy_zoom_lor_cut", "ZY position of annihilation point after LOR angle cut", 132, -16.5, 16.5, 132, -16.5, 16.5));
  getStatistics().getHisto2D("ap_zy_zoom_lor_cut")->GetXaxis()->SetTitle("Z position [cm]");
  getStatistics().getHisto2D("ap_zy_zoom_lor_cut")->GetYaxis()->SetTitle("Y position [cm]");

  getStatistics().createHistogram(
      new TH2F("ap_yx_zoom_z_cut", "YX position of annihilation point after Z position cut", 132, -16.5, 16.5, 132, -16.5, 16.5));
  getStatistics().getHisto2D("ap_yx_zoom_z_cut")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("ap_yx_zoom_z_cut")->GetYaxis()->SetTitle("X position [cm]");

  getStatistics().createHistogram(
      new TH2F("ap_zx_zoom_z_cut", "ZX position of annihilation point after Z position cut", 132, -16.5, 16.5, 132, -16.5, 16.5));
  getStatistics().getHisto2D("ap_zx_zoom_z_cut")->GetXaxis()->SetTitle("Z position [cm]");
  getStatistics().getHisto2D("ap_zx_zoom_z_cut")->GetYaxis()->SetTitle("X position [cm]");

  getStatistics().createHistogram(
      new TH2F("ap_zy_zoom_z_cut", "ZY position of annihilation point after Z position cut", 132, -16.5, 16.5, 132, -16.5, 16.5));
  getStatistics().getHisto2D("ap_zy_zoom_z_cut")->GetXaxis()->SetTitle("Z position [cm]");
  getStatistics().getHisto2D("ap_zy_zoom_z_cut")->GetYaxis()->SetTitle("Y position [cm]");

  // Histograms for scattering category
  // getStatistics().createHistogram(
  //   new TH1F("ScatterTOF_TimeDiff", "Difference of Scatter TOF and hits time difference",
  //     201, 0.0, 3.0*fScatterTOFTimeDiff));
  // getStatistics().getHisto1D("ScatterTOF_TimeDiff")->GetXaxis()->SetTitle("Scat_TOF & time diff [ps]");
  // getStatistics().getHisto1D("ScatterTOF_TimeDiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  // getStatistics().createHistogram(
  //    new TH2F("ScatterAngle_PrimaryTOT", "Angle of scattering vs. TOT of primary hits",
  //     181, -0.5, 180.5, 201, 0.0, 50000.0));
  // getStatistics().getHisto2D("ScatterAngle_PrimaryTOT")->GetXaxis()->SetTitle("Scattering Angle");
  // getStatistics().getHisto2D("ScatterAngle_PrimaryTOT")->GetYaxis()->SetTitle("TOT of primary hit [ps]");

  // getStatistics().createHistogram(
  //    new TH2F("ScatterAngle_ScatterTOT", "Angle of scattering vs. TOT of scattered hits",
  //     181, -0.5, 180.5, 201, 0.0, 50000.0));
  // getStatistics().getHisto2D("ScatterAngle_ScatterTOT")->GetXaxis()->SetTitle("Scattering Angle");
  // getStatistics().getHisto2D("ScatterAngle_ScatterTOT")->GetYaxis()->SetTitle("TOT of scattered hit [ps]");

  // Histograms for deexcitation
  // getStatistics().createHistogram(
  //   new TH1F("Deex_TOT_cut", "TOT of all hits with deex cut (30,50) ns",
  //     201, 25000.0, 55000.0));
  // getStatistics().getHisto1D("Deex_TOT_cut")->GetXaxis()->SetTitle("TOT [ps]");
  // getStatistics().getHisto1D("Deex_TOT_cut")->GetYaxis()->SetTitle("Number of Hits");
}

void EventCategorizer::initialiseCalibrationHistograms()
{
  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

  // Synchronization of TOF with annihilaion-deexcitation pairs
  getStatistics().createHistogram(new TH2F("tdiff_annih_scin", "A-D time difference for annihilation hit per scin", maxScinID - minScinID + 1,
                                           minScinID - 0.5, maxScinID + 0.5, 200, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto2D("tdiff_annih_scin")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto2D("tdiff_annih_scin")->GetYaxis()->SetTitle("Time diffrence [ps]");

  getStatistics().createHistogram(new TH2F("tdiff_deex_scin", "A-D time difference for deex hit per scin", maxScinID - minScinID + 1, minScinID - 0.5,
                                           maxScinID + 0.5, 200, -fMaxTimeDiff, fMaxTimeDiff));
  getStatistics().getHisto2D("tdiff_deex_scin")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto2D("tdiff_deex_scin")->GetYaxis()->SetTitle("Time diffrence [ps]");

  // Time walk histograms
  double revTOTLimit = 0.000000025;

  getStatistics().createHistogram(
      new TH2F("time_walk_ab_tdiff", "AB TDiff vs. reversed TOT", 200, -fMaxTimeDiff / 2.0, fMaxTimeDiff / 2.0, 200, -revTOTLimit, revTOTLimit));
  getStatistics().getHisto2D("time_walk_ab_tdiff")->GetXaxis()->SetTitle("AB Time Difference [ps]");
  getStatistics().getHisto2D("time_walk_ab_tdiff")->GetYaxis()->SetTitle("Reversed TOT [1/ps]");

  getStatistics().createHistogram(
      new TH2F("time_walk_tof", "TOF vs. reversed TOT", 200, -fMaxTimeDiff / 2.0, fMaxTimeDiff / 2.0, 200, -revTOTLimit, revTOTLimit));
  getStatistics().getHisto2D("time_walk_tof")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto2D("time_walk_tof")->GetYaxis()->SetTitle("Reversed TOT [1/ps]");

  // for (int scinID = minScinID; scinID <= maxScinID; ++scinID)
  // {
  //   getStatistics().createHistogram(new TH2F(Form("time_walk_ab_tdiff_scin_%d", scinID), Form("AB TDiff vs. reversed TOT scin %d", scinID), 201,
  //                                            -fMaxTimeDiff / 2.0, fMaxTimeDiff / 2.0, 201, -revTOTLimit, revTOTLimit));
  //   getStatistics().getHisto2D(Form("time_walk_ab_tdiff_scin_%d", scinID))->GetXaxis()->SetTitle("AB Time Difference [ps]");
  //   getStatistics().getHisto2D(Form("time_walk_ab_tdiff_scin_%d", scinID))->GetYaxis()->SetTitle("Reversed TOT [1/ps]");
  //
  //   getStatistics().createHistogram(new TH2F(Form("time_walk_tof_scin_%d", scinID), Form("TOF vs. reversed TOT scin %d", scinID), 201,
  //                                            -fMaxTimeDiff / 2.0, fMaxTimeDiff / 2.0, 201, -revTOTLimit, revTOTLimit));
  //   getStatistics().getHisto2D(Form("time_walk_tof_scin_%d", scinID))->GetXaxis()->SetTitle("AB Time Difference [ps]");
  //   getStatistics().getHisto2D(Form("time_walk_tof_scin_%d", scinID))->GetYaxis()->SetTitle("Reversed TOT [1/ps]");
  // }
}
