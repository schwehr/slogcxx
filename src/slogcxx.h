//////////////////////////////////////////////////////////////////////
/// Copyright (c) 2006 Kurt Schwehr
///     Data Visualization Research Lab,
/// 	Center for Coastal and Ocean Mapping
///	University of New Hampshire.
///	http://ccom.unh.edu
///
/// @file
/// @brief Logging class to write messages to console and file
///
/// @todo FIX: make namespaces work
/// @todo FIX: put macros that do __FILE__ and __LINE__ additions
//////////////////////////////////////////////////////////////////////

#ifndef SLOGCXX_H // Include guard
#define SLOGCXX_H

// C headers
#include <cassert>
#include <climits>

// C++ headers
#include <string>
#include <vector>

#include <iostream>
#include <fstream>

// If we're going to try to do this with Boost concurrency mechanisms to make it thread safe, boost headers
#ifdef CONCURRENT_BOOST
#include "boost/thread.hpp"
#endif

//////////////////////////////////////////////////////////////////////
// MACROS 
//////////////////////////////////////////////////////////////////////


// This avoids warning of unused arguments when prefixed with USUSED
#ifndef UNUSED
#ifdef __GNUC__
#define UNUSED __attribute((__unused__))
#else
/*!
 \brief GNU CC attribute to denote unused paramters in function calls.
 The attribute remove compiler warning for unused arguments and variable.  Only works
 for GNU compilers such as gcc and g++.
 
 http://gcc.gnu.org/onlinedocs/gcc-3.4.2/gcc/Variable-Attributes.html
 */
// FIX: Can we or do we need something like this for Win32/VC++?
#define UNUSED
#endif
#endif // ifndef UNUSED

/// Simple macro to terminate execution early while debugging
#define EXIT_DEBUG(why) std::cerr << "EXIT_DEBUG called at " \
<< __FILE__ << ":" << __LINE__<<": in function '" <<__FUNCTION__ << "'\n" \
<< "  STATED REASON: " << why << std::endl;				\
exit(EXIT_FAILURE);

/*! \brief Manipulator like item that writes out the location of the log entry.
 \code
 Slog l;
 l << WHERE << "Example of recording where in the code we are" << endl;
 \endcode
 */
#if !defined (NLOG)
#define WHERE Where(__FILE__,__LINE__,__FUNCTION__)
#endif

//////////////////////////////////////////////////////////////////////
// Where class
//////////////////////////////////////////////////////////////////////

/// \brief A class to allow the WHERE macro to work
///
/// Pretty darn simple object.  Should it have a method to generate xml and non-xml versions?

#ifndef NLOG
class Where {
public: 
    /// Basic constructor that takes __FILE__,__LINE__ and __FUNCTION__
    Where(const std::string &_file, const int _lineno, const std::string &_function);
	Where(void) : lineno(0) {}
    inline std::string const &getFile() const {return file;};
    inline int const &getLineno() const {return lineno;};
    inline std::string const &getFunction() const {return function;};
	bool operator!=(const Where& other) const
	{
		return file != other.file || lineno != other.lineno || function != other.function;
	}
private:
    std::string file; 	///< The results of __FILE__
    int lineno;		///< The results of __LINE__
    std::string function;	///< The results of __FUNCTION__
};
#endif // NLOG

//////////////////////////////////////////////////////////////////////
// ENUMS
//////////////////////////////////////////////////////////////////////

// FIX: these may need to be const int values.

/// @brief Logging levels.
///
/// Think of them from the perspective of the runtime requested
/// verbosity.  If the user sets their logging level to TERSE, then
/// they will receive messages with message levels for ALWAYS and TERSE.
///
/// When writing log messages in the program, choose the log level or
/// higher that the message will appear at.  For things that will
/// generate massive spewage, select the bombastic level.  Log messages
/// that use the NEVER tag will not show up (no matter what).  ALWAYS
/// log messages can not be turned off (if logging is enabled.
enum LogLevelsEnum { 
	// Kurt's names
	ALWAYS=INT_MIN, // Only use for entries()
	LACONIC = 0,
	TERSE,
	TRACE,
	VERBOSE,
	BOMBASTIC,
	NEVER = INT_MAX // Only use for entries()
};

/// @brief Traditional syslog(3)-like error levels, for manipulators
#define SDEBUG		BOMBASTIC << "debug: "
#define SINFO		VERBOSE << "info: "
#define SNOTICE		TRACE << "notice: "
#define SWARNING	TERSE << "warning: "
#define SERROR		LACONIC << "error: "

