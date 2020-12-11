/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include "EventCategorizerTools.h"
#include "EventCategorizer.h"
#include <iostream>

using namespace jpet_options_tools;
using namespace std;

EventCategorizer::EventCategorizer(const char* name): JPetUserTask(name) {}

EventCategorizer::~EventCategorizer() {}

bool EventCategorizer::init()
{
  INFO("Event categorization started.");
   // Parameter for back to back categorization
   if (isOptionSet(fParams.getOptions(), kBack2BackSlotThetaDiffParamKey)){
     fB2BSlotThetaDiff = getOptionAsDouble(fParams.getOptions(), kBack2BackSlotThetaDiffParamKey);
   } else {
     WARNING(Form(
       "No value of the %s parameter provided by the user. Using default value of %lf.",
       kBack2BackSlotThetaDiffParamKey.c_str(), fB2BSlotThetaDiff
     ));
   }

   // Parameter for scattering determination
   if (isOptionSet(fParams.getOptions(), kScatterTOFTimeDiffParamKey)) {
     fScatterTOFTimeDiff = getOptionAsDouble(fParams.getOptions(), kScatterTOFTimeDiffParamKey);
   } else {
     WARNING(Form(
       "No value of the %s parameter provided by the user. Using default value of %lf.",
       kScatterTOFTimeDiffParamKey.c_str(), fScatterTOFTimeDiff
     ));
   }

   // Reading file with effective light velocit and TOF synchronization constants to property tree
   // if (isOptionSet(fParams.getOptions(), kConstantsFileParamKey)) {
   //   boost::property_tree::read_json(getOptionAsString(fParams.getOptions(), kConstantsFileParamKey), fConstansTree);
   // }

   if (isOptionSet(fParams.getOptions(), kMaxTimeDiffParamKey)) {
     fMaxTimeDiff = getOptionAsDouble(fParams.getOptions(), kMaxTimeDiffParamKey);
   } else {
     WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fMaxTimeDiff));
   }

   // Reading TOT cut values
   if (isOptionSet(fParams.getOptions(), kTOTCutAnniMinParamKey)) {
     fTOTCutAnniMin = getOptionAsDouble(fParams.getOptions(), kTOTCutAnniMinParamKey);
   } else {
     WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kTOTCutAnniMinParamKey.c_str(), fTOTCutAnniMin));
   }

   if (isOptionSet(fParams.getOptions(), kTOTCutAnniMaxParamKey)) {
     fTOTCutAnniMax = getOptionAsDouble(fParams.getOptions(), kTOTCutAnniMaxParamKey);
   } else {
     WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fTOTCutAnniMax));
   }

   if (isOptionSet(fParams.getOptions(), kTOTCutDeexMinParamKey)) {
     fTOTCutDeexMin = getOptionAsDouble(fParams.getOptions(), kTOTCutDeexMinParamKey);
   } else {
     WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fTOTCutDeexMin));
   }

   if (isOptionSet(fParams.getOptions(), kTOTCutDeexMaxParamKey)) {
     fTOTCutDeexMax = getOptionAsDouble(fParams.getOptions(), kTOTCutDeexMaxParamKey);
   } else {
     WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fTOTCutDeexMax));
   }
   // Getting bool for saving histograms
   if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
     fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
   }
   if (isOptionSet(fParams.getOptions(), kSaveCalibHistosParamKey)) {
     fSaveCalibHistos = getOptionAsBool(fParams.getOptions(), kSaveCalibHistosParamKey);
   }

   // Input events type
   fOutputEvents = new JPetTimeWindow("JPetEvent");
   // Initialise hisotgrams
   if(fSaveControlHistos) initialiseHistograms();
   return true;
}

