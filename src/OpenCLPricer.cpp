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

#include "Pricer.hpp"

OpenCLPricer::OpenCLPricer()
{
    //----------------------------- PLATFORMS --------------------------------
    // Retrieve platforms
    platforms = new vector<cl::Platform>();
    cl::Platform::get(platforms);

    // Check number of platforms found
    if (platforms->size() == 0)
    {
        cerr << "[ERROR] No platform found." << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "[INFO] " << platforms->size() << " platform(s) found." << endl;
    }

    // Initialize default platform
    defaultPlatform = &(*platforms)[0];
    cout << "[INFO] Using platform: " << defaultPlatform->getInfo<CL_PLATFORM_NAME>() << endl;

    //----------------------------- DEVICES ----------------------------------
    // Retrieve devices
    devices = new vector<cl::Device>();
    defaultPlatform->getDevices(CL_DEVICE_TYPE_ALL, devices);

    // Check number of devices found
    if (devices->size() == 0)
    {
        cerr << "[ERROR] No device found." << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "[INFO] " << devices->size() << " device(s) found." << endl;
    }

    // Initialize default device
    defaultDevice = &(*devices)[0];
    cout << "[INFO] Using device: " << defaultDevice->getInfo<CL_DEVICE_NAME>()
         << " (Max work item size: " << defaultDevice->getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]
         << ", Max work group size: " << defaultDevice->getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()
         << ", Max computing units: " << defaultDevice->getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()
         << ")" << endl;

    //----------------------------- CONTEXT ----------------------------------
    // Create context
    context = new cl::Context({*defaultDevice});

    //--------------------------- OPENCL CODE --------------------------------
    // Initialize kernel.cl code
    ifstream ifs("kernel.cl");
    kernelCode = new string((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));
    sources = new cl::Program::Sources();
    sources->push_back({kernelCode->c_str(), kernelCode->length()});

    // Build kernel.cl program
    program = new cl::Program(*context, *sources);
    if (program->build({*defaultDevice}) != CL_SUCCESS)
    {
        cerr << "[ERROR] Error building: " << program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*defaultDevice) << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "[INFO] Successfully built kernel.cl program" << endl;
    }
}

OpenCLPricer::~OpenCLPricer()
{
    delete platforms;
    delete devices;
    delete context;
    delete kernelCode;
    delete sources;
    delete program;
}

double OpenCLPricer::computePrice(Option& option)
{
    if (method == Method::GROUPS)
    {
        return computePriceUsingGroups(option);
    }

    return computePriceUsingTriangles(option);
}

double OpenCLPricer::computePriceUsingGroups(Option& option)
{
    int groupSize = 20;

    // Compute other parameters
    double dt = option.T / option.N;
    double u = exp(option.sigma * sqrt(dt));
    double d = 1.0 / u;
    double r = exp(option.R * dt);
    double p = (r - d) / (u - d);
    int isCall = (option.isCall) ? 1 : -1; // used in kernel
    int isAmerican = (option.isAmerican) ? 1 : 0; // used in kernel

    // Initialize buffers
    cl::Buffer groupA(*context, CL_MEM_READ_WRITE, sizeof(float) * (option.N + 1));
    cl::Buffer groupB(*context, CL_MEM_READ_WRITE, sizeof(float) * (option.N + 1));

    // Initialize queue
    cl::CommandQueue queue(*context, *defaultDevice);

    // Run init function
    cl::Kernel initKernel(*program, "init");
    initKernel.setArg(0, option.N);
    initKernel.setArg(1, (float)option.S0);
    initKernel.setArg(2, (float)option.K);
    initKernel.setArg(3, (float)dt);
    initKernel.setArg(4, (float)u);
    initKernel.setArg(5, (float)d);
    initKernel.setArg(6, isCall);
    initKernel.setArg(7, groupA);
    queue.enqueueNDRangeKernel(initKernel, cl::NullRange, cl::NDRange(option.N + 1));
    queue.enqueueBarrierWithWaitList();

    // Run group function
    cl::Kernel groupKernel(*program, "group");
    groupKernel.setArg(0, (float)u);
    groupKernel.setArg(1, (float)d);
    groupKernel.setArg(2, (float)r);
    groupKernel.setArg(3, (float)p);
    groupKernel.setArg(4, (float)option.S0);
    groupKernel.setArg(5, (float)option.K);
    groupKernel.setArg(6, isCall);
    groupKernel.setArg(7, isAmerican);
    int nbPoints, nbWorkItems;
    for (int i = 1; i <= option.N; ++i)
    {
        nbPoints = option.N + 1 - i;
        nbWorkItems = ceil((float)nbPoints / groupSize);
        groupKernel.setArg(8, (i % 2 == 1) ? groupA : groupB);
        groupKernel.setArg(9, (i % 2 == 1) ? groupB : groupA);
        groupKernel.setArg(10, nbPoints);
        groupKernel.setArg(11, groupSize);
        queue.enqueueNDRangeKernel(groupKernel, cl::NullRange, cl::NDRange(nbWorkItems));
        queue.enqueueBarrierWithWaitList();
    }

    // Results
    float* price = new float;
    queue.enqueueReadBuffer((option.N % 2 == 1) ? groupB : groupA, CL_TRUE, 0, sizeof(float), price);
    return *price;
}

