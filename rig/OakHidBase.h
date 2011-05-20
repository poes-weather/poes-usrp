/// \file  
/// \date 2008-10-14
/// \author Xavier Michelon, Toradex SA 
///  
/// \brief Declaration of Oak HID base function
///
/// These function encapsulate the Linux hiddev calls


#ifndef OAK__HID__BASE__H
#define OAK__HID__BASE__H


#include <string>
#include <vector>


namespace Toradex { namespace Oak {


//************************************************************************************************* 
/// \brief A few integer constants
//************************************************************************************************* 
enum 
{ 
   kFeatureReportSize   = 32,                ///< Size of the feature report in bytes
   kToradexVendorID     = 0x1b67,            ///< USB Vendor ID for Toradex
   kOakMoveProductID    = 0x6                ///< Oak Move sensor product ID
};


//************************************************************************************************* 
/// \brief Enumeration for status code return by oak functions
//************************************************************************************************* 
enum EOakStatus {
   eOakStatusOK  = 0,                        ///< Success
   eOakStatusErrorOpeningFile,               ///< The device file could not be opened
   eOakStatusInvalidDeviceType,              ///< The device is not a Oak sensor
   eOakStatusInternalError,                  ///< Other errors, unlikely to happen
   eOakStatusInvalidStringDescriptorIndex,   ///< The string descriptor index is invalid
   eOakStatusReadError,                      ///< A read error occured
   eOakStatusWriteError,                     ///< A read error occured
};


typedef unsigned char OakFeatureReport[kFeatureReportSize]; ///< Type definition for the Oak sensor feature report


//************************************************************************************************* 
/// \brief Structure used for storing device information
//************************************************************************************************* 
struct DeviceInfo {
   std::string deviceName;                ///< The device hard-coded name
   std::string volatileUserDeviceName;    ///< The volatile (stored in RAM) user settable device name
   std::string persistentUserDeviceName;  ///< The persistent (stored in FLASH) user settable device name
   std::string serialNumber;              ///< The serial number of the device
   short vendorID;                        ///< The vendor ID of the device
   short productID;                       ///< The product ID of the device
   short version;                         ///< The version of the device
   int numberOfChannels;                  ///< The number of channels of the device
};


//************************************************************************************************* 
/// \brief Structure for holding channel information
//*************************************************************************************************
struct ChannelInfo {
   std::string channelName;                            ///< The hard-coded name of the channel
   std::string volatileUserChannelName;                ///< The user settable name of the device stored in RAM
   std::string persistentUserChannelName;              ///< The user settable name of the device stored in FLASH
   bool isSigned;                                      ///< Is the read value signed ?
   int bitSize;                                        ///< The size in bits of the channel
   int unitExponent;                                   ///< The unit exponent (received value must be multiplied by 10e[unitExponent]
   unsigned int unitCode;                              ///< The USB standardized unit code
   std::string unit;                                   ///< The 'fancy' unit name
};


EOakStatus openDevice(std::string const& devicePath, int& outDeviceHandle);          ///< Open a Oak sensor device
EOakStatus closeDevice(int deviceHandle);                                            ///< Close a Oak sensor device
EOakStatus getDeviceInfo(int deviceHandle, DeviceInfo& outDeviceInfo);               ///< Retrieve global information about the device                               
EOakStatus getChannelInfo(int deviceHandle, int index, ChannelInfo& outChannelInfo); ///< Retrieve information about a channel
EOakStatus readInterruptReport(int deviceHandle, std::vector<int>& outReadValues);   ///< Read an interrupt report and put the read values inside the outReadValues vector
EOakStatus sendFeatureReport(int deviceHandle, OakFeatureReport report);             ///< Send a feature report to the specified device
EOakStatus readFeatureReport(int deviceHandle, OakFeatureReport report);             ///< Read a feature report from the device
EOakStatus printFeatureReport(OakFeatureReport report);                              ///< Display a feature report to the standard output
EOakStatus sendReportAndWaitForReply(int deviceHandle, OakFeatureReport report);     ///< Send a report and wait for the reply

EOakStatus putStringInReport(OakFeatureReport report, std::string const& theString); ///< Utility function that insert a string into a report
EOakStatus getStringFromReport(OakFeatureReport report, std::string& outString);     ///< utility function that extract a string from a feature report sent by a Oak device

std::string getStatusString(EOakStatus);                                             ///< Return a string containing a description of the given status code

} } // namespace Toradex::Oak


#endif // #ifndef OAK__HID__BASE__H
