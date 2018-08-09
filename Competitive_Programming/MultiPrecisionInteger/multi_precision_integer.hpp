#include "../header.hpp"

class MPI : public vector<int> {
private:

    static constexpr int root = 5;
    static constexpr int MOD_ = 924844033;

    inline bool abs_less(const MPI& a, const MPI& b) const {
        if(a.size() != b.size()) return a.size() < b.size();
        for(int index = (int)a.size() - 1; index >= 0; index--){
            if(a[index] != b[index]) return a[index] < b[index];
        }
        return false;
    }
    inline static void num_sbst(MPI& a, const MPI& b){
        int n = (int)b.size();
        a.resize(n);
        for(int i = 0; i < n; i++) a[i] = b[i];
    }
    inline void add(const MPI& a, const MPI& b, MPI& res) const {
        num_sbst(res, a);
        int mx = (int)max(a.size(), b.size());
        res.resize(mx, 0);
        int carry = 0;
        for(int i = 0; i < mx; i++){
            int val = res[i] + ((i < (int)b.size()) ? b[i] : 0) + carry;
            carry = val/10;
            res[i] = val%10;
        }
        if(carry) res.push_back(1);
    }
    inline void sub(const MPI& a, const MPI& b, MPI& res) const {
        num_sbst(res, a);
        int carry = 0;
        for(int i = 0; i < (int)res.size(); i++){
            int val = res[i] - carry - ((i < (int)b.size()) ? b[i] : 0);
            if(val < 0){
                carry = 1, val += 10;
            }else{
                carry = 0;
            }
            res[i] = val;
        }
        trim_digit(res), trim_sign(res);
    }
    inline static int add_(const int x, const int y) { return (x + y < MOD_) ? x + y : x + y - MOD_; }
    inline static int mul_(const int x, const int y) { return (ll)x * y % MOD_; }
    inline static int power(int x, int n){
        int res = 1;
        while(n > 0){
            if(n & 1) res = mul_(res, x);
            x = mul_(x, x);
            n >>= 1;
        }
        return res;
    }
    inline static int inverse(const int x) { return power(x, MOD_ - 2); }
    inline static void ntt(vector<int>& a, const bool rev = false){
        int i,j,k,s,t,v,w,wn;
        const int size = (int)a.size();
        const int height = (int)log2(2 * size - 1);
        for(i = 0; i < size; i++){
            j = 0;
            for(k = 0; k < height; k++) j |= (i >> k & 1) << (height - 1 - k);
            if(i < j) std::swap(a[i], a[j]);
        }
        for(i = 1; i < size; i *= 2) {
            w = power(root, (MOD_ - 1) / (i * 2));
            if(rev) w = inverse(w);
            for(j = 0; j < size; j += i * 2){
                wn = 1;
                for(k = 0; k < i; k++){
                    s = a[j + k], t = mul_(a[j + k + i], wn);
                    a[j + k] = add_(s, t);
                    a[j + k + i] = add_(s, MOD_ - t);
                    wn = mul_(wn, w);
                }
            }
        }
        if(rev){
            v = inverse(size);
            for (i = 0; i < size; i++) a[i] = mul_(a[i], v);
        }
    }
    static void mul(const MPI& a, const MPI& b, MPI& res){
        const int size = (int)a.size() + (int)b.size() - 1;
        int t = 1;
        while (t < size) t <<= 1;
        vector<int> A(t, 0), B(t, 0);
        for(int i = 0; i < (int)a.size(); i++) A[i] = a[i];
        for(int i = 0; i < (int)b.size(); i++) B[i] = b[i];
        ntt(A), ntt(B);
        for(int i = 0; i < t; i++) A[i] = mul_(A[i], B[i]);
        ntt(A, true);
        res.resize(size);
        int carry = 0;
        for(int i = 0; i < size; i++){
            int val = A[i] + carry;
            carry = val / 10;
            res[i] = val % 10;
        }
        if(carry) res.push_back(carry);
        trim_digit(res), trim_sign(res);
    }
    inline bool isZero() const {
        return (int)size() == 1 && (*this)[0] == 0;
    }
    inline static bool div_geq(const MPI& mod, const MPI& num){
        if((int)mod.size() != (int)num.size()) return (int)mod.size() > (int)num.size();
        int n = (int)mod.size();
        for(int i = 0; i < n; i++){
            if(mod[i] != num[n-1-i]){
                return mod[i] > num[n-1-i];
            }
        }
        return true;
    }
    inline void div_(const MPI& a, const MPI& b, MPI& quo, MPI& mod) const {
        vector<MPI> mult(9);
        mult[0] = b;
        for(int i = 0; i < 8; i++) mult[i+1] = mult[i] + b;
        for(int i = (int)a.size() - 1; i >= 0; i--){
            if(mod.isZero()){
                mod.back() = a[i];
            }else{
                mod.push_back(a[i]);
            }
            if(div_geq(mod, b)){
                int l = 0, r = 9;
                reverse(mod.begin(), mod.end());
                while(r-l>1){
                    int mid = (l+r)/2;
                    if(mult[mid] > mod){
                        r = mid;
                    }else{
                        l = mid;
                    }
                }
                mod -= mult[l];
                reverse(mod.begin(), mod.end());
                quo.push_back(l+1);
            }else{
                quo.push_back(0);
            }
        }
        reverse(quo.begin(), quo.end());
        trim_digit(quo);
        reverse(mod.begin(), mod.end());
    }

public:
    bool sign;

    MPI() : sign(false){}

    MPI(ll val) : MPI(){
        if(val == 0){
            this->push_back(0);
        }else{
            if(val < 0) sign = true, val = -val;
            while(val){
                this->push_back(val%10);
                val /= 10;
            }
        }
    }

