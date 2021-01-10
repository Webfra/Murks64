#ifndef HEX_H
#define HEX_H

#include <stdint.h>
#include <string>

//========================================================================
template<typename int_type>
int hex2bin(const std::string &hex, int_type &result, bool accept_lower = false, bool end_on_space = false)
{
    int num_chars_used = 0;
    int_type _result = 0;
    for(auto &c:hex)
    {
        if( end_on_space and (c==' ') ) break;

        _result <<= 4;
        if     ((c>='0')&&(c<='9')) { _result += c - '0';}
        else if((c>='A')&&(c<='F')) { _result += c - 'A' + 10; }
        else if(accept_lower && (c>='a')&&(c<='f')) { _result += c - 'a' + 10; }
        else
        {
            return 0;
        }
        num_chars_used++;
    }
    result = _result;
    return num_chars_used;
}

//========================================================================
inline
char bin2hex1(uint8_t value)
{
    char c = value & 0x0f;
    if( c>=10 ) c+= 'A'-10;
    else        c+= '0';
    return c;
}

//========================================================================
inline
std::string bin2hex2( uint8_t value )
{
    std::string s="";
    s += bin2hex1( value>>4 );
    s += bin2hex1( value );
    return s;
}

//========================================================================
inline
std::string bin2hex4( uint16_t value )
{
    std::string s;
    s = bin2hex2( value>>8 ) + bin2hex2(value);
    return s;
}

//========================================================================

#endif // HEX_H
