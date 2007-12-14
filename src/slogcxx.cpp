//////////////////////////////////////////////////////////////////////
/// \file
/// \brief Logging class to write messages to console and file
///
/// Copyright (c) 2006 Kurt Schwehr
///     Data Visualization Research Lab,
/// 	Center for Coastal and Ocean Mapping
///	University of New Hampshire.
///	http://ccom.unh.edu
///
/// \bug FIX: make namespaces work
/// \todo get tv_usec into the time.
/// \todo Optionally for XML mode, 
///       look for the closing tag of the previous log and clip it so the logs blend.
///
/// Documentation for each method is in the header.
///
//////////////////////////////////////////////////////////////////////

// C headers
#include <ctime> // for time() to log the time

// WinDoze stuff
#ifdef WIN32
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif


// C++ headers
#include <sstream> // stringstream to convert values into strings
#include <iostream> // cerr

// Local headers
#include "slogcxx.h"

using namespace std;  // this must be after slogcxx.h to make sure we use std correctly
//using namespace slogcxx;  // FIX: would be nice to have namespaces


Where::Where(const std::string &_file, const int _lineno, const std::string &_function)
  : file(_file), lineno(_lineno), function(_function)
{
  // Nothin
}


//////////////////////////////////////////////////////////////////////
// Slog class methods
//////////////////////////////////////////////////////////////////////


Slog::Slog(const std::string &filename, const std::string &indentStr,
	   const bool append, const bool enableXml, const bool enableTime)
  : logLevel(1), msgLevel(1), curStr(""),
    xmlEnabled(enableXml), timeEnabled(enableTime)//, stateIndent(" ")//("\t")
	  ,stateIndent(indentStr)
{
  if (0<filename.size()) {
    if (1<=logLevel) cerr << "Opening log file: '" << filename << "'" << endl;
    if (append) logFile.open(filename.c_str(),ios::out | ios::app);
    else {
      logFile.open(filename.c_str(),ios::out); // Overwrite the old file
      logFile.setf(ios::fixed, ios::floatfield);
     }
    assert (logFile.is_open());
    if (xmlEnabled) logFile << "<slogcxx>"<<endl;
  }
  entry(ALWAYS,"started logging");
}


Slog::~Slog() {
  if (0<stateStack.size()) {
    cerr << "WARNING: shutting down the logger with open scopes.\n" 
	 << "  I hope you know what you are doing" << endl;
    const int depth=stateStack.size();
    for (int i=0;i<depth;i++) popState();
  }
  if (!curStr.empty()) {
    cerr << "WARNING: shutting down with uncompleted partial log message!\n  FORCING COMPLETE\n";
    complete();
  }
#ifndef WIN32  //ifdef'd out by mdp 12/14/2007 because using strstream during shutdown causes errors on WIN32
  entry(ALWAYS,"stopped logging");
#endif
  if (logFile.is_open()) {
      if (xmlEnabled) logFile << "</slogcxx>"<<endl;
      logFile.flush(); // Be extra sure that everything is written out.
      logFile.close();
  }

}

/// Allow the definition of time to be tweaked.  Floats should be enough for now
#define TIME_T float //double
//#define TIME_T int

bool
Slog::entry(const int lvl, const std::string str) {
  if (lvl>logLevel) return false; // Not powerful enough to get out
  //cerr << getStateNumberStr() << " " << getCurScope() << ":" << indent << str << endl;
  cerr << getStateNumberStr() << indent() << getCurScope() << ": " << str << endl;
  if (logFile.is_open()) {
#ifdef WIN32
    timeb timebuffer;
    ftime(&timebuffer);
    const TIME_T currentSysTime = timebuffer.time+(timebuffer.millitm/1000.0);
#else
    timeval timebuffer;
    gettimeofday(&timebuffer,NULL);
    const TIME_T currentSysTime = timebuffer.tv_sec+(timebuffer.tv_usec/1000000.0);
#endif
    if (xmlEnabled) {
      logFile << indent() << "<entry";
      //logFile.setf(ios_base::fixed,ios::floatfield);
      if (timeEnabled) logFile << " time=\""<< currentSysTime << "\"";
      if (!stateStack.empty()) {
	logFile << " scope=\"" << stateStack[stateStack.size()-1] << "\"";
      }
      logFile << ">" << str << "</entry>" << endl;;
    } else {
      // NO XML
      logFile << indent();
      if (timeEnabled) logFile << currentSysTime << " ";
      if (!stateStack.empty()) logFile << stateStack[stateStack.size()-1] << ": ";
      logFile << str << endl;
    }
  }
  return true;
}