    MPI(const string& s) : MPI(){
        if(s[0] == '-') sign = true;
        for(int i = (int)s.size() - 1; i >= sign; i--) this->push_back(s[i]-'0');
    }

    inline static void trim_sign(MPI& num){
        if(num.isZero()) num.sign = false;
    }

    inline static void trim_digit(MPI& num){
        while(num.back() == 0 && (int)num.size() >= 2) num.pop_back();
    }

    friend istream& operator>>(istream& is, MPI& num) {
        string s;
        is >> s;
        num = MPI(s);
        return is;
    }

    friend ostream& operator<<(ostream& os, const MPI& num) {
        if(num.sign) os << '-';
        for(int i = (int)num.size()-1; i >= 0; i--) os << (char)('0'+num[i]);
        return os;
    }

    void operator=(ll val) {
        *this = MPI(val);
    }

    bool operator<(const MPI& another) const {
        if(sign ^ another.sign) return sign;
        if(size() != another.size()) return (size() < another.size()) ^ sign;
        for(int index = (int)size() - 1; index >= 0; index--){
            if((*this)[index] != another[index]) return ((*this)[index] < another[index]) ^ sign;
        }
        return false;
    }

    bool operator>(const MPI& another) const {
        if(sign ^ another.sign) return another.sign;
        if(size() != another.size()) return (size() > another.size()) ^ sign;
        for(int index = (int)size() - 1; index >= 0; index--){
            if((*this)[index] != another[index]) return ((*this)[index] > another[index]) ^ sign;
        }
        return false;
    }

    bool operator<=(const MPI& another) const {
        if(sign ^ another.sign) return sign;
        if(size() != another.size()) return (size() < another.size()) ^ sign;
        for(int index = (int)size() - 1; index >= 0; index--){
            if((*this)[index] != another[index]) return ((*this)[index] < another[index]) ^ sign;
        }
        return true;
    }

    bool operator>=(const MPI& another) const {
        if(sign ^ another.sign) return another.sign;
        if(size() != another.size()) return (size() > another.size()) ^ sign;
        for(int index = (int)size() - 1; index >= 0; index--){
            if((*this)[index] != another[index]) return ((*this)[index] > another[index]) ^ sign;
        }
        return true;
    }

    bool operator<(const ll another) const {
        return (*this) < MPI(another);
    }

    bool operator>(const ll another) const {
        return (*this) > MPI(another);
    }

    bool operator<=(const ll another) const {
        return (*this) <= MPI(another);
    }

    bool operator>=(const ll another) const {
        return (*this) >= MPI(another);
    }

    bool operator==(const ll another) const {
        return *this == MPI(another);
    }

    bool operator!=(const ll another) const {
        return !(*this == MPI(another));
    }

    inline MPI abs() const {
        MPI res = (*this);
        res.sign = false;
        return res;
    }

    MPI operator+(const MPI& num) const {
        MPI res; res.sign = sign;
        if(sign != num.sign){
            if(abs_less(*this, num)){
                res.sign = num.sign;
                sub(num, *this, res);
                return res;
            }else{
                sub(*this, num, res);
                return res;
            }
        }
        add(*this, num, res);
        return res;
    }

    MPI operator+(ll num) const {
        return (*this) + MPI(num);
    }

    void operator+=(const MPI& num){
        *this = *this + num;
    }

    void operator+=(ll num){
        *this = *this + num;
    }

    MPI operator-(const MPI& num) const {
        if(sign != num.sign){
            MPI res; res.sign = sign;
            add(*this, num, res);
            return res;
        }
        MPI res; res.sign = (abs_less(*this, num) ^ sign);
        if(res.sign){
            sub(num, *this, res);
        }else{
            sub(*this, num, res);
        }
        return res;
    }

    MPI operator-(ll num) const {
        return (*this) - MPI(num);
    }

    void operator-=(const MPI& num){
        *this = *this - num;
    }

    void operator-=(ll num){
        *this = *this - num;
    }

    MPI operator*(MPI num) const {
        MPI res; res.sign = (sign^num.sign);
        mul(*this, num, res);
        return res;
    }

    MPI operator*(ll num) const {
        return (*this) * MPI(num);
    }

    void operator*=(const MPI& num){
        *this = *this * num;
    }

    void operator*=(ll num){
        *this = *this * num;
    }

    MPI operator/(const MPI& num) const {
        MPI num_ = num.abs();
        MPI a, b;
        div_(*this, num_, a, b);
        a.sign = (sign^num.sign);
        trim_sign(a);
        return a;
    }

    MPI operator/(ll num) const {
        return (*this) / MPI(num);
    }

    void operator/=(const MPI& num){
        *this = *this / num;
    }

    void operator/=(ll num){
        *this = *this / num;
    }

    MPI operator%(const MPI& num) const {
        MPI num_ = num.abs();
        MPI a, b;
        div_(*this, num_, a, b);
        b.sign = sign;
        trim_sign(b);
        return b;
    }

    MPI operator%(ll num) const {
        return (*this) % MPI(num);
    }

    void operator%=(const MPI& num){
        *this = *this % num;
    }

    void operator%=(ll num){
        *this = *this % num;
    }

    inline MPI div2() const {
        MPI res; res.sign = sign;
        int n = (int)this->size(), carry = 0;
        for(int i = n-1; i >= 0; i--){
            int val = (*this)[i]+carry*10;
            carry = val%2;
            if(i != n-1 || val >= 2) res.push_back(val/2);
        }
        reverse(res.begin(), res.end());
        return res;
    }
};