double OpenCLPricer::computePriceUsingTriangles(Option& option)
{
    int maxNbWorkItems = defaultDevice->getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0];
    int nbWorkItems = 512;
    if (nbWorkItems > maxNbWorkItems)
    {
        cerr << "[ERROR] Number of work items not valid. Cannot have more than "
             << maxNbWorkItems << " work items per work group" << endl;
        exit(EXIT_FAILURE);
    }

    // Compute other parameters
    double dt = option.T / option.N;
    double u = exp(option.sigma * sqrt(dt));
    double d = 1.0 / u;
    double r = exp(option.R * dt);
    double p = (r - d) / (u - d);
    int isCall = (option.isCall) ? 1 : -1; // used in kernel
    int isAmerican = (option.isAmerican) ? 1 : 0; // used in kernel

    // Initialize buffers
    cl::Buffer leaves(*context, CL_MEM_READ_WRITE, sizeof(float) * (option.N + 1));
    cl::Buffer triangle(*context, CL_MEM_READ_WRITE, sizeof(float) * (option.N + 1));

    // Initialize queue
    cl::CommandQueue queue(*context, *defaultDevice);

    // Run init function
    cl::Kernel initKernel(*program, "init");
    initKernel.setArg(0, option.N);
    initKernel.setArg(1, (float)option.S0);
    initKernel.setArg(2, (float)option.K);
    initKernel.setArg(3, (float)dt);
    initKernel.setArg(4, (float)u);
    initKernel.setArg(5, (float)d);
    initKernel.setArg(6, isCall);
    initKernel.setArg(7, leaves);
    queue.enqueueNDRangeKernel(initKernel, cl::NullRange, cl::NDRange(option.N + 1));
    queue.enqueueBarrierWithWaitList();

    int workGroupSize = nbWorkItems + 1;

    // Build upTriangle program
    cl::Kernel upKernel(*program, "upTriangle");
    upKernel.setArg(0, (float)r);
    upKernel.setArg(1, (float)p);
    upKernel.setArg(2, leaves);
    upKernel.setArg(3, cl::Local(sizeof(int) * workGroupSize));
    upKernel.setArg(4, triangle);

    // Build downTriangle program
    cl::Kernel downKernel(*program, "downTriangle");
    downKernel.setArg(0, (float)r);
    downKernel.setArg(1, (float)p);
    downKernel.setArg(2, leaves);
    downKernel.setArg(3, cl::Local(sizeof(int) * workGroupSize));
    downKernel.setArg(4, triangle);

    for (int i = 0; i < option.N / nbWorkItems; ++i)
    {
        int nbWorkGroupsUp = option.N / nbWorkItems - i;
        int nbWorkGroupsDown = nbWorkGroupsUp - 1;
        int nbWorkItemsUp = workGroupSize * nbWorkGroupsUp;
        int nbWorkItemsDown = workGroupSize * nbWorkGroupsDown;

        // Run upTriangle program
        queue.enqueueNDRangeKernel(upKernel, cl::NullRange, cl::NDRange(nbWorkItemsUp)), cl::NDRange(workGroupSize);
        queue.enqueueBarrierWithWaitList();

        // Run downTriangle program
        if (nbWorkGroupsDown > 0)
        {
            queue.enqueueNDRangeKernel(downKernel, cl::NullRange, cl::NDRange(nbWorkItemsDown)), cl::NDRange(workGroupSize);
            queue.enqueueBarrierWithWaitList();
        }
    }

    // Results
    double* price = new double;
    queue.enqueueReadBuffer(leaves, CL_TRUE, 0, sizeof(double), price);
    return *price;
}
