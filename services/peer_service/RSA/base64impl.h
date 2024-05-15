//
// Created by costi on 11/04/24.
//

#ifndef RSA_BASE64IMPL_H
#define RSA_BASE64IMPL_H

#include <string>
using namespace std;

void initBase64Alphabet();
string base64encode(string source);
string base64decode(string source);

#endif //RSA_BASE64IMPL_H
