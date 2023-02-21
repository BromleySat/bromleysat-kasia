#include "KasiaEncryption.h"

const char fillchar = '=';
static std::string cvt = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                         "abcdefghijklmnopqrstuvwxyz"
                         "0123456789+/";

std::string KasiaEncryption::encode(std::string text)
{
  std::vector<char> data(text.begin(), text.end());
  std::string::size_type i;
  char c;
  unsigned int len = data.size();
  std::string ret;

  for (i = 0; i < len; ++i)
  {
    c = (data[i] >> 2) & 0x3f;
    ret.append(1, cvt[c]);
    c = (data[i] << 4) & 0x3f;
    if (++i < len)
      c |= (data[i] >> 4) & 0x0f;

    ret.append(1, cvt[c]);
    if (i < len)
    {
      c = (data[i] << 2) & 0x3f;
      if (++i < len)
        c |= (data[i] >> 6) & 0x03;

      ret.append(1, cvt[c]);
    }
    else
    {
      ++i;
      ret.append(1, fillchar);
    }

    if (i < len)
    {
      c = data[i] & 0x3f;
      ret.append(1, cvt[c]);
    }
    else
    {
      ret.append(1, fillchar);
    }
  }

  return (ret);
}