bool EventCategorizer::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    vector<JPetEvent> events;
    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++) {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));

      // Check types of current event
      bool is2Gamma = EventCategorizerTools::checkFor2Gamma(
        event, getStatistics(), fSaveControlHistos, fB2BSlotThetaDiff,
        fMaxTimeDiff, fTOTCutAnniMin, fTOTCutAnniMax
      );

      // Select hits for TOF calibration, if making calibraiton
      if(fSaveCalibHistos){
        EventCategorizerTools::selectForCalibration(
          event, getStatistics(), fSaveControlHistos,
          fTOTCutAnniMin, fTOTCutAnniMax, fTOTCutDeexMin, fTOTCutDeexMax
        );
      }

      // bool is3Gamma = EventCategorizerTools::checkFor3Gamma(
      //   event, getStatistics(), fSaveControlHistos
      // );
      // bool isPrompt = EventCategorizerTools::checkForPrompt(
      //   event, getStatistics(), fSaveControlHistos, fTOTCutDeexMin, fTOTCutDeexMax
      // );
      // bool isScattered = EventCategorizerTools::checkForScatter(
      //   event, getStatistics(), fSaveControlHistos, fScatterTOFTimeDiff
      // );

      JPetEvent newEvent = event;
      if(is2Gamma) newEvent.addEventType(JPetEventType::k2Gamma);
      // if(is3Gamma) newEvent.addEventType(JPetEventType::k3Gamma);
      // if(isPrompt) newEvent.addEventType(JPetEventType::kPrompt);
      // if(isScattered) newEvent.addEventType(JPetEventType::kScattered);
      //
      // if(fSaveControlHistos){
      //   for(auto hit : event.getHits()){
      //     getStatistics().getHisto2D("All_XYpos")->Fill(hit.getPosX(), hit.getPosY());
      //   }
      // }
      events.push_back(newEvent);
    }
    saveEvents(events);
  } else { return false; }
  return true;
}

bool EventCategorizer::terminate()
{
  INFO("Event categorization completed.");
  return true;
}

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) { fOutputEvents->add<JPetEvent>(event); }
}

