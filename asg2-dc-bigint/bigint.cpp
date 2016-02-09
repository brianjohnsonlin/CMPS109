// $Id: bigint.cpp,v 1.61 2014-06-26 17:06:06-07 - - $

#include <cstdlib>
#include <exception>
#include <limits>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"
using digit_t = unsigned char;
using bigvalue_t = vector<digit_t>;

bigint::bigint (long that): big_value (that) {
	DEBUGF ('~', this << " -> " << big_value);
}

bigint::bigint (const string& that) {
	DEBUGF ('~', "that = " << that);
	auto itor = that.crbegin();
	negative = false;
	if (that.size() > 1 and *(that.begin()) == '_') negative = true;
	while (itor != that.crend()){
		if(negative && *itor == *(that.begin())) itor++;
		else if(*itor >= '0' && *itor <= '9') big_value.push_back((*itor++) - '0');
		else throw ydc_exn ("invalid input: " + *itor);
	}
}

bool do_bigless (const bigvalue_t& left, const bigvalue_t& right){
	//returns true if equal or "left" is bigger
	//or false if "right" is bigger or values are equal (compares abs values)
	if(left.size() > right.size()) return true;
	if(left.size() < right.size()) return false;
	for(int i = left.size()-1; i > -1; i--){
		if(left[i] > right[i]) return true;
		if(left[i] < right[i]) return false;
	}
	return false;
}

bigvalue_t do_bigadd (const bigvalue_t& left, const bigvalue_t& right){
	if(right.size() == 0) return left;
	bigvalue_t bigger; bigvalue_t smaller;
	if(do_bigless(left,right)){bigger = left; smaller = right;}
	else{bigger = right; smaller = left;}
	bigvalue_t result;
	bool carry = false;
	for(size_t i = 0; i < bigger.size() + 1; i++){
		digit_t biggerDigit = 0;
		digit_t smallerDigit = 0;
		if(i < bigger.size()) biggerDigit = bigger[i];
		if(i < smaller.size()) smallerDigit = smaller[i];
		digit_t resultDigit = biggerDigit + smallerDigit;
		if(carry) resultDigit++;
		carry = false;
		if(resultDigit > 9){
			resultDigit -= 10;
			carry = true;
		}
		result.push_back(resultDigit);
	}
	while (result.size() > 0 && result.back() == 0) result.pop_back();
	return result;
}

bigvalue_t do_bigsub (const bigvalue_t& left, const bigvalue_t& right){
	if(right.size() == 0) return left;
	bigvalue_t bigger; bigvalue_t smaller;
	if(do_bigless(left,right)){bigger = left; smaller = right;}
	else{bigger = right; smaller = left;}
	bigvalue_t result;
	bool borrow = false;
	for(size_t i = 0; i < bigger.size(); i++){
		digit_t biggerDigit = bigger[i];
		digit_t smallerDigit = 0;
		if(i < smaller.size()) smallerDigit = smaller[i];
		int resultInt = biggerDigit - smallerDigit;
		if(borrow) resultInt--;
		borrow = false;
		if(resultInt<0){
			resultInt += 10;
			borrow = true;
		}
		digit_t resultDigit = resultInt;
		result.push_back(resultDigit);
	}
	while (result.size() > 0 && result.back() == 0) result.pop_back();
	return result;
}

bigint operator+ (const bigint& left, const bigint& right) {
	bigint result;
	const bigint *bigger;
	const bigint *smaller;
	if(do_bigless(left.big_value,right.big_value))
		{bigger = &left; smaller = &right;}
	else{bigger = &right; smaller = &left;}
	if(bigger->negative == smaller->negative)
		result.big_value = do_bigadd(bigger->big_value,smaller->big_value);
	else result.big_value = do_bigsub(bigger->big_value,smaller->big_value);
	result.negative = bigger->negative;
	if(result.big_value.size() == 0) result.negative = false;
	return result;
}

bigint operator- (const bigint& left, const bigint& right) {
	bigint result;
	const bigint *bigger;
	const bigint *smaller;
	bool reversed = false;
	if(do_bigless(left.big_value,right.big_value))
		{bigger = &left; smaller = &right;}
	else{bigger = &right; smaller = &left; reversed = true;}
	if(bigger->negative == smaller->negative)
		result.big_value = do_bigsub(bigger->big_value,smaller->big_value);
	else result.big_value = do_bigadd(bigger->big_value,smaller->big_value);
	result.negative = bigger->negative;
	if(reversed) result.negative = !result.negative;
	if(result.big_value.size() == 0) result.negative = false;
	return result;
}

/*bigint operator+ (const bigint& right) {
	return +right.big_value;
}*///???

/*bigint operator- (const bigint& right) {
	return -right.big_value;
}*///???

long bigint::to_long() const {
	if (*this <= bigint ("_9223372036854775808")
	 or *this > bigint ("9223372036854775807"))
		throw range_error ("bigint__to_long: out of range");
	auto itor = this->big_value.crbegin();
	long newval = 0;
	while (itor != this->big_value.crend()) newval = newval * 10 + *itor++;
	return newval;
}

