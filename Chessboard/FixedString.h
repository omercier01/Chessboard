
#pragma once

#include "Chessboard.h"

// Custom fixed-length string class to allocate short text on the stack.
// MAXLENGTH is max length without null.
// Assuming MAXLENGTH is at least 1.
template <unsigned int MAXLENGTH>
struct FixedString {

    enum struct Alignment {
        Left,
        Center,
        Right    
    };

    FixedString() {}

    FixedString(const char* str) {
        if(strlen(str) > MAXLENGTH) {
            Serial.println("FixedString ERROR FixedString(...) overflow");
        } else {
            strcpy(chars, str);
        }
    }

    FixedString(const char c) {
        chars[0] = c;
        chars[1] = 0;
    }

    FixedString& operator=(FixedString other) {
        strcpy(chars, other.c_str());
        return *this;
    }

    template <unsigned int OTHERMAXLENGTH>
    FixedString(const FixedString<OTHERMAXLENGTH> other, Alignment alignment = Alignment::Left) {
        if(other.length() > MAXLENGTH) {
            // cut the larger bits depending on alignment
            int iStart;
            switch(alignment) {
                case Alignment::Left:
                    strncpy(chars, other.c_str(), MAXLENGTH);
                    break;
                case Alignment::Center:
                    iStart = (other.length() - MAXLENGTH)/2;
                    strncpy(chars, other.c_str()+iStart, MAXLENGTH);
                    break;
                case Alignment::Right:
                    iStart = (other.length() - MAXLENGTH);
                    strncpy(chars, other.c_str()+iStart, MAXLENGTH);
                    break;
            }
        } else {
            // copy the whole thing
            strcpy(chars, other.c_str());
        }
    }

    FixedString(const int val) {
        itoa(val, chars, 10);
    }

    template <unsigned int OTHERMAXLENGTH>
    void operator=(const FixedString<OTHERMAXLENGTH> other) {
        if(strlen(other.c_str()) > MAXLENGTH) {
            Serial.println("FixedString ERROR operator= CAN'T COPY LONG STRING TO SHORT STRING.");
            return;
        }
        strcpy(chars, other.c_str());
    }

    void operator=(const char* other) {
        if(strlen(other) > MAXLENGTH) {
            Serial.println("FixedString ERROR operator= CAN'T COPY LONG STRING TO SHORT STRING.");
            return;
        }
        strcpy(chars, other);
    }

    void operator=(const char c) {
        chars[0] = c;
        chars[1] = 0;
    }

    FixedString<MAXLENGTH> operator+(const char c) {
        if(length() >= MAXLENGTH) {
            Serial.println("FixedString ERROR operator+ would overflow");
            return *this;
        }
        int len = length();
        chars[len] = c;
        chars[len+1] = 0;
        return *this;
    }

    void operator+=(const char c) {
        operator=(operator+(c));
    }

    template <unsigned int OTHERMAXLENGTH>
    void operator+=(const FixedString<OTHERMAXLENGTH> other) {
        operator=(operator+(other));
    }

    void operator+=(const char* other) {
        operator=(operator+(other));
    }    

    FixedString<MAXLENGTH> operator+(const char* other) {
        if(strlen(chars) + strlen(other) > MAXLENGTH) {
            Serial.println("FixedString ERROR add(...) would overflow");
            return *this;
        }
        strcat(chars, other);
        return *this;
    }

    template <unsigned int OTHERMAXLENGTH>
    FixedString<MAXLENGTH> operator+(const FixedString<OTHERMAXLENGTH> other) {
        if(strlen(chars) + strlen(other.c_str()) > MAXLENGTH) {
            Serial.println("FixedString ERROR add(...) would overflow");
            return *this;
        }
        strcat(chars, other.c_str());
        return *this;
    }

    template <unsigned int OTHERMAXLENGTH>
    bool operator==(const FixedString<OTHERMAXLENGTH> other) {
        return strcmp(chars, other.c_str()) == 0;
    }

    bool operator==(const char* other) {
        return strcmp(chars, other) == 0;
    }

    template <unsigned int OTHERMAXLENGTH>
    bool operator!=(const FixedString<OTHERMAXLENGTH> other) {
        return !(operator==(other));
    }

    bool operator!=(const char* other) {
        return !(operator==(other));
    }

    const char* c_str() const {
        return chars;
    }

    void set(int index, char c) {
        chars[index] = c;
    }

    unsigned int length() const {
        return strlen(chars);
    }

    char charAt(unsigned int i) const {
        if(i >= length()) {
            Serial.println("FixedString ERROR charAt(...) index larger than length");
            return 0;
        }
        return chars[i];
    }

    void removeLastChar() {
        int len = strlen(chars);
        if(len > 0) {
            chars[len-1] = 0;
        }
    }

    void firstCharToUpperCase() {
        if(chars[0] != 0 && chars[0] >= 'a') {
            chars[0] += 'A' - 'a';
        }
    }

    int asInt() {
        return atoi(chars);
    }

    int indexOfFirst(const char delimiter) {
        const char* pos = strchr(chars, delimiter);
        if(!pos) {
            return -1;
        } else {
            return pos - chars;
        }
    }

    template <unsigned int OTHERMAXLENGTH>
    void getSubstring(int iStart, int iEnd, FixedString<OTHERMAXLENGTH>& output) {
        for(int i = iStart; i < iEnd; i++) {
            output.set(i-iStart, chars[i]);
        }
        output.set(iEnd-iStart, 0);
    }

    void setToSubstring(int iStart, int iEnd) {
        for(int i = iStart; i < iEnd; i++) {
            chars[i-iStart] = chars[i];
        }
        chars[iEnd-iStart] = 0;
    }

private:
    char chars[MAXLENGTH+1] = {0};
};


typedef FixedString<SHORT_STRING_LENGTH> ShortString;
typedef FixedString<LONG_STRING_LENGTH> LongString;


