/* Copyright 2009-2011 Michael Sechooler
 *
 * This file is part of URT.
 * 
 * URT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * URT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with URT.  If not, see <http://www.gnu.org/licenses/>.
 */

//This file doesn't actually contain any globals (yet). Instead, it is used to document files, namespaces,
//and other features for Doxygen that don't belong in any one specific file.

/**@namespace urt
 * All classes that are a part of the UCLA Robotics Toolkit are inside the urt namespace.
 */

/**@namespace urt::contrib
 * Library code that is not technically a part of URT but rather closely associated with it.
 *
 * Code found in here was not added to URT due to the fact that it was too
 * specific -- URT's goal is to provide a common framework for high-level
 * robotics projects -- yet useful and generic enough to be used across multiple
 * projects.
 */


/**@mainpage
 * The UCLA Robotics Toolkit (URT) is designed to provide a level of abstraction from low-level operating system
 * calls and reduce the need to write boilerplate code. It is written primarily for clarity, ease of use,
 * and generic applications. Its primary purpose is to handle much of the burden of communicating with several
 * devices over serial and TCP links.
 *
 * @section toc Table of Contents
 * <ul>
 * <li>\ref glossary
 * <li>\ref first_glance
 * <li>\ref misc
 * <li>\ref compiling
 * </ul>
 */
 
/** @page glossary Glossary of Terms
 * <dl>
 * <dt>\ref urt::State "State" <dd> A "static" class; a table of substates identified by their keys. Only one can exist per
 *				program.
 * <dt>\b Substate	<dd> A key/value pair representing some status of the program. (E.g., "sonar"/"12 ft." pair 
 *				representing sonar sensory data.)
 * <dt>\b Signal	<dd> An object that, when triggered, calls a set of attached slots.
 * <dt>\b Slot		<dd> A function, whether static member, regular member, global, or functor, attached to a signal.
 * <dt>\ref urt::StateDevice "StateDevice" <dd> An object representing an embedded device connected over a serial port that 
 *				speaks only in terms of substates.
 * <dt>\ref urt::StateSocket "StateSocket"	<dd> An object representing a TCP connection of similar nature to the 
 *				StateDevice.
 * <dt>\ref urt::DeviceManager "DeviceManager"	<dd> An object that searches for physical StateDevices per given rules 
 *				(e.g., /dev/ttyUSB*) and instantiates an objects to represent them.
 * <dt>\ref urt::EventLoop "EventLoop"	<dd> An object that watches StateDevices, StateSockets, and other I/O objects for 
 *				activity and services them accordingly. Generally, only one exists per program.
 * <dt>\b App \b ID	<dd> An unsigned char common for all StateDevices of the same type. When StateDevices access a
 *				substate, their App ID and UID are automatically prepended. See UID.
 * <dt>\b UID		<dd> Unit identifier; an unsigned char that enables distinguishing StateDevices of the same type.
 *				Ideally, would be unique for each StateDevice of the same app id, but generally is null.
 *				When StateDevices access a substate, their app ID and UID are automatically prepended
 *				to the substate key; e.g., a sonar device of app id '0' and uid '\\0' attempting to set
 *				substate "sonar" would actually set "0\0sonar".				
 * </dl>
 */
 
