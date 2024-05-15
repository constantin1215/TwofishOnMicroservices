#include <algorithm>
#include <iostream>
#include <string>
#include <bitset>
#include <vector>
#include "base64impl.h"

vector<char> base64alphabet;

void initBase64Alphabet() {
    for(int i = 0; i < 26; i++)
        base64alphabet.push_back('A' + i);

    for(int i = 0; i < 26; i++)
        base64alphabet.push_back('a' + i);

    for(int i = 0; i < 10; i++)
        base64alphabet.push_back('0' + i);

    base64alphabet.push_back('+');
    base64alphabet.push_back('/');
}

int indexInBase64Alphabet(const char ch) {
    for(int i = 0; i < base64alphabet.size(); i++)
        if (base64alphabet[i] == ch)
            return i;

    throw runtime_error("Wtf");
}

void displayBase64Alphabet() {
    for(int i = 0;i < 64; i++)
        cout << base64alphabet.at(i) << ' ';
}

string base64encode(string source)
{
    string result;
    const int sourceLength = source.length();
    int paddingSize = 0;

    if (sourceLength * 8 % 24 != 0) {
        paddingSize = 24 - sourceLength * 8 % 24;
    }

    string bitsAsString;
    for (int i = 0; i < sourceLength; i++) {
        bitsAsString += bitset<8>(source.c_str()[i]).to_string();
    }

    int addedPadding = 0;
    while (paddingSize) {
        paddingSize -= 8;
        addedPadding++;
        bitsAsString += "00000000";
    }

    const int resultSize = bitsAsString.length() / 6;
    for (int i = 0; i < resultSize; i++) {
        result += base64alphabet.at(bitset<6>(bitsAsString.substr(i * 6, 6)).to_ulong());
    }

    while (addedPadding) {
        result.at(result.length() - addedPadding) = '=';
        addedPadding--;
    }

    return result;
}

string base64decode(string source) {
    string result;

    const int equalsCount = count(source.begin(), source.end(), '=');

    string resultAsBinary;
    for(int i = 0;i < source.length() - equalsCount; i++) {
        const unsigned char valueOfChar = indexInBase64Alphabet(source[i]);
        resultAsBinary += bitset<6>(bitset<8>(valueOfChar).to_string().substr(2, 6)).to_string();
    }

    for(int i = 0; i < resultAsBinary.length() / 8; i++)
        result += static_cast<char>(bitset<8>(resultAsBinary.substr(8 * i, 8)).to_ulong());


    return result;
}