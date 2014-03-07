#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "07";
	static const char MONTH[] = "03";
	static const char YEAR[] = "2014";
	static const char UBUNTU_VERSION_STYLE[] =  "14.03";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 0;
	static const long MINOR  = 4;
	static const long BUILD  = 80;
	static const long REVISION  = 418;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 147;
	#define RC_FILEVERSION 0,4,80,418
	#define RC_FILEVERSION_STRING "0, 4, 80, 418\0"
	static const char FULLVERSION_STRING [] = "0.4.80.418";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 80;
	

}
#endif //VERSION_H
