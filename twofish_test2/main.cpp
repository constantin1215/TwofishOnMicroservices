#include "twofish.h"
#include "keys.h"
#include "galois.h"

int main() {
    Twofish twofish(TestKey2.N, TestKey2.key);
    //twofish.showKey();
    twofish.keySchedule();
    return 0;
}