//////////////////////////////////////////////////////////////////////
// The main slog class
//////////////////////////////////////////////////////////////////////


/*!
 \brief simple logging class with a ostream like (<<) interface
 
 Here is the simplest possible example of using slogcxx:
 \code
 #include <slogcxx.h>
 
 int
 main(int argc, char *argv[]) {
 Slog log;
 log << "argc " << argc << endl;
 log << "argv[0] " << argv[0] << endl;
 return (EXIT_SUCCESS);
 }
 \endcode
 
 When compiled and run, this code logs only to stderr like this:
 \verbatim
 0: started logging
 0: argc 1
 0: argv[0] ./simplest
 0: stopped logging
 \endverbatim
 
 Here is a slightly more complicated example that logs to a file and uses a few more of the features.
 \code
 int
 main(int argc, char *argv[]) {
 Slog log("sample.log");
 log << "argc " << argc << endl;
 log << "argv[0] " << argv[0] << endl;
 log << "The WHERE object marks a location in the code " << WHERE << endl;
 {
 LogState ls1(&log,"scope name here");
 log << "LogState will pop a log scope when it is destroyed" << endl;
 {
 LogState ls2(&log,"two");
 log << "Here is another log scope" << endl;
 }
 }
 log << "Not all of a log message will show up" << incl <<"VANISHING"<<decl << endl;
 log << "No reason not to add your own XML <mytag>some info</mytag>" << endl;
 return (EXIT_SUCCESS);
 }
 \endcode
 Running this generates this output to stderr:
 \verbatim
 g++ foo.C slogcxx.o -I. && ./a.out
 Opening log file: 'sample.log'
 0: started logging
 0: argc 1
 0: argv[0] ./a.out
 0: The WHERE object marks a location in the code <where file="foo.C" line="8" function="main" />
 1 scope name here: LogState will pop a log scope when it is destroyed
 2  two: Here is another log scope
 0: Not all of a log message will show up
 0: No reason not to add your own XML <mytag>some info</mytag>
 0: stopped logging
 \endverbatim
 And xml is written to the log file.  Note that there is currently a bug with writing the time.  Scientific notation is not helpful.
 \verbatim
 <slogcxx>
 <entry time="1.15092e+09">started logging</entry>
 <entry time="1.15092e+09">argc 1</entry>
 <entry time="1.15092e+09">argv[0] ./a.out</entry>
 <entry time="1.15092e+09">The WHERE object marks a location in the code <where file="foo.C" line="8" function="main" /></entry>
 <scope name="scope name here">
 <entry time="1.15092e+09" scope="scope name here">LogState will pop a log scope when it is destroyed</entry>
 <scope name="two">
 <entry time="1.15092e+09" scope="two">Here is another log scope</entry>
 </scope> <!-- two -->
 </scope> <!-- scope name here -->
 <entry time="1.15092e+09">Not all of a log message will show up</entry>
 <entry time="1.15092e+09">No reason not to add your own XML <mytag>some info</mytag></entry>
 <entry time="1.15092e+09">stopped logging</entry>
 </slogcxx>
 \endverbatim
 
 
 \todo get fractions of a second into the time entry
 \todo get people other than Kurt to write a bit of documentation.
 */

#if !defined(NLOG)
class Slog {
public:
	/// \brief simple console constructor using cerr
	///
	/// @param filename Also log to a file
	/// @param append Set to false to wipe out previous loggin with the same file name
	/// @param enableXml set false to write plain text to log file.  Console logging not affected.
	/// @param enableTime set to false to stop writing time to the log file entries
	/// @param indentStr Whatever string to indent by for each scope (e.g. " ", "\t" or "...")
	Slog(const std::string &filename="", const std::string &indentStr=" ", 
		 const bool append=true, const bool enableXml=true, const bool enableTime=true,
		 const bool enableLocation=true);
	/// We must not have a mine shaft gap!
	~Slog();
	