bigvalue_t do_bigmul (const bigvalue_t& left, const bigvalue_t& right){
	bigvalue_t result;
	for (size_t i = 0; i < left.size() + right.size(); i++) result.push_back(0);
	for (size_t i = 0; i < left.size(); i++){
		digit_t c = 0;
		for (size_t j = 0; j < right.size(); j++){
			digit_t d = result[i+j] + left[i]*right[j] + c;
			result[i+j] = d % 10;
			c = d / 10;
		}
		result[i+right.size()] = c;
	}
	while (result.size() > 0 && result.back() == 0) result.pop_back();
	return result;
}

bigint operator* (const bigint& left, const bigint& right) {
	bigint result;
	result.big_value = do_bigmul(left.big_value, right.big_value);
	if(left.negative == right.negative) result.negative = false;
	else result.negative = true;
	if(result.big_value.size() == 0) result.negative = false;
	return result;
}

void multiply_by_2 (bigvalue_t& value) {
	value = do_bigmul(value, {2});
}

void divide_by_2 (bigvalue_t& value) {
	bool carry = false;
	for(size_t i = value.size(); i > 0; i--){
		bool carry_ = carry;
		if(value[i-1] % 2 == 1) carry = true;
		else carry = false;
		value[i-1] /= 2;
		if(carry_) value[i-1] += 5;
	}
	while (value.size() > 0 && value.back() == 0) value.pop_back();
}

bigint::quot_rem do_bigdiv (const bigvalue_t& left, const bigvalue_t& right) {
	if (right.size() == 0) throw domain_error ("divide by 0");
	static bigvalue_t zero = {};
	if (right.size() == 0) throw domain_error ("bigint::divide");
	bigvalue_t divisor = right;
	bigvalue_t quotient = {};
	bigvalue_t remainder = left;
	bigvalue_t power_of_2 = {1};
	//cout << "power_of_2: "; for(size_t i = power_of_2.size(); i > 0; i--) cout << (int)power_of_2[i-1]; cout << endl;
	//cout << "divisor:    "; for(size_t i = divisor.size(); i > 0; i--) cout << (int)divisor[i-1]; cout << endl;
	//cout << "quotient:   "; for(size_t i = quotient.size(); i > 0; i--) cout << (int)quotient[i-1]; cout << endl;
	//cout << "remainder:  "; for(size_t i = remainder.size(); i > 0; i--) cout << (int)remainder[i-1]; cout << endl;
	while (do_bigless(remainder, divisor)) {
		multiply_by_2 (divisor);
		multiply_by_2 (power_of_2);
	}
	while (do_bigless(power_of_2, zero)) {
		if (not do_bigless(divisor, remainder)) {
			remainder = do_bigsub(remainder, divisor);
			quotient = do_bigadd(quotient, power_of_2);
		}
		divide_by_2 (divisor);
		divide_by_2 (power_of_2);
	}
	while (quotient.size() > 0 && quotient.back() == 0) quotient.pop_back();
	while (remainder.size() > 0 && remainder.back() == 0) remainder.pop_back();
	return {quotient, remainder};
}

bigint operator/ (const bigint& left, const bigint& right) {
	bigint result;
	result.big_value = do_bigdiv(left.big_value, right.big_value).first;
	if(left.negative == right.negative) result.negative = false;
	else result.negative = true;
	if(result.big_value.size() == 0) result.negative = false;
	return result;
}

bigint operator% (const bigint& left, const bigint& right) {
	bigint result;
	result.big_value = do_bigdiv(left.big_value, right.big_value).second;
	result.negative = false;
	return result;
}

bool operator== (const bigint& left, const bigint& right) {
	if(left.big_value.size() != right.big_value.size() || left.negative != right.negative) return false;
	for(int i = left.big_value.size()-1; i > -1; i--) if(left.big_value[i] != right.big_value[i]) return false;
	return true;
}

bool operator< (const bigint& left, const bigint& right) {
	if(left.negative != right.negative) return left.negative;
	if(left.big_value.size() > right.big_value.size()) return left.negative;
	if(left.big_value.size() < right.big_value.size()) return !left.negative;
	for(size_t i = 0; i < left.big_value.size(); i++){
		if(left.big_value[i] > right.big_value[i]) return !left.negative;
		if(left.big_value[i] < right.big_value[i]) return left.negative;
	}
	return false;
}

ostream& operator<< (ostream& out, const bigint& that) {
	if(that.negative) out << '-';
	size_t newlinecounter = 0;
	for(size_t i = that.big_value.size(); i > 0; i--){
		out << (int)that.big_value[i-1];
		newlinecounter++;
		if(newlinecounter == 69){
			out << '\\' << endl;
			newlinecounter = 0;
		}
	}
	if(that.big_value.size() == 0) out << '0';
	return out;
}

bigint pow (const bigint& base, const bigint& exponent) {
	DEBUGF ('^', "base = " << base << ", exponent = " << exponent);
	if (base == 0) return 0;
	bigint base_copy = base;
	long expt = exponent.to_long();
	bigint result("1");
	if (expt < 0) //will always return less than 1, which bigint cannot store
		return 0;
	while (expt > 0) {
		if (expt % 2 == 1) { //odd
			result = result * base_copy;
			--expt;
		}else { //even
			base_copy = base_copy * base_copy;
			expt /= 2;
		}
	}
	DEBUGF ('^', "result = " << result);
	return result;
}
