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

double SequentialPricer::computePrice(Option& option)
{
    // Compute other parameters
    double dt = option.T / option.N;
    double u = exp(option.sigma * sqrt(dt));
    double d = 1.0 / u;
    double r = exp(option.R * dt);
    double p = (r - d) / (u - d);

    // Fill binomial tree leaves with its final option values
    vector<double> leaves(option.N + 1);
    double ST;
    for (int i = 0; i <= option.N; ++i)
    {
        ST = option.S0 * pow(u, i) * pow(d, option.N - i);
        leaves[i] = (option.callPricing) ? max(ST - option.K, 0.0) : max(option.K - ST, 0.0);
        if (option.withDebugInfo)
        {
            cout << "[TRACE] leaves[" << i << "] = " << leaves[i] << endl;
        }
    }

    // Iterate backwards to compute initial option value
    for (int i = option.N - 1; i >= 0; --i)
    {
        for (int j = 0; j <= i; ++j)
        {
            leaves[j] = (1.0 / r) * (p * leaves[j+1] + (1.0 - p) * leaves[j]);
            ST = option.S0 * pow(u, j) * pow(d, i-j);
            leaves[j] = max(leaves[j], (option.callPricing) ? max(ST - option.K, 0.0) : max(option.K - ST, 0.0));
        }
    }

    return leaves[0];
}
