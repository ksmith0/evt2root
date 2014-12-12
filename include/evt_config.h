#ifndef CONFIG_H
#define CONFIG_H

///Indicates the evt files come from a VM-USB crate.
#define VM_USB false
///Indicate the evt file is a ldf file from HRIBF
#define LDF true

//Uncomment the following line to use a manually specified buffer size.
// Most setups use the default buffer size and this line can be ignored.
//#define BUFFER_SIZE 4096 ///< Manually defined buffer size.

///The list of modules to be unpacked.
/**Modules must be listed in the order that they are to be unpacked. 
 * (Be sure to continue the define line with the '\' character.)
 * If multiple modules of the same type are used a line for each
 * module is required. See modules/include for available modules.
 * The predefine is read by cmake to build a library specific to
 * each implementation and created a vector of pointers to the
 * ReadEvent() methods of each module class.
 * 
 * New modules can be added with the following required prototype:
 * \code 
 * ReadEvent(nsclBuffer*,eventData*,bool)
 * \endcode
 * See modules/include for examples.
 */
#define MODULE_LIST(MODULE) \
	MODULE(hribfModule) 
//	MODULE(Caen_General)
//	MODULE(Caen_IO_V977) \
//	MODULE(Mesytec_ADC_MADC32) \
//	MODULE(XIA_Pixie16) 

#endif

