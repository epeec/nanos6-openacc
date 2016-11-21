#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <bitset>
#define MAX_CACHES  8

/* Not sure where this kind of declaration should go
 * I dont think a class is necessary, since only a bitset with a certain number of bits is necessary (for now)
 * Anyway, it should go in a more appropiate place (such as hardware folder). Will wait for discussion before deciding anything.
 */

typedef std::bitset<MAX_CACHES> cache_mask; 

#endif //GLOBALS_HPP
