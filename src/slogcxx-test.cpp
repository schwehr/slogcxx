//////////////////////////////////////////////////////////////////////
//
/// \file
/// \brief Testing routines for slogcxx (slog-icks)
///
/// Copyright (c) 2006 Kurt Schwehr
///     Data Visualization Research Lab,
/// 	Center for Coastal and Ocean Mapping
///	University of New Hampshire.
///	http://ccom.unh.edu
///
/// \todo convert asserts to tests that call FAILED_HERE and return false
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <cstdlib>

#include <slogcxx.h>

// For testing do NOT add "using namespace std;"!!  By not doing this, we can tell better what is going on

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
#define UNUSED
#endif // ifdef __GNUC__
#endif // ifndef UNUSED



/*!
  \brief Use \a 'FAILED_HERE;' to emit a string that looks like a compiler warning
  Use this to allow emacs to jump to this source line with either emacs command: C-x ` or F2
*/
/*
#define FAILED_HERE std::cerr <<  __FILE__ << ":" << __LINE__		\
			 << " failed in function " << __FUNCTION__ << std::endl;
*/
//#define FAILED_HERE std::cerr << "Hello\n";
#define FAILED_HERE std::cerr << __FILE__ << ":" << __LINE__ << ": error: failed in function " << __FUNCTION__<< std::endl;

//////////////////////////////////////////////////////////////////////
// Test functions
//////////////////////////////////////////////////////////////////////

/// test writing to a file without and with xml
bool testFile() {
  // Logging to a file
  {
    Slog l("foo.log"," ",false,false);
    l.entry(ALWAYS,"Hello World");
    l.enableXml();
    l.entry(ALWAYS,"Hello World in XML");
  }
  return true;
}

/// Try out writing to a file in xml mode with one LogState
bool testFileXml() {
  {
    Slog l("fooXml.log"," ",true,true);
    l.entry(ALWAYS,"Hello World in XML");
    {
      LogState ls(&l,"lvl_1");
      l.entry(ALWAYS,"Should be at lvl one");
    }
  }
  return true;
}

/// Try out a couple scopes
bool testHeavyScope() {
  Slog l("foo-testScoping.log");
  //l.setStateIndent("\t");
  LogState ls1(&l,"one");
  {
    l << "inside 1" << endl;
    LogState ls2(&l,"two");
    {
      l << "inside 2" << endl;
      LogState ls3(&l,"three");
      {
	l << "inside 3" << endl;
      } // 3
    } // 2
  } // 1
  return true;
}

/// Try out many scopes without xml.  Should still indent
bool testHeavyScopeNoXml() {
  Slog l("foo-testScopingNoXml.log"," ",false,false,true);
  //l.setStateIndent("\t");
  LogState ls1(&l,"one");
  {
    l << "inside 1" << endl;
    LogState ls2(&l,"two");
    {
      l << "inside 2" << endl;
      LogState ls3(&l,"three");
      {
	l << "inside 3" << endl;
      } // 3
    } // 2
  } // 1
  return true;
}


/// Make a ton of scopes and leave them without closing.  Generates a runtime warning.
bool testScope() {
  Slog l("scopeTests.log");
  l.pushState("one");
  l.pushState("2");
  l.pushState("3");
  l.pushState("four");
  l.pushState("5");
  l.pushState("6");
  l.pushState("seven");
  l.writeState();
  l.writeState(false);
  // Unbalanced pops... this generates a warning
  return true;  
}

/// Go crazy with scoping and use incl/decl and incMsg;
bool testScopeWithMsgLvl() {
  Slog l("scopeTestsWithMsgLvl.log");

  l.setLevel(TERSE);

  l.setMsgLevel(TRACE);
  l << "No" << endl;
  assert(TRACE == l.getMsgLevel());

    l.pushState("1",TERSE);
    l << "Yes" << endl;
    assert(TERSE == l.getMsgLevel());
    l.popState();
  l << "No" << endl;
  assert(TRACE == l.getMsgLevel());

  l.incMsg();
  assert(VERBOSE == l.getMsgLevel());

    l.pushState("1",BOMBASTIC);
    l << "No" << endl;
    assert(BOMBASTIC == l.getMsgLevel());

      l.pushState("2");
      l << "No" << endl;
      assert(BOMBASTIC == l.getMsgLevel());
      l << decl << "No" << decl << "No" << endl;
      assert(TRACE == l.getMsgLevel());
      l.popState();
    l << "No" << endl;
    assert(TRACE == l.getMsgLevel());

      l.pushState("2",TERSE);
      l << "Yes" << endl;
      assert(TERSE == l.getMsgLevel());
      l.popState();
    l << "No" << endl;
    assert(TRACE == l.getMsgLevel());

    l.popState();
    l << "No" << endl;
  assert(VERBOSE == l.getMsgLevel());
  return true;
}


