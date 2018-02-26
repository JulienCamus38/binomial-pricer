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

#include "binomial_pricer.hpp"
#include "util.hpp"
#include <err_code.h>
#include "device_picker.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    // Simulation parameters
    int nbTimeSteps = 100;
    int nbPaths     = 32;
    int nbRuns      = 1;

    // Option parameters
    double S0    = 100;
    double K     = 10;
    double T     = 3.0;
    double r     = 0.05;
    double sigma = 0.2;

    // Bool do we price a put or a call
    bool callPricing = true;

    // Do we want debug info
    #ifdef WITH_CPU_REFERENCE
    bool withDebugInfo = false;
    #endif

    // Read command-line options
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-timesteps"))
            nbTimeSteps = strtol(argv[++i], NULL, 10);
        else if (!strcmp(argv[i], "-paths"))
            nbPaths = strtol(argv[++i], NULL, 10);
        else if (!strcmp(argv[i], "-runs"))
            nbRuns = strtol(argv[++i], NULL, 10);
        else if (!strcmp(argv[i], "-S0"))
            S0 = strtod(argv[++i], NULL);
        else if (!strcmp(argv[i], "-K"))
            K = strtod(argv[++i], NULL);
        else if (!strcmp(argv[i], "-T"))
            T = strtod(argv[++i], NULL);
        else if (!strcmp(argv[i], "-r"))
            r = strtod(argv[++i], NULL);
        else if (!strcmp(argv[i], "-sigma"))
            sigma = strtod(argv[++i], NULL);
        else if (!strcmp(argv[i], "-put"))
            callPricing = false;
        #ifdef WITH_CPU_REFERENCE
        else if (!strcmp(argv[i], "-debug-info"))
            withDebugInfo = true;
        #endif
        else
        {
            fprintf(stderr, "Unknown option %s. Aborting!\n", argv[i]);
            exit(1);
        }
    }

    // Print the arguments
    printf("==============\n");
    printf("NbTimesteps  : %d\n",  nbTimeSteps);
    printf("NbPaths      : %dK\n", nbPaths);
    printf("NbRuns       : %d\n",  nbRuns);
    printf("S0           : %lf\n", S0);
    printf("K            : %lf\n", K);
    printf("T            : %lf\n", T);
    printf("r            : %lf\n", r);
    printf("sigma        : %lf\n", sigma);
    printf("Option type  : American %s\n", callPricing ? "Call" : "Put");
    printf("==============\n");

    // Discretization step
    double dt = T / nbTimeSteps;

    // TODO

    return EXIT_SUCCESS;
}
