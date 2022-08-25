#include <string>

#include "../include/format.h"

using std::string;
using std::to_string;

static inline string normalizeTimeToken(long time) {
  return time < 10 ? (string("0") + to_string(time)) : to_string(time);
}

/**
 * @brief takes Long int measuring seconds and output time in 'HH:MM:SS'
 *
 * @param seconds 
 * @return string 
 */
string Format::ElapsedTime(long seconds) {
  long hours = seconds / (60 * 60);
  seconds %= (60 * 60);
  long minutes = seconds / 60;
  seconds %= 60;
  return normalizeTimeToken(hours % 24) + string(":") + normalizeTimeToken(minutes) +
         ":" + normalizeTimeToken(seconds);
}