/// One large test that does logs of stuff with dec/inc, etc
bool testBig() {
  // FIX: change asserts to tests!!!

  Slog log("foo2.log"," ",false,false,false);

  log << 1;
  log << endl;
  log << "a string";
  log << endl;
  log << 2 << " " << 3 << endl;

  log.setLevel(1);
  if (1!=log.getLevel()) {FAILED_HERE; return false;}

  log.dec();
  assert(0==log.getLevel());
  log.dec();
  assert(0==log.getLevel());

  log.setLevel(999);
  assert(999==log.getLevel());
  log.dec();
  assert(998==log.getLevel());
  log.inc();
  assert(999==log.getLevel());
  

  log.setLevel(TRACE);
  assert(log.entry(TRACE,"trace"));
  assert(!log.entry(VERBOSE,"verbose")); // Not seen
  log.inc();
  assert(log.entry(VERBOSE,"verbose after log")); // Seen

  assert(log.partial(TRACE,"tracePartial"));
  assert(log.complete());

  log.dec();
  assert(log.partial(TRACE,"a "));
  assert(!log.partial(VERBOSE,"b "));
  assert(log.partial(TRACE,"c "));
  assert(log.complete());


  log.setLevel(VERBOSE);
  log.setMsgLevel(TRACE);
  log << "Should see this"<<endl;
  log.setMsgLevel(BOMBASTIC);
  log << "Should NOT see this"<<endl;

  //
  // THIS IS THE TRUE WAY TO USE IT!!!
  //

  log.setLevel(TRACE);
  log.setMsgLevel(TRACE);
  log << "Yes " << incl << "No " << decl << "Yes!" << endl;

  log.setLevel(VERBOSE);
  log << "Yes " << incl << "YES " << decl << "Yes!" << endl;

  {
    assert(0==log.getStateDepth());
    LogState logstate1(&log,"one");
    assert(1==log.getStateDepth());
    {
      LogState logstate2(&log,"two");
      assert(2==log.getStateDepth());
      log.writeState();
      log << 2 << endl;
      // Try out an early explicit pop
      LogState logstate3(&log,"three");
      assert(3==log.getStateDepth());
      log << 3 << endl;
      log.writeState();
      logstate3.pop(); // Here is the pop
      assert(2==log.getStateDepth());
    }
    assert(1==log.getStateDepth());
  }
  assert(0==log.getStateDepth());
  return true;
}

/// This is about as simple a test as can be made
bool testSimple() {
  Slog l("fooSimple.log");
  l << "Hello World" << endl;
  return true;
}

/// Test out all the types available.
bool testTypes() {
  Slog l("types.log");
  l << "int: " << int(1) << endl;
  char *cstr="c style string";
  l << cstr << endl;
  l << std::string("C++ STL string") << endl;
  l << "char: " << 'c' << endl;
  l << "short: " << short(2) << endl;
  l << "long: " << long(3) << endl;
  l << "float: " << float(4.1) << endl;
  l << "double: " << double(5.2) << endl;
  return true;
}

/// Simple test to see what happens
class whereClassTest {
public:
  /// Some method that does logging
  void doWhere(Slog &s) { s << "Call where in a class method " << WHERE << endl;}
};

/// Try out the where and WHERE calls
bool testWhere() {
  {
    Slog l("test-where-noxml.log","\t",false,false);
    l.where("a file",123456,"some function");
    l.complete();

    l.where(__FILE__,__LINE__,__FUNCTION__);
    l.partial(l.getMsgLevel()," test of the __FILE__ etc macros");
    l.complete();
  }
  {
    Slog l("test-where-xml.log","\t");
    l.where("a file",123456,"some function");
    l.complete();

    l.where(__FILE__,__LINE__,__FUNCTION__);
    l.partial(l.getMsgLevel()," test of the __FILE__ etc macros");
    l.complete();

    // This is really how it should be used
    l << "This has an embedded where "<< WHERE <<" xml tag" << endl;
    l << "Probably better practice to put the WHERE at the end " << WHERE << endl;

    whereClassTest wct;
    wct.doWhere(l);
  }
  return true;
}

bool testPointer() {
    Slog *l = new Slog("test-pointer.log");
    assert(l);
    *l << "string" << endl;
    *l << 6 << endl;
    *l << 9.99 << endl;
    delete l;
    return true;
}

//////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////

/// This is bone head unit testing
int main(UNUSED int argc, UNUSED char *argv[]) {
  bool ok = true;

  //EXIT_DEBUG("because I am too lazy to go any farther");
  // FAILED_HERE; // Uncomment to see what an error looks like


  // This will not be pretty since the loggers send stuff to cout
  if (!testSimple())	 	{FAILED_HERE; ok=false; std::cout << "testSimple ... ERROR\n";} 	else std::cout << "testSimple ... ok\n";
  if (!testWhere())	 	{FAILED_HERE; ok=false; std::cout << "testWhere ... ERROR\n";} 	else std::cout << "testWhere ... ok\n";
  if (!testTypes())	 	{FAILED_HERE; ok=false; std::cout << "testTypes ... ERROR\n";} 		else std::cout << "testTypes ... ok\n";
  if (!testHeavyScope()) 	{FAILED_HERE; ok=false; std::cout << "testHeavyScope ... ERROR\n";} 	else std::cout << "testHeaveScope ... ok\n";
  if (!testHeavyScopeNoXml()) 	{FAILED_HERE; ok=false; std::cout << "testHeavyScopeNoXML ... ERROR\n";} else std::cout << "testHeaveScopeNoXML ... ok\n";
  if (!testFileXml()) 		{FAILED_HERE; ok=false; std::cout << "testFileXml ... ERROR\n";}	else std::cout << "testHeavyXml ... ok\n";
  if (!testFile()) 		{FAILED_HERE; ok=false; std::cout << "testFile ... ERROR\n";}	 	else std::cout << "testFile ... ok\n";
  if (!testBig()) 		{FAILED_HERE; ok=false; std::cout << "testBig ... ERROR\n";}	 	else std::cout << "testBig ... ok\n";

  if (!testScopeWithMsgLvl()) {FAILED_HERE; ok=false; std::cout << "testScopeWithMsgLvl ... ERROR\n";}	 	else std::cout << "testScopeWithMsgLvl ... ok\n";

  if (!testPointer())           {FAILED_HERE; ok=false; std::cout << "testPointer ... ERROR\n";}	else std::cout << "testPointer ... ok\n";

  // std::cout << "early"<< endl;exit(EXIT_FAILURE); // Use this line to run a subset of tests

  std::cout << "  " << argv[0] << " test:  " << (ok?"ok":"failed")<<std::endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