// See also operator<< on a Where class object
// FIX: consider removing this from the interface.
bool Slog::where(const std::string &file, const int lineno, const std::string &function) {
  stringstream sstr;
  if (xmlEnabled) {
    sstr << "<where file=\""<<file<<"\" line=\""<<lineno<<"\" function=\""<<function+"\"/>";
  } else {
    // NO XML
    sstr << "("<<file<<":"<<lineno<<":"<<function+")";
  }
  partial(getMsgLevel(),sstr.str());
  return true;
}

bool
Slog::partial(const int lvl, const std::string str) {
  if (lvl>logLevel) return false; // Not powerful enough to get out
  curStr += str;
  return true;
}

bool
Slog::complete() {
  if (0==curStr.length()) return false; // Nothing to log, so ignore the request
  entry(ALWAYS,curStr); // We got this far so for a message to go out.
  curStr="";
  return true;
}

////////////////////////////////////////
// State

std::string 
Slog::indent() {
  std::string s;
  const int depth=getStateDepth();
  for (int i=0;i<depth;i++) s+=stateIndent;
  return s;
}

std::string 
Slog::getStateNumberStr() {
  stringstream sstr;
  sstr << getStateDepth();
  std::string s = sstr.str();
  if (1==s.size()) s = " "+s;
  return s;
}

// FIX: implement with xml goodness... now it just does scopes in straight text.
void
Slog::writeState(bool flat) {
  if (flat) {
    std::vector<std::string>::iterator itor;
    for(itor = stateStack.begin(); itor!=stateStack.end(); itor++) {
      cerr << "." << *itor;
      if (logFile.is_open()) logFile << "." << *itor;
    }	
    cerr << endl;
    if (logFile.is_open()) logFile << endl;
  } else {
    // Not flat
    const int depth = stateStack.size();
    for (int i=0; i<depth; i++) {
      for (int z=0;z<i;z++) {
	cerr << stateIndent;
	if (logFile.is_open()) logFile << stateIndent;
      }
      cerr << stateStack[i] << endl;
      if (logFile.is_open()) logFile << stateStack[i] << endl;
    }	
    //cerr << endl;
    
  }
}

void 
Slog::pushState(std::string scope, int msgLvl) {
  if (xmlEnabled) logFile << indent() << "<scope name=\""<< scope <<"\">" << endl;
  stateStack.push_back(scope);
  if (msgLvl != -1)
  {
    msgLvlStack.push_back(getMsgLevel());
    setMsgLevel(msgLvl);
  }
  else
  {
    msgLvlStack.push_back(-1);
  }
}

std::string
Slog::popState() {
  assert(!stateStack.empty()); // FIX: is it right to fail?
  std::string s=stateStack[stateStack.size()-1];
  int ml = msgLvlStack[msgLvlStack.size()-1];
  if (ml != -1)
    setMsgLevel(ml);
  stateStack.pop_back();
  msgLvlStack.pop_back();
  if (xmlEnabled) logFile << indent() << "</scope> <!-- "<< s <<" -->" << endl;
  return s;
}




//////////////////////////////////////////////////////////////////////
// IO Manipulators that control the ``stream''
//////////////////////////////////////////////////////////////////////

