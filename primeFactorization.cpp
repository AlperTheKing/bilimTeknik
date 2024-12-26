#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <gmpxx.h>

extern "C" {
#include <ecm.h>  // The GMP-ECM C header
}

std::atomic_bool g_foundFactor(false);
mpz_class g_factor(0);
std::mutex g_factorMutex;

static const char* N_str = 
  "1000000000000000000000000000000000000000000000000000000000000019";

// Each thread tries a certain number of curves
void workerECM(const mpz_class& N, unsigned threadID, unsigned totalCurvesPerThread)
{
    mpz_t mpzN;
    mpz_init_set(mpzN, N.get_mpz_t());

    mpz_t factor;
    mpz_init(factor);

    // For older GMP-ECM, 'params' is an array of 1
    // e.g. ecm_params params; ecm_init(params);
    // We'll let ecm_init do defaults (no B2 assignment).
    ecm_params params;
    ecm_init(params);

    // Example: do 'totalCurvesPerThread' ECM attempts
    for (unsigned i = 0; i < totalCurvesPerThread && !g_foundFactor.load(); i++)
    {
        // B1 = 1e6, you can adjust if needed
        int ret = ecm_factor(factor, mpzN, 1e9, params);
        if (ret == 1)
        {
            mpz_class candidate(factor);
            if (candidate > 1 && candidate < N) 
            {
                bool expected = false;
                if (g_foundFactor.compare_exchange_strong(expected, true))
                {
                    std::lock_guard<std::mutex> lock(g_factorMutex);
                    g_factor = candidate;
                }
            }
            break;
        }
    }

    mpz_clear(factor);
    mpz_clear(mpzN);
    ecm_clear(params);
}

int main()
{
    mpz_class N(N_str);
    std::cout << "Factoring N = 10^63 + 19 with older GMP-ECM.\n";
    std::cout << "N = " << N.get_str() << "\n\n";

    auto start = std::chrono::steady_clock::now();

    unsigned numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;
    std::cout << "Using " << numThreads << " threads.\n";

    // Suppose we want 100 total curves:
    unsigned totalCurves = 100;
    unsigned curvesPerThread = totalCurves / numThreads;
    if (curvesPerThread < 1) curvesPerThread = 1;

    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (unsigned t = 0; t < numThreads; t++)
    {
        threads.emplace_back(workerECM, std::cref(N), t, curvesPerThread);
    }

    for (auto &th : threads) {
        if (th.joinable()) th.join();
    }

    auto end = std::chrono::steady_clock::now();
    double elapsedSec = std::chrono::duration<double>(end - start).count();

    mpz_class f1;
    {
        std::lock_guard<std::mutex> lock(g_factorMutex);
        f1 = g_factor;
    }

    if (f1 == 0)
    {
        std::cout << "\nNo factor found after " << totalCurves 
                  << " total curves.\nTime elapsed: " 
                  << elapsedSec << " seconds.\n";
        return 0;
    }

    mpz_class f2 = N / f1;
    if (f1 > f2) std::swap(f1, f2);

    std::cout << "\nFactor found!\n";
    std::cout << "  f1 = " << f1.get_str() << "\n";
    std::cout << "  f2 = " << f2.get_str() << "\n";
    if ((f1 * f2) == N) {
        std::cout << "Verification: f1 * f2 == N. Good!\n";
    } else {
        std::cout << "ERROR: Product mismatch.\n";
    }

    std::cout << "Time elapsed: " << elapsedSec << " seconds.\n";
    return 0;
}
