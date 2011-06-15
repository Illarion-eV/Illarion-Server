#include <string>
#include <algorithm>
#include <cctype>

bool mypred(char c1, char c2) {
	return tolower(c1) == tolower(c2);
}

bool comparestrings_nocase( const std::string& s1, const std::string& s2 ) {

	if (s1.size() != s2.size())
		return false;

	return equal(s1.begin(), s1.end(), s2.begin(), mypred);

}