/** @page first_glance First Glance
 * @section first_glance_section First Glance
 * Using only urt::StateSocket and urt::StateDevice classes for communication with other computers and embedded
 * serial devices makes the URT system quite simple and elegant; systems that require functionality above and beyond
 * these two classes cannot be implemented entirely as described in this section.
 *
 * @subsection state_system The State System
 * All persistent data related to sensors, actuators, remote control clients, and other attached devices is stored
 * in urt::State, a "static" class which essentially holds key/value pairs in a map. For example, sensory data from
 * a sonar sensor would be held in the key/value pair (often referred to as a "substate" in this documentation)
 * with the key "1\0sonar". Conforming to urt::StateDevice protocol (which defines serial communication with attached
 * embedded devices), the key encodes an embedded device type, unit identifier, and description.
 *
 * @subsection receiving_state Receiving State Data
 * When an attached device has information to send, it sends it in a key/value pair, which is committed to the State class.
 * The key is determined by the embedded device; when it has information to be stored in
 * the State class, it simply transmits a command to modify the substate directly.
 *
 * @subsection sending_state Sending State Data
 * There are two ways to send state data. Remote devices attached via a StateSocket or StateDevice object can request
 * a substate, which will be looked up in the State class and forwarded. Additionally, substates can be sent unsolicitedly.
 * Normally, the only reason to choose the latter is when dealing with time-sensitive information or when working with
 * embedded devices. However, this choice is not automatic and requires further intervention by the programmer, which will
 * be addressed later.
 *
 * @subsection accessing_state Accessing State Data
 * Any code which would wish to access sensory or other data, therefore, simply would have to query the State class
 * using the urt::State::get() or urt::State::set() functions. URT is a static class, meaning all member functions are
 * static; there is no way to instantiate a State object. All code shares the same State, which exists throughout execution.
 *
 * Note that changing a substate does not necessarily mean that attached devices will know of the change. Likewise,
 * accessing a substate does not necessarily mean that the information is fresh.
 *
 * @subsection integrating_other_code Interacting with StateDevices
 * So far, we've discussed the ways in which the system automatically commits incoming data into the global State
 * and ways in which external devices can request data from the global State. However, we have no mentioned ways
 * to direct external objects or unsolicitedly send data.
 *
 * The StateDevice class provides urt::StateDevice::poll() and urt::StateDevice::sendSubstate() to poll embedded devices
 * (requesting that they set all substates they can) and relay substates, respectively. We will ignore the use of the
 * first function, since urt::DeviceManager (which automatically finds and registers attached devices) handles this for you.
 *
 * The latter function, however, is important. For example, let's say that a remote computer, via a StateSocket, modifies
 * the substate with key "0\0motor_velocity". In our case, per the urt::StateDevice documentation, that means a substate
 * associated with a device of app type '0' and uid '\\0', which describes our motor controller. One way for the motor 
 * controller to know about the change is to constantly request the substate. However, the more efficient way is to call 
 * urt::StateDevice::sendSubstate() and send the information unsolicitedly. We can do that using a substate's signal, as
 * described in the next subsection.
 *
 * @subsection other_code Integrating Other Code
 * There are generally two similar ways to enable other functions to be called appropriately
 * to add such functionality to your program.
 *
 * Functions that should be called upon a change in a particular substate can be added to that substate's own signal
 * via urt::State::registerSlot(). One particular use of this functionality is to connect
 * the urt::StateDevice::sendSubstate() of our motor controller to the substate holding the speed value of a robot. In this 
 * manner, the motor controller will be updated whenever the value is changed.
 *
 * Other functions that should be called on a regular interval can be added to an EventLoop's interval signal via
 * urt::EventLoop::registerIntervalSlot(). Your program 
 * will generally have one EventLoop, which services incoming events from sockets, serial ports, etc. The interval signal
 * (a misnomer, since it is called regardless of whether the system is in fact interval), is called every timeout period, which
 * can be specified in the EventLoop's constructor. Any slot (a function, whether standalone or class member) registered
 * with the signal will be called and can then manipulate the State or attached devices. 
 *
 * More on the signal/slot system is available below, in the original documentation of URT that follows.
 *
 * @section important_components Important Components You Just Have to Know
 * Writing a simple URT-based program requires the use of the following classes: EventLoop, DeviceManager, and State. <ul>
 * <li>The urt::EventLoop is the object the watches that various different connections to other devices and services them.</li>
 * <li>The urt::DeviceManger (perhaps better called the StateDeviceManager) searches for attached StateDevices, creates StateDevice
 * objects to represent them, and associates them with an EventLoop.</li><li>The urt::State "static" class (static in that all member
 * functions are static; there is no way to instantiate an actual State object) is the map that holds all information
 * returned by devices as key/values pairs (referred to as substates herein).</li></ul>
 *
 * Most URT-based programs will create an EventLoop and a DeviceManager, giving it the rules to use to find StateDevice
 * device files (for example, all files starting with "ttyUSB" in the /dev directory). After calling
 * urt::DeviceManager::execute() to actually execute the rules, the program would call urt::EventLoop::run() to start handling I/O
 * events.
 *
 * @section three_ways_stuff_happens Three Ways Stuff Happens
 * <ul>
 * <li>Events on I/O connections cause the corresponding objects' onActivity() function to be called. Provided you do
 * 	not create new communications classes, you will not need to implement your own such functions.
 * <li>Functions attached to an urt::EventLoop interval signal will be called periodically every timeout duration.
 *	The urt::DeviceManager will automatically hookup StateDevice::poll() to the EventLoop's signal.
 * <li>Functions attached to a substate's signal will be called upon a change in that substate's value.
 *	Only the urt::Watchdog class will automatically configure a substate's signal. Generally, this means that
 *	on-change substate transmissions will need to be configured by the programmer, generally at the time the StateDevice
 *	is created in urt::DeviceManager::onFound() by extending the class and implementing your own.
 * </ul>
 */
 
