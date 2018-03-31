/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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
 *  @file main.cpp
 */

#include <JPetManager/JPetManager.h>
#include "TimeWindowCreator.h"
#include "TimeCalibLoader.h"
#include "SignalFinder.h"
#include "SignalTransformer.h"
#include "HitFinder.h"
#include "EventFinder.h"
#include "EventCategorizer.h"
#include "ImageReco.h"

using namespace std;

int main(int argc, const char* argv[])
{

<<<<<<< HEAD
  // Here create all analysis modules to be used:

  manager.registerTask([](){
      return new JPetTaskLoader("hld", "tslot.raw",
  				  new TaskA("Module A: Unp to TSlot Raw",
  					    "Process unpacked HLD file into a tree of JPetTSlot objects"));
    });
  
  manager.registerTask([](){
      return new JPetTaskLoader("tslot.raw", "raw.sig",
  				new TaskB1("Module B1: Make TOT histos and assemble signals",
					   "Assemble signals and create TOT historgrams"));
    });
=======
  JPetManager& manager = JPetManager::getManager();
>>>>>>> 48f63a57b23c66ec13297f4152eb9b97e7c66110

  manager.registerTask<TimeWindowCreator>("TimeWindowCreator");
  manager.registerTask<TimeCalibLoader>("TimeCalibLoader");
  manager.registerTask<SignalFinder>("SignalFinder");
  manager.registerTask<SignalTransformer>("SignalTransformer");
  manager.registerTask<HitFinder>("HitFinder");
  manager.registerTask<EventFinder>("EventFinder");
  manager.registerTask<EventCategorizer>("EventCategorizer");
  manager.registerTask<ImageReco>("ImageReco");

<<<<<<< HEAD
  manager.registerTask([](){
      return new JPetTaskLoader("phys.hit", "phys.hit.means",
				new TaskD("Module D: Make histograms for hits",
					  "Only make timeDiff histos and produce mean timeDiff value for each threshold and slot to be used by the next module"));
    });
  
  manager.registerTask([](){
      return new JPetTaskLoader("phys.hit.means", "phys.hit.coincplots",
  				new TaskE("Module E: Filter hits",
  					  "Pass only hits with time diffrerence close to the peak"));
    });
=======
  manager.useTask("TimeWindowCreator", "hld", "tslot.raw");
  manager.useTask("TimeCalibLoader", "tslot.raw", "tslot.calib");
  manager.useTask("SignalFinder", "tslot.calib", "raw.sig");
  manager.useTask("SignalTransformer", "raw.sig", "phys.sig");
  manager.useTask("HitFinder", "phys.sig", "hits");
  manager.useTask("EventFinder", "hits", "unk.evt");
  manager.useTask("EventCategorizer", "unk.evt", "cat.evt");
  manager.useTask("ImageReco", "unk.evt", "reco");
>>>>>>> 48f63a57b23c66ec13297f4152eb9b97e7c66110

  manager.run(argc, argv);
}