	/// @name Verbosity
	//@{
	/// This controls the amount of output
	void setLevel(const int lvl)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		assert(0<=lvl);logLevel=lvl;
	} 
	/// What is the current verbosity level?  Higher means more spewage
	int getLevel(void)
	{
		return logLevel;
	}
	/// Ask for more pain (err... log messages)
	int inc(void)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		return ++logLevel;
	}
	/// Stick your head in the sand (ostrich mode)... see fewer log messages
	int dec(void)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		--logLevel;
		if (0>logLevel) logLevel=0;
		return logLevel;
	}
	//@}
	
	/// @name Time control
	//@{
	/// turn on time stamping in log entries
	void enableTime(void)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		timeEnabled=true;
	}
	/// turn off time stamping in log entries
	void disableTime(void)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		timeEnabled=false;
	}
	/// return true if time logging is on
	bool getTimeStatus(void)
	{
		return timeEnabled;
	}
	//@}
	
	/// @name XML control - only applies to log files.
	///
	/// Generally you will want to just leave XML logging on.  You can
	/// get unbalanced begin and end log tags if the state is different
	/// at the log contruction/destruction.
	//@{
	/// Switch to xml encoding of log messages
	void enableXml(void)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		xmlEnabled=true;
	}
	/// Switch back to text mode
	void disableXml(void)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		xmlEnabled=false;
	}
	/// Are we in xml output mode?
	bool getXmlStatus(void)
	{
		return xmlEnabled;
	}
	//@}
	/// @name Location control - applies to log and console files.
	///
	/// Location information can be provided to the logger explicitly, or through a WHERE state in the
	/// stream output.  If the output of location is enabled, this information preceeds the log message and
	/// time, or is added as a tag in the XML before the text of the entry.  The code cannot, of course, verify
	/// that the location is correct, and only reports what it's given.
	///@{
	/// Switch to enable location information in log messages
	void enableLocation(void)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		locationEnabled = true;
	}
	/// Switch to disable location information in log messages
	void disableLocation(void)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		locationEnabled = false;
	}
	/// Inspector to provide state information
	bool getLocationStatus(void)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		return(locationEnabled);
	}
	///@}
	
	/// do one complete log. return true if logged.  This is the more traditionalC like interface
	/// return false if there was some trouble
	bool entry(const int lvl, const std::string str); 
	
	/// \brief Add a tag of where the log is being generated from.  Adds to the partial log message.
	///
	/// Use the WHERE macro rather than this call directly.  Will write out an xml tag if in xml mode.
	/// @param file filename string for what file the call is currently in
	/// @param lineno The current line number within the file
	/// @param function The current function that execution is occuring in.
	bool where(const std::string &file, const int lineno, const std::string &function);
	
	/// @name Controlling log messages in the stream style (<<)
	//@{
	/// This sets the level of the entry. 
	// The following messages are for controlling what the << log messages do
	void setMsgLevel(const int lvl)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		assert(0<=lvl);
		msgLevel=lvl;
	}
	/// Return the current log level number
	int getMsgLevel(void)
	{
		return msgLevel;
	}
	/// Increase the log level for all messages following (less likely to be logged)
	int incMsg(void)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		return ++msgLevel;
	}
	/// Decrease the log level for all messages following (more likely to be logged)
	int decMsg(void)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		--msgLevel;
		if (0>msgLevel) msgLevel=0;
		return msgLevel;
	}
	//@}
	
	/// @name Implementation for stream (<<) handling
	/// These two are not really meant to be used in code... just within the slogcxx class
	//@{
	/// \brief add to the current log (for <<)
	///
	/// FIX: maybe this should be some friend type thing, but me no like friends
	/// @returns true if it added anything to the log message
	bool partial(const int lvl, const std::string str); 
	/// Finish up a log entry after partials
	/// @return False if there was no stored message to write to the log
	bool complete(void);  
	///@}
	
	/// @name State stack handling
	//@{
	/// Change the indenting to a different string
	void setStateIndent(const std::string &str)
	{
#ifdef CONCURRENT_BOOST
		boost::mutex::scoped_lock lock(m_stateMutex);
#endif
		stateIndent=str;
	}
	/// What is the current indent string.
	std::string getStateIndent(void)
	{
		return stateIndent;
	}
	/// Return the string with the proper indenting
	std::string indent(void);
	/// Return a 2+ character scope depth
	std::string getStateNumberStr(void);
	std::string getCurScope(void)
	{
		if (0==getStateDepth()) return "";
		return stateStack[stateStack.size()-1];
	}
	
	/// Put the current scope onto the state stack
	void pushState(std::string scope, int msgLvl = -1);
	/// Back out one level of scope.  FIX: what if no scope to pop?
	std::string popState(void);
	/// @brief Write out the state to the log.  
	/// @param flat If flat is false, then it tries to pretty pring the scopes on more than one line
	void writeState(bool flat=true);
	int getStateDepth(void)
	{
		return stateStack.size();
	}
	//@}
	
	
	
	// P 65 of EC++ 2nd ed wants the const.
	// FIX: is there any point in having an op=?  Should it be just an assert false?
	/// Prevent copying from happening.  How could we handle a copy?
	Slog& operator=(UNUSED const Slog& rhs) {
		std::cerr << "Slog op=!" << std::endl;
		assert ("WTF... do not copy!");
		return *this;
	}
	
	void SetLocation(const Where& w) { curLocation = w; }
	
