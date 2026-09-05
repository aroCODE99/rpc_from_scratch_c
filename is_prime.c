#include "is_prime.h"

// i don't know how this function works
bool is_prime(int num)
{
    if (num == 2 || num == 3) {
        return true;
    }
    
    // Check for 1 or easy modulos
    if (num == 1 || num % 2 == 0 || num % 3 == 0) {
        return false;
    }
    
    // Now check all the nums up to sqrt(num)
    int i = 5;
    while (i * i <= num) {
        // If we've found something (or something + 2) that divides it evenly, it's not
        // prime.
        if (num % i == 0 || num % (i + 2) == 0) {
            return false;
        }
        i += 6;
    }
    return true;

}
