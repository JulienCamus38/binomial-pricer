/* ----------------------------------------------------------------
**
**  Include file for the binomial pricer
**
** ----------------------------------------------------------------
*/

#ifndef __BIN_PRICER
#define __BIN_PRICER

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS   /* clCreateCommandQueue
                                               is deprecated: avoids
                                               warning */
#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

// ----------------------------------------------------------------
//  Functions from ./common
// ----------------------------------------------------------------
extern double wtime();   /* returns time since some fixed past point
                            (wtime.c) */

// ----------------------------------------------------------------
//  Constants
// ----------------------------------------------------------------
#define SUCCESS  1
#define FAILURE  0

#endif