void EventCategorizer::initialiseHistograms(){

  // Histograms for 2Gamama selection
  getStatistics().createHistogram(
    new TH1F("2g_tot", "Opposite module hits TOT div. by multiplicity", 100, 0.0, 400000.0)
  );
  getStatistics().getHisto1D("2g_tot")->GetXaxis()->SetTitle("Time over Threshold [ps]");
  getStatistics().getHisto1D("2g_tot")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
    new TH1F("2g_tdiff", "Opposite module hits registration time difference", 200, 0.0, 1.1*fMaxTimeDiff)
  );
  getStatistics().getHisto1D("2g_tdiff")->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto1D("2g_tdiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(
    new TH1F("2g_tof", "Opposite module hits TOF", 200, -15000.0, 15000.0)
  );
  getStatistics().getHisto1D("2g_tof")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("2g_tof")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(
    new TH1F("2g_hit_tdiff", "Opposite module hits A-B time difference", 100, -15000.0, 15000.0)
  );
  getStatistics().getHisto1D("2g_hit_tdiff")->GetXaxis()->SetTitle("A-B Signal Time Difference [ps]");
  getStatistics().getHisto1D("2g_hit_tdiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  // TDiff anfter TOT cut
  getStatistics().createHistogram(
    new TH1F("2g_hit_tdiff_cut_tot", "Opposite module hits A-B time difference after TOT cut", 100, -15000.0, 15000.0)
  );
  getStatistics().getHisto1D("2g_hit_tdiff_cut_tot")->GetXaxis()->SetTitle("A-B Signal Time Difference [ps]");
  getStatistics().getHisto1D("2g_hit_tdiff_cut_tot")->GetYaxis()->SetTitle("Number of Hit Pairs");

  // Histograms for slots
  auto minSlotID = getParamBank().getSlots().begin()->first;
  auto maxSlotID = getParamBank().getSlots().rbegin()->first;

  for(int slotID = minSlotID; slotID <= maxSlotID; slotID++){
    getStatistics().createHistogram(new TH1F(
      Form("2g_tof_slot_%d", slotID),
      Form("TOF of 2 gamma event in Slot %d", slotID),
      200, -15000.0, 15000.0)
    );
    getStatistics().getHisto1D(Form("2g_tof_slot_%d", slotID))->GetXaxis()->SetTitle("Time of Flight [ps]");
    getStatistics().getHisto1D(Form("2g_tof_slot_%d", slotID))->GetYaxis()->SetTitle("Number of Hit Pairs");
  }

  // Annihilation point histograms
  getStatistics().createHistogram(
    new TH1F("ann_point_tof", "Annihilation pairs Time of Flight", 200, -15000.0, 15000.0));
  getStatistics().getHisto1D("ann_point_tof")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("ann_point_tof")->GetYaxis()->SetTitle("Number of Annihilation Pairs");

  getStatistics().createHistogram(
     new TH2F("ann_point_xy", "XY position of annihilation point", 101, -50.5, 50.5, 101, -50.5, 50.5));
  getStatistics().getHisto2D("ann_point_xy")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("ann_point_xy")->GetYaxis()->SetTitle("X position [cm]");

  getStatistics().createHistogram(
    new TH2F("ann_point_xz", "XZ position of annihilation point", 101, -50.5, 50.5, 101, -50.5, 50.5));
  getStatistics().getHisto2D("ann_point_xz")->GetXaxis()->SetTitle("X position [cm]");
  getStatistics().getHisto2D("ann_point_xz")->GetYaxis()->SetTitle("Z position [cm]");

  getStatistics().createHistogram(
    new TH2F("ann_point_yz", "YZ position of annihilation point", 101, -50.5, 50.5, 101, -50.5, 50.5));
  getStatistics().getHisto2D("ann_point_yz")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("ann_point_yz")->GetYaxis()->SetTitle("Z position [cm]");

  getStatistics().createHistogram(
     new TH2F("ann_point_xy_zoom", "XY position of annihilation point", 101, -10.5, 10.5, 101, -10.5, 10.5));
  getStatistics().getHisto2D("ann_point_xy_zoom")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("ann_point_xy_zoom")->GetYaxis()->SetTitle("X position [cm]");

  getStatistics().createHistogram(
    new TH2F("ann_point_xz_zoom", "XZ position of annihilation point", 101, -10.5, 10.5, 101, -10.5, 10.5));
  getStatistics().getHisto2D("ann_point_xz_zoom")->GetXaxis()->SetTitle("X position [cm]");
  getStatistics().getHisto2D("ann_point_xz_zoom")->GetYaxis()->SetTitle("Z position [cm]");

  getStatistics().createHistogram(
    new TH2F("ann_point_yz_zoom", "YZ position of annihilation point", 101, -10.5, 10.5, 101, -10.5, 10.5));
  getStatistics().getHisto2D("ann_point_yz_zoom")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("ann_point_yz_zoom")->GetYaxis()->SetTitle("Z position [cm]");

  // Histograms for scintillator synchronization constatns
  if(fSaveCalibHistos){
    auto minScinID = getParamBank().getScins().begin()->first;
    auto maxScinID = getParamBank().getScins().rbegin()->first;

    for(int scinID = minScinID; scinID<= maxScinID; scinID++){
      getStatistics().createHistogram(new TH1F(
        Form("tdiff_a_d_scin_%d", scinID),
        Form("TDiff annihilation hit from scin %d and deex hit", scinID),
        200, -15000.0, 15000.0
      ));

      getStatistics().createHistogram(new TH1F(
        Form("tdiff_d_a_scin_%d", scinID),
        Form("TDiff deex hit from scin %d and annihilation hit", scinID),
        200, -15000.0, 15000.0
      ));
    }
  }
  // General histograms
  // getStatistics().createHistogram(
  //   new TH2F("All_XYpos", "Hits position XY", 222, -50.5, 50.5, 222, -50.5, 50.5));
  // getStatistics().getHisto2D("All_XYpos")->GetXaxis()->SetTitle("Y position [cm]");
  // getStatistics().getHisto2D("All_XYpos")->GetYaxis()->SetTitle("X position [cm]");

  // Histograms for 2Gamama category
  // getStatistics().createHistogram(
  //   new TH2F("2Gamma_XYpos", "2 gamma hits position XY", 222, -50.5, 50.5, 222, -50.5, 50.5));
  // getStatistics().getHisto2D("All_XYpos")->GetXaxis()->SetTitle("Y position [cm]");
  // getStatistics().getHisto2D("All_XYpos")->GetYaxis()->SetTitle("X position [cm]");

  // getStatistics().createHistogram(
  //   new TH1F("2Gamma_Zpos", "B2B hits Z position", 200, -50.0, 50.0));
  // getStatistics().getHisto1D("2Gamma_Zpos")->GetXaxis()->SetTitle("Z axis position [cm]");
  // getStatistics().getHisto1D("2Gamma_Zpos")->GetYaxis()->SetTitle("Number of Hits");

  // getStatistics().createHistogram(
  //   new TH1F("2Gamma_TimeDiff", "B2B hits time difference", 100, -10000.0, 10000.0));
  // getStatistics().getHisto1D("2Gamma_TimeDiff")->GetXaxis()->SetTitle("Time Difference [ps]");
  // getStatistics().getHisto1D("2Gamma_TimeDiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  // getStatistics().createHistogram(
  //   new TH1F("2Gamma_Dist", "B2B hits distance", 150, 0.0, 150.0));
  // getStatistics().getHisto1D("2Gamma_Dist")->GetXaxis()->SetTitle("Distance [cm]");
  // getStatistics().getHisto1D("2Gamma_Dist")->GetYaxis()->SetTitle("Number of Hit Pairs");

  // Histograms for 3Gamama category
  // getStatistics().createHistogram(
  //   new TH2F("3Gamma_Angles", "Relative angles - transformed", 251, -0.5, 250.5, 201, -0.5, 200.5));
  // getStatistics().getHisto2D("3Gamma_Angles")->GetXaxis()->SetTitle("Relative angle 1-2");
  // getStatistics().getHisto2D("3Gamma_Angles")->GetYaxis()->SetTitle("Relative angle 2-3");

  // Histograms for scattering category
  // getStatistics().createHistogram(
  //   new TH1F("ScatterTOF_TimeDiff", "Difference of Scatter TOF and hits time difference",
  //     200, 0.0, 3.0*fScatterTOFTimeDiff));
  // getStatistics().getHisto1D("ScatterTOF_TimeDiff")->GetXaxis()->SetTitle("Scat_TOF & time diff [ps]");
  // getStatistics().getHisto1D("ScatterTOF_TimeDiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  // getStatistics().createHistogram(
  //    new TH2F("ScatterAngle_PrimaryTOT", "Angle of scattering vs. TOT of primary hits",
  //     181, -0.5, 180.5, 200, 0.0, 40000.0));
  // getStatistics().getHisto2D("ScatterAngle_PrimaryTOT")->GetXaxis()->SetTitle("Scattering Angle");
  // getStatistics().getHisto2D("ScatterAngle_PrimaryTOT")->GetYaxis()->SetTitle("TOT of primary hit [ps]");

  // getStatistics().createHistogram(
  //    new TH2F("ScatterAngle_ScatterTOT", "Angle of scattering vs. TOT of scattered hits",
  //     181, -0.5, 180.5, 200, 0.0, 40000.0));
  // getStatistics().getHisto2D("ScatterAngle_ScatterTOT")->GetXaxis()->SetTitle("Scattering Angle");
  // getStatistics().getHisto2D("ScatterAngle_ScatterTOT")->GetYaxis()->SetTitle("TOT of scattered hit [ps]");

  // Histograms for deexcitation
  // getStatistics().createHistogram(
  //   new TH1F("Deex_TOT_cut", "TOT of all hits with deex cut (30,50) ns",
  //     200, 25000.0, 55000.0));
  // getStatistics().getHisto1D("Deex_TOT_cut")->GetXaxis()->SetTitle("TOT [ps]");
  // getStatistics().getHisto1D("Deex_TOT_cut")->GetYaxis()->SetTitle("Number of Hits");
}