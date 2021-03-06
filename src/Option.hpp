/* ----------------------------------------------------------------
**
**  Option parameters
**
** ----------------------------------------------------------------
*/

#ifndef __OPTION__
#define __OPTION__

#include <iostream>

using namespace std;

struct Option
{
    // Simulation parameters
    int N               = 100;

    // Option parameters
    float S0            = 100.0;
    float K             = 100.0;
    float T             = 3.0;
    float R             = 0.05;
    float sigma         = 0.2;

    // Bool do we price a put or a call
    bool isCall         = false;

    // Bool do we price an American or an European option
    bool isAmerican     = true;

    // Do we want debug info
    bool withDebugInfo  = false;
};

ostream& operator<<(ostream& output, const Option& option);

#endif
