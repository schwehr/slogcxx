// -*- c++ -*- Tell emacs this is c++
//////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 Kurt Schwehr
//      Data Visualization Research Lab,
// 	Center for Coastal and Ocean Mapping
//	University of New Hampshire.
//	http://ccom.unh.edu
//
/// @file
/// @brief Stripped down version that tries to dissapear at compile time
///
//////////////////////////////////////////////////////////////////////


// WARNING: do not include this file directly... Define NLOG on slogcxx.h instead.

//////////////////////////////////////////////////////////////////////
// MACROS 
//////////////////////////////////////////////////////////////////////

// FIX: Put a NOP in the stream?
//#define WHERE Where(__FILE__,__LINE__,__FUNCTION__)
// Patterned after assert.h.  Will this work?
#undef WHERE
//#define WHERE ((void)0)
#define WHERE (0)

//////////////////////////////////////////////////////////////////////
// Where class
//////////////////////////////////////////////////////////////////////

// class Where {
//  public: 
//     Where(UNUSED const std::string &_file, UNUSED const int &_lineno, UNUSED const std::string &_function) {};
//     inline std::string getFile() const {return std::string("unknown file");};
//     inline int getLineno() const {return -1;};
//     inline std::string getFunction() const {return std::string("unknown function");};
// };

class Where {
 public: 
    Where(UNUSED const std::string &_file, UNUSED const int &_lineno, UNUSED const std::string &_function) {}
    inline std::string const &getFile() const {
	static const std::string unknownFile("unknown file");
	return unknownFile;
    }
    inline int const &getLineno() const {
	static const int unknownLine=-1;
	return unknownLine;
    }
    inline std::string const &getFunction() const {
	static const std::string unknownFunction("unknown function");
	return unknownFunction;
    }
};



//////////////////////////////////////////////////////////////////////
// The main Slog class
//////////////////////////////////////////////////////////////////////

// FIX: probably needs help optimizing this down
class Slog {
public:
    Slog(UNUSED const std::string &filename="", const std::string &indentStr=" ", 
	 UNUSED const bool append=true, const bool enableXml=true, const bool enableTime=true)
	: logLevel(1), msgLevel(1), curStr(""),
	    xmlEnabled(enableXml), timeEnabled(enableTime)
	    ,stateIndent(indentStr)
    {}
    ~Slog() {}

    void setLevel(const int lvl) {logLevel=lvl;} 
    int getLevel() {return logLevel;}
    int inc() {return ++logLevel;}
    int dec() {--logLevel; if (0>logLevel) logLevel=0; return logLevel;}
    void enableTime() {timeEnabled=true;};
    void disableTime() {timeEnabled=false;};
    bool getTimeStatus() {return timeEnabled;};
    void enableXml() {xmlEnabled=true;}; 
    void disableXml() {xmlEnabled=false;};  
    bool getXmlStatus() {return xmlEnabled;};
    bool entry(UNUSED const int lvl, UNUSED const std::string str) {return true;}

    bool where(UNUSED const std::string &file, UNUSED const int lineno, UNUSED const std::string &function) {return true;}

    void setMsgLevel(const int lvl) {assert(0<=lvl);msgLevel=lvl;}
    int getMsgLevel() {return msgLevel;}
    int incMsg() {return ++msgLevel;}
    int decMsg() {--msgLevel; if (0>msgLevel) msgLevel=0; return msgLevel;}

    bool partial(UNUSED const int lvl, UNUSED const std::string str) {return true;} 
    bool complete() {return true;}

    void setStateIndent(const std::string &str) {stateIndent=str;};
    std::string getStateIndent() {return stateIndent;};
    std::string indent();
    std::string getStateNumberStr();
    std::string getCurScope() {if (0==getStateDepth()) return ""; return stateStack[stateStack.size()-1];}

    void pushState(std::string scope, int msgLvl = -1) {
	stateStack.push_back(scope);
	if (msgLvl != -1) {
	    msgLvlStack.push_back(getMsgLevel());
	    setMsgLevel(msgLvl);
	}  else { msgLvlStack.push_back(-1); }
    }
    std::string popState() {
	std::string s=stateStack[stateStack.size()-1];
	int ml = msgLvlStack[msgLvlStack.size()-1];
	if (ml != -1) setMsgLevel(ml);
	stateStack.pop_back();
	msgLvlStack.pop_back();
	return s;
    }
    void writeState(UNUSED bool flat=true) {}
    int getStateDepth() {return stateStack.size();};

    Slog& operator=(UNUSED const Slog& rhs) {
	std::cerr << "Slog op=!" << std::endl;
	return *this;
    }

private:
    int logLevel;
    int msgLevel;
    std::string curStr;
    bool xmlEnabled;
    bool timeEnabled;

    std::string stateIndent;
    std::vector<std::string> stateStack;
    std::vector<int> msgLvlStack;
}; // end Slog class


inline Slog& operator<<(Slog&s, UNUSED Slog&(*manip)(Slog&)) {return s;}

inline Slog& endl(Slog& s) {return s;}
inline Slog& decl(Slog& s) {return s;}
inline Slog& incl(Slog& s) {return s;}

inline Slog& operator<< (Slog &s, UNUSED const int &r){return s;} 
inline Slog& operator<< (Slog &s, UNUSED const char *str){return s;}
inline Slog& operator<< (Slog &s, UNUSED const std::string &str){return s;}

inline Slog& operator<< (Slog &s, UNUSED const char &c){return s;} 
inline Slog& operator<< (Slog &s, UNUSED const short &sh){return s;}
inline Slog& operator<< (Slog &s, UNUSED const long &l){return s;} 
inline Slog& operator<< (Slog &s, UNUSED const float &f){return s;} 
inline Slog& operator<< (Slog &s, UNUSED const double &d){return s;}

inline Slog& operator<< (Slog &s, UNUSED const Where &w){return s;}


class LogState {
public:
    LogState(Slog *logInstance, const std::string &scope, int msgLvl = -1): log(logInstance), popped(false) {log->pushState(scope,msgLvl);}
    inline std::string pop() {  if (popped) return "";  popped=true; return log->popState();}
    ~LogState() {}
private:
    Slog *log;
    bool popped;
};
