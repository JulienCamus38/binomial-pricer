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
#include "Pricer.hpp"
#include "err_code.h"

#include <fstream>

// ----------------------------------------------------------------
//  Functions from ./CXX_common
// ----------------------------------------------------------------
extern double wtime();   /* returns time since some fixed past point
                            (wtime.c) */

#define MAX_NBTIMESTEPS 51200

int main()
{
    try
    {
        // Initialization
        Option option;
        cout << option;

        util::Timer timer;
        double start, end;

        int multiplyRate = 2;
        //int addRate = 4000;

        Pricer* seqPricer = new SequentialPricer();
        Pricer* oclPricer = new OpenCLPricer();

        double seqPrice, oclPrice;

        // File used to plot values
        ofstream plots;
        plots.open("out.txt");
        plots << "N,Sequential,OpenCL" << endl;

        // Information
        cout << "============================================" << endl;
        cout << "OPTION PRICING" << endl;
        cout << "--------------------------------------------" << endl;
        if (dynamic_cast<OpenCLPricer*>(oclPricer)->getPricingMethod() == Method::GROUPS)
        {
            cout << "[INFO] Computing price using groups" << endl;
        }
        else {
            cout << "[INFO] Computing price using triangles" << endl;
        }
        cout << "============================================" << endl;

        // Computation
        while (option.N <= MAX_NBTIMESTEPS)
        {
            // Reset the timer
            timer.reset();

            cout << "[INFO] Number of time steps:\t " << option.N << endl;
            plots << option.N << ",";

            // Sequential pricing
            start = static_cast<double>(timer.getTimeMicroseconds());
            seqPrice = seqPricer->computePrice(option);
            end = static_cast<double>(timer.getTimeMicroseconds());
            cout << "[Sequential] Price:\t\t " << seqPrice << endl;
            cout << "[Sequential] Duration:\t\t " << (end - start)/1000.0 << " ms" << endl;
            plots << (end - start)/1000.0 << ",";

            // OpenCL pricing
            start = static_cast<double>(timer.getTimeMicroseconds());
            oclPrice = oclPricer->computePrice(option);
            end = static_cast<double>(timer.getTimeMicroseconds());
            cout << "[OpenCL] Price:\t\t\t " << oclPrice << endl;
            cout << "[OpenCL] Duration:\t\t " << (end - start)/1000.0 << " ms" << endl;
            cout << "--------------------------------------------" << endl;
            plots << (end - start)/1000.0 << endl;

            // Update number of time steps
            option.N *= multiplyRate;
            //option.N += addRate;
        }
        cout << "============================================" << endl;
    }
    catch (cl::Error e)
    {
        cerr << "[ERROR] " << e.what() << " (" << e.err() << ": "
             << err_code(e.err()) << ")" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
