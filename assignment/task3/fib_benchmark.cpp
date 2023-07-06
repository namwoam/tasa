#include <iostream>
#include <vector>
#define MAX_BENCHMARK_CYCLE 9999

int main()
{

    for (int i = 0; i < MAX_BENCHMARK_CYCLE; i++)
    {
        std::vector<int> fib_result(10);
        fib_result[0] = 1;
        fib_result[1] = 1;
        for (int k = 2; k < fib_result.size(); k++)
        {
            fib_result[k] = fib_result[k - 1] + fib_result[k];
        }
        for (int k = 0; k < fib_result.size(); k++)
        {
            std::cout << "The " << k + 1 << " fibonacci number is: " << fib_result.at(k) << std::endl;
        }
    }
    return 0;
}