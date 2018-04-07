__kernel void init(
    const int N,
    const float S0,
    const float K,
    const float dt,
    const float u,
    const float d,
    const int isCall,
    __global float* leaves)
{
    size_t id = get_global_id(0);
    float ST = S0 * pow(u, id) * pow(d, N - id);
    leaves[id] = max(isCall * (ST - K), 0.0f);

    //printf("[init] leaves[%d] = %f\n", id, leaves[id]);
}

__kernel void group(
    const float u,
    const float d,
    const float r,
    const float p,
    const float S0,
    const float K,
    const int isCall,
    const int isAmerican,
    __global float* pricesIn,
    __global float* pricesOut,
    const int nbPoints,
    const int groupSize)
{
    int startId = get_global_id(0) * groupSize;
    int endId = min(startId + groupSize, nbPoints);
    float St;

    for (int i = startId; i < endId; ++i)
    {
        pricesOut[i] = (1.0f / r) * (p * pricesIn[i + 1] + (1.0f - p) * pricesIn[i]);

        if (isAmerican == 1)
        {
            St = S0 * pow(u, i) * pow(d, nbPoints - i);
            pricesOut[i] = max(pricesOut[i], max(isCall * (St - K), 0.0f));
        }

        //printf("[group] pricesOut[%d] = %f\n", i, pricesOut[i]);
    }
}

__kernel void upTriangle(
    const float r,
    const float p,
    __global float* leaves,
    __local float* tmpLeaves,
    __global float* triangle)
{
    int localId = get_local_id(0);
    int localSize = get_local_size(0);
    int groupId = get_group_id(0);

    int nbWorkItems = localSize - 1;
    int offset = nbWorkItems * groupId;
    int globalId = offset + localId;

    // Copy data into temporary buffer
    tmpLeaves[localId] = leaves[globalId];

    float price;
    for (int i = 1; i <= nbWorkItems; ++i)
    {
        // Synchronize at every time step
        barrier(CLK_LOCAL_MEM_FENCE);

        // Compute option price
        if (localId <= nbWorkItems - i)
        {
            price = (1.0f / r) * (p * tmpLeaves[localId + 1] + (1.0f - p) * tmpLeaves[localId]);
            tmpLeaves[localId] = price;
        }

        if (localId == 0)
        {
            // Store boundary node value into secondary global buffer
            triangle[offset + i] = price;

            // Only store first node of first group back into global buffer
            // First nodes of rest of the groups handled by downTriangle
            if (groupId == 0)
            {
                leaves[globalId] = price;
            }
        }
        // Store boundary node value back into global buffer
        else if (localId == nbWorkItems - i)
        {
           leaves[globalId] = price;
        }
    }

    //printf("[upTriangle] leaves[%d] = %f\n", localId, leaves[localId]);
    //printf("[upTriangle] triangle[%d] = %f\n", localId, triangle[localId]);
}

__kernel void downTriangle(
    const float r,
    const float p,
    __global float* leaves,
    __local float* tmpLeaves,
    __global float* triangle)
{
    int localId = get_local_id(0);
    int localSize = get_local_size(0);
    int groupId = get_group_id(0);

    int nbWorkItems = localSize - 1;
    int offset = nbWorkItems * groupId;
    int globalId = offset + localId;

    for (int i = 1; i <= nbWorkItems - 1; ++i)
    {
        // Synchronize at every time step
        barrier(CLK_LOCAL_MEM_FENCE);

        float price, upPrice, downPrice;

        // Compute option price
        upPrice = (localId == nbWorkItems - 1) ? triangle[offset + nbWorkItems + i] : tmpLeaves[localId + 1];
        downPrice = (localId == nbWorkItems - i) ? leaves[globalId] : tmpLeaves[localId];
        if (localId >= nbWorkItems - i && localId < nbWorkItems)
        {
            price = (1.0f / r) * (p * upPrice + (1.0f - p) * downPrice);
            tmpLeaves[localId] = price;
        }
    }

    // Store missing option prices back to original global buffer
    if (localId > 0 && localId < nbWorkItems)
    {
        leaves[globalId] = tmpLeaves[localId];
    }

    // Store first node of each group back to original global buffer
    if (localId == nbWorkItems)
    {
        leaves[globalId] = triangle[globalId + nbWorkItems];
    }

    //printf("[downTriangle] leaves[%d] = %f\n", localId, leaves[localId]);
    //printf("[downTriangle] triangle[%d] = %f\n", localId, triangle[localId]);
}
