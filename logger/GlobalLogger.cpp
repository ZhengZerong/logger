#include "GlobalLogger.hpp"

#include <vector>
#include <string>
#include <fstream>

SYSTEMTIME getSystemDateTime()
{
    SYSTEMTIME systemDateTime;
    GetLocalTime(&systemDateTime);
    return systemDateTime;
}

GlobalLogger logger(getSystemDateTime(), "");