/**@page misc Miscellaneous Information 
 * @section nutshell URT in a Nutshell
 * URT is primarily built around a polymorphic event-based paradigm. It is event based in the sense that
 * most the process spends most of its time waiting for activity on file descriptors and other event
 * sources. It is polymorphic because when an event occurs, the event system calls a pure virtual function --
 * urt::FDEvtSource::onActivity() -- which is implemented by client subclasses.
 *
 * @section contrib The Contrib Namespace/Directory
 * The urt::contrib namespace and contrib directory contains code that is not a part of URT proper but nevertheless
 * has a general use. More information is available in the namespace's documentation.
 *
 * @section event_system Event System
 * urt::EventLoop objects are responsible for watching for events on attached event source objects and dispatching
 * them to the object's onActivity() function. Presently, all event source objects are associated with a particular
 * file descriptor and, as such, are children of the abstract base urt::FDEvtSource. The EventLoop also provides an interval event
 * signal that calls associated slots (see below for more information on the signal-slot system) on a regular interval given on instantiation.
 * Generally, one event loop exists per process, especially since URT is not thread safe.
 *
 * @section smart_ptr Smart Pointers
 * URT utilizes the Boost smart_ptr library to provide automatic deletion of event sources. The two types
 * of smart pointer utilized by URT is the boost::shared_ptr and the boost::weak_ptr. URT provides the respective
 * aliases LockedEvtSourcePtr and EvtSourcePtr for FDEvtSource pointers.
 *
 * When a shared_ptr is created for an object,
 * it takes ownership of that object. It also keeps track of all copies made of itself. When it and all its copies dies,
 * the shared_ptr will delete the associated object. In this way, an EventLoop will manage any associated FDEvtSource. It
 * holds shared_ptrs to every FDEvtSource. When an onActivity() returns false, indicating that it should be deleted, the EventLoop
 * will delete its shared_ptr. If no other shared_ptr copies exist to the object, it will be deleted.
 *
 * If you want outside code to have a reference to an FDEvtSource but still permit it to be immediately deleted when
 * the EventLoop deletes its shared_ptr, the weak_ptr will enable you to do this. A weak_ptr has no bearing on when
 * an object is deleted. However, it also cannot be used to directly access the object; first, it must be converted to a shared_ptr by
 * calling the lock() function (hence the alias for shared_ptr being LockedEvtSourcePtr). (The lock() function will return a null pointer if
 * the object has already been deleted.) As long as the new shared_ptr exists, the object will not be deleted. As such, it is suggested that
 * you ensure the shared_ptr goes out of scope as soon as possible, thereby unlocking the object and permitting it to be deleted.
 *
 * @attention As long as an FDEvtSource object is associated with an EventLoop, no outside code should hold a regular pointer to it. Instead,
 * 	you should use either boost::shared_ptr (LockedEvtSourcePtr) or boost::weak_ptr (EvtSourcePtr).
 *
 * @section signal_slot Signal-Slot System
 * While all file descriptor events are dispatched polymorphically, other URT systems use the Boost signals library to provide a signal-slot system.
 * A signal is a particular object that can have a variable number of slots attached to it. When the signal is fired, all attached slots will be called.
 * A slot can be a normal function, a static function, or even a member function of a particular object. The interval event system of an EventLoop is managed
 * this way. There is an interval event signal to which slots can be attached using urt::EventLoop::registerIntervalSlot(). Whenever an interval interval has passed,
 * all attached slots will be called.
 *
 * @section state Global State
 * The urt::State class is a special class; all its members are wholly static. This is formally known as a monostate. There is no way to instantiate a State
 * object; there is always one State and it exists throughout execution. The State object is essentially a hash map between std::string keys and
 * std::string values; a pair is referred to as a substate in this documentation.
 *
 * All the substates relevant to the robot are stored here. For example, a sonar sensor would update substate "sonar" with "12 ft," which would later
 * be retrieved by a client program over a TCP socket. All communication between classes essentially should happen through the State system. For
 * more flexibility, each substate has a signal. Whenever the substate changes (or is merely touched -- set to the same value -- if configured this way),
 * the signal will be fired. In this manner, it is possible for objects to observe certain substates to enable immediate action upon specific changes.
 * For example, an object in charge of drive-train control may wish to be notified immediately upon change of the substate "Xvelocity." The interface
 * to add slots to a substate's signal is similar to EventLoop's interval signal.
 *
 * The urt::StateDevice class provides a standard way for devices to get and set substates over a serial link. It is encapsulated in the ARD protocol
 * (provided by base class urt::ArdPort), which defines a standard way to transmit and receive datagrams over a serial link. The urt::StateSocket class
 * provides a similar mechanism over a TCP socket.
 *
 * @section threading Thread Safety
 * URT is \b not thread safe. urt::ArdPort and urt::StateSocket both utilize static buffers, urt::State is wholly static, and the use of signals
 * further complicates the matter. Instead, URT provides the urt::ExternalProgram class, which enables client code to execute another process and
 * interact with its stdin, stdout, and optionally stderr. It permits this by connecting the three to a socket, which can then be interfaced with
 * using a normal socket class, like urt::StateSocket. Essentially, URT permits multiprocessing without permitting multi-threading.
 *
 * @section posix_signal POSIX Signal Safety
 * URT is \b not POSIX signal safe. URT was designed under the assumption that if a system call returns in an unexpected
 * manner (for example, a read call returns with fewer than expected bytes of data), an error of some sort occurred; it does
 * not check for interruption by POSIX signal as a possibility. As a result, signals will generated spurious URT exceptions.
 * POSIX signal compatibility is possible in the future.
 *
 * @section cpp0x C++0x Support
 * While URT does not mandate the use of C++0x, if it is compiled with a complier that supports C++0x, certain new features will be activated.
 * Currently, the only C++0x specific feature is a type-safe form of the varadic urt::ExternalProgram::Create(), but more features may be added
 * that will benefit from C++0x support. Presently, only G++'s C++0x is supported, but more compliers may be added soon.
 *
 * @section guideline General Guideline
 * Generally, you should take the following steps to create your own URT-based robotic control server: <ol>
 * <li>Ensure that your devices and clients utilize the StateDevice or StateSocket protocol whenever possible.</li>
 * <li>Create an EventLoop object.</li>
 * <li>Create FDEvtSource-derived objects for all your devices and clients and add them to the EventLoop. This can be done
 * automatically with urt::DeviceManager for StateDevices. </li>
 * <li>Add necessary slots to the EventLoop interval signal. For example, connect the poll() function of each StateDevice, 
 * causing the system to poll all StateDevices on a regular basis (urt::DeviceManger does this automatically).</li>
 * <li>Add necessary slots to substate signals. For example, connect the setVelocity() function of your own MotorController to the "xvelocity"
 * 		and "yvelocity" keys. (Note: the MotorController class does not actually exist.)</li>
 * <li>Run the urt::EventLoop::run() function.</li>
 * </ol>
 */

/**@page compiling Compiling and Linking
  * Presently, URT is not provided as a static or dynamic library. When the API/ABI stablizes, static libraries may be
  * released; due to the intracies of using a C++ dynamic library (primarily the lack of typeinfo, which most impacts
  * the use of exceptions), URT will probably never be released as a dynamic library.
  *
  * Therefore, using URT consists of compiling the source code along with your program code and linking the resulting 
  * object code as normal. Note that the following libraries must be linked with your program as well:
  * 	\li rt
  *	\li boost_signals
  *	\li boost_filesystem
  *	\li boost_system
  *	\li boost_regex
  *	\li pthread
  *
  * All of the libraries listed after boost_signals can be ignored if your program does not use or link with
  * urt::DeviceManager or urt::HotDeviceManager.
  *
  * @note Some operating system installations may use slightly different library names. For example, on our testing
  * computers, all boost libraries are postfixed with "-mt".
  *
  * @note With g++, pthread library support is linked by using "-pthread", unlike the rest, which use normal linking
  * syntax (e.g., "-lrt").
  */
