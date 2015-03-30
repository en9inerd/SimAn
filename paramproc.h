#ifndef PARAMPROC_H
#define PARAMPROC_H

//:(base class) Catches a given parameter from the command line 
class Param            
{               
public:
    enum Type { NOPARAM, BOOL, INT, UNSIGNED, DOUBLE, STRING };
    // NOPARAM means "empty command line"
    // BOOL means "no value: either found in command line or not"
private:
	bool          _b;  // found
	bool          _on; 
	int           _i;
	unsigned      _u;
	double        _d;
	const char *  _s;
	Type          _pt;
	const char *  _key;
public:
	Param(const char * keyy, Type part, int argc, const char * const argv[]);
	Param(Type part, int argc, const char * const argv[]); // for NOPARAM only
	~Param() {};
	bool      found()       const; 
	// for any Param::Type, always need to check before anything else
	bool      on()          const;  
	// for any Param::Type; true if the option was invoked with +
	
	int       getInt()      const;
	unsigned  getUnsigned() const;
	double    getDouble()   const;
	const char* getString() const;
	
/*  operator  double()      const;  // deprecated : use below classes */
/*  operator  char* ()      const;  //              instead of Param */
};

//:Constructed from argc/argv, returns to true 
// if the command line had no parameters 
class NoParams  : private Param
{
public:
	NoParams(int argc, const char * const argv[]):Param(Param::NOPARAM,argc,argv) {}
	bool found()    const { return Param::found(); }
	operator bool() const { return Param::found(); }
	Param::on;      // base class member access adjustment
};

//: Catches a given boolean parameter
class BoolParam : private Param
{
public:
	BoolParam(const char * key, int argc, const char * const argv[]) 
	: Param(key,Param::BOOL,argc,argv) {}
	bool found() const    { return Param::found(); }
	operator bool() const { return Param::found(); }
	Param::on;      // base class member access adjustment
};

//: Catches a given Unsigned parameter
class UnsignedParam : private Param
{
public:
	UnsignedParam(const char * key, int argc, const char *const argv[])
	: Param(key,Param::UNSIGNED,argc,argv) {}
	bool found() const { return Param::found() && getUnsigned()!=unsigned(-1); }
	operator unsigned() const { return getUnsigned();  }
	Param::on;     // base class member access adjustment
};

//: Catches a given integer parameter
class IntParam : private Param
{
public:
	IntParam(const char * key, int argc, const char * const argv[])
	: Param(key,Param::INT,argc,argv) {}
	bool found()   const { return Param::found();   }
	operator int() const { return getInt();  }
	Param::on;      // base class member access adjustment
};

//: Catches a given double parameter
class DoubleParam : private Param
{
public:
	DoubleParam(const char * key, int argc, const char * const argv[])
	: Param(key,Param::DOUBLE,argc,argv) {}
	bool found() const { return Param::found() && getDouble()!=-1.29384756657; }
	operator double() const { return getDouble();  }
	Param::on;      // base class member access adjustment
};

//: Catches a given string parameter
class StringParam : private Param
{
public:
	StringParam(const char * key, int argc, const char * const argv[])
	: Param(key,Param::STRING,argc,argv) {}
	bool found()     const       
		{ return Param::found() && strcmp(getString(),"Uninitialized"); }
	operator const char*() const  { return getString();  }
	Param::on;      // base class member access adjustment
};

#endif