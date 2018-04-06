/* ----------------------------------------------------------------
**
**  Include file for the binomial pricer
**
** ----------------------------------------------------------------
*/

#ifndef __PRICER__
#define __PRICER__

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define __CL_ENABLE_EXCEPTIONS

#include "cl.hpp"
#include "util.hpp"

#include "Option.hpp"

#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <algorithm>

using namespace std;

// Abstract class for option pricing
class Pricer
{
public:
    // Destructor
    virtual ~Pricer() {}

    // Price computing
    virtual double computePrice(Option& option) = 0;
};

// Child class for option pricing using sequential algorithm
class SequentialPricer: public Pricer
{
public:
    // Price computing
    virtual double computePrice(Option& option);
};

// Child class for option pricing using OpenCL algorithm
class OpenCLPricer: public Pricer
{
public:
    // Constructor
    OpenCLPricer();

    // Destructor
    virtual ~OpenCLPricer();

    // Price computing
    virtual double computePrice(Option& option);

private:
    vector<cl::Platform>* platforms;
    cl::Platform* defaultPlatform;
    vector<cl::Device>* devices;
    cl::Device* defaultDevice;
    cl::Context* context;
    string* kernelCode;
    cl::Program::Sources* sources;
    cl::Program* program;

    double computePriceUsingTriangles(Option& option);
    double computePriceUsingGroups(Option& option);
};

#endif
