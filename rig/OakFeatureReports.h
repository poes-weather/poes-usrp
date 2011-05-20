/// \file  
/// \date 2008-10-17
/// \author Xavier Michelon, Toradex SA 
///  
/// \brief Implementation of wrapper functions for generic feature reports of Oak devices


#ifndef OAK__FEATURE_REPORTS__H
#define OAK__FEATURE_REPORTS__H


#include "OakHidBase.h"


namespace Toradex { namespace Oak {


//************************************************************************************************* 
/// \brief Enumeration for the LED mode
//************************************************************************************************* 
enum EOakLedMode
{ 
   eLedModeOff =          0,  ///< LED is Off 
   eLedModeOn =           1,  ///< LED is On
   eLedModeBlinkSlowly =  2,  ///< LED is blinking slowly
   eLedModeBlinkFast =    3,  ///< LED is blinking fast
   eLedModeBlink4Pulses = 4   ///< LED is in 4 pulses blink mode
};


//************************************************************************************************* 
/// \brief Enumeration for the report mode
//************************************************************************************************* 
enum EOakReportMode
{
   eReportModeAfterSampling = 0,     ///< Interrupt reports are sent after sampling
   eReportModeAfterChange   = 1,     ///< Interrupt reports are sent after change
   eReportModeFixedRate     = 2,     ///< Interrupt reports are sent at a fixed rate
};




EOakStatus setReportMode(int deviceHandle, EOakReportMode reportMode, bool persistent);                  ///< Set the device report mode
EOakStatus getReportMode(int deviceHandle, EOakReportMode& outReportMode, bool persistent);              ///< Retrieve the device report mode  
EOakStatus setLedMode(int deviceHandle, EOakLedMode ledMode, bool persistent);                           ///< Set the device LED blinking mode
EOakStatus getLedMode(int deviceHandle, EOakLedMode& outLedMode, bool persistent);                       ///< Get the device LED blinking mode
EOakStatus setReportRate(int deviceHandle, unsigned int reportRate, bool persistent);                    ///< Set the device report rate (period at which the device send interrupt reports to the host)
EOakStatus getReportRate(int deviceHandle, unsigned int& outReportRate, bool persistent);                ///< Get the device report rate (period at which the device send interrupt reports to the host)
EOakStatus setSampleRate(int deviceHandle, unsigned int sampleRate, bool persistent);                    ///< Set the device sample rate
EOakStatus getSampleRate(int deviceHandle, unsigned int& outSampleRate, bool persistent);                ///< Get the device sample rate
EOakStatus setUserDeviceName(int deviceHandle, std::string const& deviceUserName, bool persistent);      ///< Set the user changeable device name
EOakStatus getUserDeviceName(int deviceHandle, std::string& outDeviceUserName, bool persistent);         ///< Get the user changeable device name
EOakStatus setUserChannelName(int deviceHandle, int channel,std::string const& userChannelName,              
                        bool persistent);                                                                ///< Set the user changeable name for a channel of the device
EOakStatus getUserChannelName(int deviceHandle, int channel, std::string& outUserChannelName,
                        bool persistent);                                                                ///< Get the user changeable name for a channel of the device



} } // namespace Toradex::Oak


#endif // #ifndef OAK__FEATURE_REPORTS__H