private:
#ifdef CONCURRENT_BOOST
	boost::thread::id m_currentThread;	///< Boost thread ID for the thread that's current accumulating a msg
	boost::mutex	m_outputMutex;		///< Boost mutex to protect the output stream(s) in this object
	boost::mutex	m_stateMutex;		///< Boost mutex to protect state variables in this object
	boost::mutex	m_accumulatorMutex;	///< Boost mutex to protect accumulating curStr in this object
#endif
	int logLevel; ///< what the programs logging level is.  Turn this higher to get more messags
	int msgLevel; ///< For partial messages, this is their default level
	std::string curStr; ///< building the current message
	bool xmlEnabled; ///< Should the output be to xml?
	bool timeEnabled; ///< If true, then log entries should include a time stamp.
	bool locationEnabled;	///< Flag: true => prefix location (if provided)
	// FIX: how should time stamp formats be controlled?
	
	std::string stateIndent; ///< How much to indent the output for each level.
	std::vector<std::string> stateStack; ///< All of the state names in a stack
	std::vector<int> msgLvlStack; ///< for push and pop state
	
	std::ofstream logFile; ///< If open then also log to a file.
	
	/// \brief Format the current location information, if available
	std::string& format_location(const bool xmlOutput) const;
	Where	curLocation;	///< Current location, if set
}; // end Slog class


Slog& operator<<(Slog&s, Slog&(*manip)(Slog&));		//!< Allow the use of iomanipulators
Slog& operator<<(Slog& s, const LogLevelsEnum e);	//!< Set message log level for this message

Slog& endl(Slog& s); //!< endl terminates a log... similar to std::endl
Slog& decl(Slog& s); //!< make the message MORE likely to show up
Slog& incl(Slog& s); //!< make the message LESS likely to show up

// Logging operators for basic types
Slog& operator<< (Slog &s, const int &r); //!< Allow logging of ints
Slog& operator<< (Slog &s, const unsigned int &r); //!< Allow logging of unsigned ints
Slog& operator<< (Slog &s, const size_t &r); //!< 32/64 bit safe unsigned numbers
Slog& operator<< (Slog &s, const char *str); //!< Allow logging of C strings
Slog& operator<< (Slog &s, const std::string &str); //!< Log a string

// FIX: can this be made to work?
/* Quote from nntp:
 
 "You can't overload operators that involve only built in types. At
 least one operand must be of class/struct/enum type. Pointers are
 considered built in types, even if they point to user-defined
 types. Thus you cannot overload any operator that works exclusively on
 pointers."
 */

//Slog* operator<< (Slog *s, const char *str);
//Slog* foo (Slog *s, const char *str);
//int foo(Slog &s);
//int foo(Slog *s);

Slog& operator<< (Slog &s, const char &c); //!<  Log insertion of a single character
Slog& operator<< (Slog &s, const short &sh); //!< Insert a short integer
Slog& operator<< (Slog &s, const long &l); //!< Insert a long integer
//Slog& operator<< (Slog &s, const long long &ll); //!< Insert a long long character
Slog& operator<< (Slog &s, const float &f); //!< Insert a 4 byte float
Slog& operator<< (Slog &s, const double &d); //!< Insert a 8 byte float

Slog& operator<< (Slog &s, const unsigned short &ush); //!< Insert an unsigned short integer

////// More complicated insertions of non-basic types.
Slog& operator<< (Slog &s, const Where &w); //!< Insert where object


/// @brief Put this sucker on the stack to save your state.
///
/// FIX: write an example of how this is done
class LogState {
public:
	/// @brief Create a log instance with a given scope name and optional msg level.
	///
	/// Do not delete the log until after this state has cleared!
	LogState(Slog *logInstance, const std::string &scope, int msgLvl = -1);
	/// @brief Request pop.  Only pop if !popped
	/// @return the popped scope name
	std::string pop(void); 
	/// Make sure that the scope has been popped
	~LogState();
private:
	Slog *log; //!< Handle for the associated log to allow popping
	bool popped; //!< Cache early pop request
};

#endif // !defined(NLOG)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// MODE TO ALLOW CODE TO STAY WITH MINIMAL COST

#if defined(NLOG)
#  include <slogcxx-nlog.h>
#endif

#endif // SLOGCXX_H
