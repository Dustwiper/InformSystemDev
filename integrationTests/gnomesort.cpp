#include "gnomesort.h"
#include <utility>

std::vector<int> gnomeSort(std::vector<int> input)
{
    int i = 0;
    const int n = static_cast<int>(input.size());
    if (n <= 1) return input;

    while (i < n) {
        if (i == 0) {
            ++i;
        } else if (input[i - 1] <= input[i]) {
            ++i;
        } else {
            std::swap(input[i], input[i - 1]);
            --i;
        }
    }
    return input;
}
