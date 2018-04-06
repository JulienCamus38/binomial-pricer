/* ----------------------------------------------------------------
**
**  PROGRAM:
**
**  PURPOSE:
**
**  USAGE:
**
** ----------------------------------------------------------------
*/

#include "Option.hpp"

ostream& operator<<(ostream& output, const Option& option)
{
    output << "=====================================" << endl;
    output << "Option parameters" << endl;
    output << "=====================================" << endl;
    output << "Number of time steps:\t" << option.N << endl;
    output << "Number of paths:\t" << option.nbPaths << endl;
    output << "Number of runs:\t\t" << option.nbRuns << endl;
    output << "Spot price:\t\t" << option.S0 << endl;
    output << "Strike price:\t\t" << option.K << endl;
    output << "Maturity:\t\t" << option.T << endl;
    output << "Risk free rate:\t\t" << option.R << endl;
    output << "Volatility:\t\t" << option.sigma << endl;
    output << "Option type:\t\tAmerican " << (option.callPricing ? "Call" : "Put") << endl;
    output << "=====================================" << endl;

    return output;
}
