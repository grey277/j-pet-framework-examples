# ImageReconstruction parameters description

## Contents

Description of available parameters in ImageReconstruction example.

- `FilterEvents_Cut_On_Z_Value_float`  
  Events with Z value greater then this value will not be included in output file. [cm]

- `FilterEvents_Cut_On_LOR_Distance_From_Center_float`
  Events with LOR distance greater then this value will not be included in output file [cm]

- `FilterEvents_TOT_Min_Value_In_Ns_float`
  Events with sum of TOT with value less then this value will not be included in output file [ns]

- `FilterEvents_TOT_Max_Value_In_Ns_float`
  Events with sum of TOT with value greater then this value will not be included in output file [ns]

- `FilterEvents_Angle_Delta_Min_Value_float`
  Events that hits have angle differences value less then this value will not be included in output file [degrees]

- `MLEMRunner_OutFileName_std::string`
  Path to file where will be saved converted data for futher reconstruction in j-pet-mlem [string]

- `MLEMRunner_NumberOfPixelsInOneDimension_int`
  Number of pixels in reconstructed image in one demension [px]

- `MLEMRunner_PixelSize_double`
  Size of 1 pixel in reconstructed image [m]

- `MLEMRunner_StartPixelForPartialMatrix_int`
  Start pixel of partial matrix.

- `MLEMRunner_NumberOfEmissionsPerPixel_int`
  Number of emmisions per pixel for system matrix.

- `MLEMRunner_TOFStepSize_double`
  TOF step size [m]

- `MLEMRunner_TOFSigmaAxis_float`
  TOF sigma [m]

- `MLEMRunner_TOFSigmaAlongZAxis_float`
  TOF sigma along Z axis [m]

- `MLEMRunner_DisplayInfo_bool`
  If true display extra info about reconstruction into cerr

- `MLEMRunner_SystemMatrixOutputPath_std::string`
  Path to file where system matrix will be saved and from what file read system matrix if already generated

- `MLEMRunner_SystemMatrixSaveFull_bool`
  If true generate full system matrix, not faster 1/8 of the system matrix and using symetries reconstruct full matrix

- `MLEMRunner_ReconstructionOutputPath_std::string`
  Path to file where reconstruction will be saved

- `MLEMRunner_ReconstuctionIterations_int`
  Number of MLEM iterations

- `ImageReco_Annihilation_Point_Z_float`
  Maximum value of Z of reconstructed annihilation point to be included in reconstruction

- `ImageReco_Xrange_On_3D_Histogram_int`
  Range of X axis on 3D Histogram

- `ImageReco_Yrange_On_3D_Histogram_int`
  Range of Y axis on 3D Histogram

- `ImageReco_Zrange_On_3D_Histogram_int`
  Range of Z axis on 3D Histogram

- `ImageReco_Bin_Multiplier_double`
  Used to decrease size of bin, if bin multiplier is 1: 1 bin correspondes to 1 cm.

- `SinogramCreator_OutFileName_std::string`
  Path where sinogram root file will be saved.

- `SinogramCreator_ReconstructionDistanceAccuracy_float`
  In cm, setting to 1 means 1 px represents 1cm.

- `SinogramCreator_MaxReconstructionLayerRadius_float`
  In cm, useful to increase reconstruction time, by reducing radius of reconstructed image.

- `SinogramCreator_SinogramZSplitNumber_int`
  Number of splits on Z axis.

- `SinogramCreator_ScintillatorLength_float`
  Length of the scintillators, by default set to 50cm.

- `SinogramCreator_EnableObliqueLORRemapping_bool`
  Enables remapping oblique LORs to same Z position based on TOF.

- `SinogramCreator_EnableNEMAAttenuation_bool`
  Enables attenuation correction for NEMA phantom. It should only be enabled, if input data contains NEMA phantom.

- `SinogramCreator_TOFBinSliceSize_float`
  Size of TOF bin used for TOF-FBP.

- `SinogramCreator_GojaInputFilesPaths_std::vector<std::string>`
  Only set if input data should be readed from Goja format, left blank for reading data from framework.

- `ReconstructionTask_ReconstructSliceNumbers_std::vector<int>`
  Set to reconstruct only selected slices on Z, useful to increase reconstruction time.

- `ReconstructionTask_FilterCutOffValueBegin_float`
- `ReconstructionTask_FilterCutOffValueEnd_float`
- `ReconstructionTask_FilterCutOffValueStep_float`
  Options allowing to set cutoff value for filter. It will reconstruct and save image for all cutoff values between ``begin`` and ``end`` by ``step`` values.

- `ReconstructionTask_FilterName_std::string`
  Filter to use, allowed values: ``None``, ``Cosine``, ``Hamming``, ``Hann``, ``Ridgelet``, ``SheppLogan``. By default ``None`` is used.

- `ReconstructionTask_FilterAlphaValue_float`
  Some filters have extra ``alpha`` parameter. If filter is using this parameter, it can be set here.

- `ReconstructionTask_ReconstructionType_std::string`
  Reconstruction type, allowed values: ``FBP``, ``TOFFBP``.

- `ReconstructionTask_LORTOFSigma_float`
  TOF sigma on LOR.

- `ReconstructionTask_OutFileName_std::string`
  Where reconstructed images should be saved.
  