// FIX: inline these for speed!
Slog& operator<<(Slog&s, Slog&(*manip)(Slog&)) {
  return manip(s);
}

Slog& endl(Slog& s) {
  s.complete();
  return s;
}

Slog& incl(Slog& s) {
  s.incMsg();
  return s;
}

Slog& decl(Slog& s) {
  s.decMsg();
  return s;
}



//////////////////////////////////////////////////////////////////////
// Handlers for each type that can be logged.
//////////////////////////////////////////////////////////////////////

// stringstream is probably not the fastest way to do this
// FIX: This should be templated!!  Or can I if there are lots of special cases?

Slog& operator<< (Slog &s, const int &r) {
  int lvl = s.getMsgLevel();
  stringstream sstr;
  sstr << r;
  s.partial(lvl,sstr.str());
  return s;
}

Slog& operator<< (Slog &s, const size_t &r) {
  int lvl = s.getMsgLevel();
  stringstream sstr;
  sstr << r;
  s.partial(lvl,sstr.str());
  return s;
}


Slog& operator<< (Slog &s, const char &c) {
  int lvl = s.getMsgLevel();
  stringstream sstr;
  sstr << c;
  s.partial(lvl,sstr.str());
  return s;
}

Slog& operator<< (Slog &s, const short &sh) {
  int lvl = s.getMsgLevel();
  stringstream sstr;
  sstr << sh;
  s.partial(lvl,sstr.str());
  return s;
}

Slog& operator<< (Slog &s, const unsigned short &ush) {
  int lvl = s.getMsgLevel();
  stringstream sstr;
  sstr << ush;
  s.partial(lvl,sstr.str());
  return s;
}

Slog& operator<< (Slog &s, const long &l) {
  int lvl = s.getMsgLevel();
  stringstream sstr;
  sstr << l;
  s.partial(lvl,sstr.str());
  return s;
}

Slog& operator<< (Slog &s, const float &f) {
  int lvl = s.getMsgLevel();
  stringstream sstr;
  sstr << f;
  s.partial(lvl,sstr.str());
  return s;
}

Slog& operator<< (Slog &s, const double &d) {
  int lvl = s.getMsgLevel();
  stringstream sstr;
  sstr << d;
  s.partial(lvl,sstr.str());
  return s;
}

Slog& operator<< (Slog &s, const char *str) {
  int lvl = s.getMsgLevel();
  s.partial(lvl,string(str));
  return s;
}

Slog& operator<< (Slog &s, const std::string &str) {
  int lvl = s.getMsgLevel();
  s.partial(lvl,str);
  return s;
}

// FIX: add more basic types here such as vector, deque, list, and ???

//////////////////////////////////////////////////////////////////////
// More complicated insertion operators
//////////////////////////////////////////////////////////////////////


Slog& operator<< (Slog &s, const Where &w) {
    stringstream sstr;
    if (s.getXmlStatus()) {
	sstr << "<where file=\""<<w.getFile()<<"\" line=\""<<w.getLineno()<<"\" function=\""<<w.getFunction()+"\"/>";
    } else {
	// NO XML
	sstr << "("<<w.getFile()<<":"<<w.getLineno()<<":"<<w.getFunction()+")";
    }
  s.partial(s.getMsgLevel(),sstr.str());
  return s;
}


//////////////////////////////////////////////////////////////////////
// LogState
//////////////////////////////////////////////////////////////////////

LogState::LogState(Slog *logInstance, const std::string &scope, int msgLvl) 
  : log(logInstance), popped(false)
{
  assert(logInstance);
  #ifndef NDEBUG
  	const int count = log->getStateDepth();
  #endif
  log->pushState(scope,msgLvl);
  assert(count+1==log->getStateDepth());
}

std::string
LogState::pop() {
  if (popped) return "";
  popped=true;
  return log->popState();
}

LogState::~LogState() {
  if (!popped) log->popState();
}
