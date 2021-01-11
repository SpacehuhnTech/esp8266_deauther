#ifndef StringCLI_h
#define StringCLI_h

#if defined(ARDUINO)
    #include "Arduino.h"           // String
#else
    #include <string>              // std::string
    #include <cstring>             // strlen()
    typedef std::string String;
    inline String F(String s) { return s; };
#endif

#endif // ifndef StringCLI_h