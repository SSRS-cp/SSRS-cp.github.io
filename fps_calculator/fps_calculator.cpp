#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <algorithm>
#include <emscripten/emscripten.h>
const int INF = 10000;
const long long MOD = 998244353;
const int INDETERMINATE_COUNT = 2;
const std::string INDETERMINATE = "xy";
const std::vector<std::string> token = {"+", "-", "*", "/", "^", "=", "(", ")", "[", "]", ",", "print", "diff", "int", "log", "exp"};
//https://qiita.com/soramimi_jp/items/1b7ed0ddcefb0f4a7172
EM_JS(void *, getElementValue_, (char const *id), {
  var e = document.getElementById(UTF8ToString(id));
  var str = e.value;
  var len = lengthBytesUTF8(str) + 1;
  var heap = _malloc(len);
  stringToUTF8(str, heap, len);
  return heap;
});
std::string getElementValue(std::string const &id){
  void *p = getElementValue_(id.c_str());
  std::string s((char const *)p);
  free(p);
  return s;
}
void setElementInnerText(std::string const &id, std::string const &value){
  EM_ASM({
    var e = document.getElementById(UTF8ToString($0));
    e.innerText = UTF8ToString($1);
  }, id.c_str(), value.c_str());
}
bool is_number(std::string s){
  return isdigit(s[0]) != 0;
}
bool is_indeterminate(char c){
  return std::find(INDETERMINATE.begin(), INDETERMINATE.end(), c) != INDETERMINATE.end();
}
bool is_indeterminate(std::string s){
  return s.size() == 1 && is_indeterminate(s[0]);
}
int get_indeterminate_id(char c){
  return std::find(INDETERMINATE.begin(), INDETERMINATE.end(), c) - INDETERMINATE.begin();
}
int get_indeterminate_id(std::string s){
  return get_indeterminate_id(s[0]);
}
bool is_variable(char c){
  return isalpha(c) != 0 && !is_indeterminate(c);
}
bool is_variable(std::string s){
  return s.size() == 1 && is_variable(s[0]);
}
std::vector<std::string> split(std::string S){
  int N = S.size();
  std::vector<std::string> A(1);
  for (int i = 0; i < N; i++){
    if (S[i] == '\n'){
      A.push_back("");
    } else if (isspace(S[i]) == 0){
      A.back() += S[i];
    }
  }
  return A;
}
std::vector<std::string> lex(std::string S){
  int N = S.size();
  std::vector<std::string> A(1);
  A[0] += S[0];
  for (int i = 1; i < N; i++){
    if (isalpha(S[i]) != 0 && isalpha(S[i - 1]) != 0){
      A.back() += S[i];
    } else if (isdigit(S[i]) != 0 && isdigit(S[i - 1]) != 0){
      A.back() += S[i];
    } else {
      A.push_back("");
      A.back() += S[i];
    }
  }
  int M = A.size();
  for (int i = 0; i < M; i++){
    if (!is_number(A[i]) && !is_indeterminate(A[i]) && !is_variable(A[i])){
      if (std::find(token.begin(), token.end(), A[i]) == token.end()){
        return {"error"};
      }
    }
  }
  return A;
}
int number(std::vector<std::pair<std::string, std::vector<int>>> &T, std::vector<std::string> &S, int &p){
  if (!is_number(S[p])){
    return -1;
  }
  int v = T.size();
  T.push_back(std::make_pair(S[p], std::vector<int>(0)));
  p++;
  return v;
}
int variable(std::vector<std::pair<std::string, std::vector<int>>> &T, std::vector<std::string> &S, int &p){
  if (!is_variable(S[p])){
    return -1;
  }
  int v = T.size();
  T.push_back(std::make_pair(S[p], std::vector<int>(0)));
  p++;
  return v;
}
int indeterminate(std::vector<std::pair<std::string, std::vector<int>>> &T, std::vector<std::string> &S, int &p){
  if (!is_indeterminate(S[p])){
    return -1;
  }
  int v = T.size();
  T.push_back(std::make_pair(S[p], std::vector<int>(0)));
  p++;
  return v;
}
int expr(std::vector<std::pair<std::string, std::vector<int>>> &T, std::vector<std::string> &S, int &p);
int primary(std::vector<std::pair<std::string, std::vector<int>>> &T, std::vector<std::string> &S, int &p){
  if (is_variable(S[p])){
    return variable(T, S, p);
  } else if (is_number(S[p])){
    return number(T, S, p);
  } else if (is_indeterminate(S[p])){
    return indeterminate(T, S, p);
  } else if (S[p] == "("){
    p++;
    int v = expr(T, S, p);
    if (v == -1){
      return -1;
    }
    if (S[p] != ")"){
      return -1;
    }
    p++;
    return v;
  } else if (S[p] == "diff" || S[p] == "int"){
    int v = T.size();
    T.push_back(std::make_pair(S[p], std::vector<int>(1)));
    p++;
    if (S[p] != "("){
      return -1;
    }
    p++;
    int v1 = expr(T, S, p);
    if (v1 == -1){
      return -1;
    }
    T[v].second[0] = v1;
    if (S[p] == ","){
      p++;
      int v2 = indeterminate(T, S, p);
      if (v2 == -1){
        return -1;
      }
      T[v].second.push_back(v2);
    }
    if (S[p] != ")"){
      return -1;
    }
    p++;
    return v;
  } else if (S[p] == "log" || S[p] == "exp"){
    int v = T.size();
    T.push_back(std::make_pair(S[p], std::vector<int>(1)));
    p++;
    if (S[p] != "("){
      return -1;
    }
    p++;
    int v1 = expr(T, S, p);
    if (v1 == -1){
      return -1;
    }
    T[v].second[0] = v1;
    if (S[p] != ")"){
      return -1;
    }
    p++;
    return v;
  } else {
    return -1;
  }
}
int coef_primary(std::vector<std::pair<std::string, std::vector<int>>> &T, std::vector<std::string> &S, int &p){
  if (S[p] != "["){
    return primary(T, S, p);
  } else {
    p++;
    int v = T.size();
    T.push_back(std::make_pair("[]", std::vector<int>(0)));
    while (S[p] != "]"){
      if (!is_indeterminate(S[p])){
        return -1;
      }
      int v1 = indeterminate(T, S, p);
      T[v].second.push_back(v1);
      if (S[p] != "^"){
        return -1;
      }
      p++;
      int v2 = primary(T, S, p);
      if (v2 == -1){
        break;
      }
      T[v].second.push_back(v2);
    }
    p++;
    int v3 = coef_primary(T, S, p);
    if (v3 == -1){
      return -1;
    }
    T[v].second.push_back(v3);
    return v;
  }
}
int factor(std::vector<std::pair<std::string, std::vector<int>>> &T, std::vector<std::string> &S, int &p){
  if (S[p] != "-"){
    return coef_primary(T, S, p);
  } else {
    int v = T.size();
    T.push_back(std::make_pair(S[p], std::vector<int>(1)));
    p++;
    int v1 = factor(T, S, p);
    if (v1 == -1){
      return -1;
    }
    T[v].second[0] = v1;
    return v;
  }
}
int term(std::vector<std::pair<std::string, std::vector<int>>> &T, std::vector<std::string> &S, int &p){
  std::vector<int> c;
  int w = factor(T, S, p);
  if (w == -1){
    return -1;
  }
  c.push_back(w);
  std::vector<std::string> op;
  while (S[p] == "*" || S[p] == "/"){
    op.push_back(S[p]);
    p++;
    w = factor(T, S, p);
    if (w == -1){
      return -1;
    }
    c.push_back(w);
  }
  int N = c.size();
  if (N == 1){
    return c[0];
  }
  int v = T.size();
  for (int i = 0; i < N - 1; i++){
    T.push_back(std::make_pair(op[N - 2 - i], std::vector<int>(2)));
  }
  T[v + N - 2].second[0] = c[0];
  for (int i = 0; i < N - 1; i++){
    T[v + N - 2 - i].second[1] = c[i + 1];
  }
  for (int i = 0; i < N - 2; i++){
    T[v + i].second[0] = v + i + 1;
  }
  return v;
}
int expr(std::vector<std::pair<std::string, std::vector<int>>> &T, std::vector<std::string> &S, int &p){
  std::vector<int> c;
  int w = term(T, S, p);
  if (w == -1){
    return -1;
  }
  c.push_back(w);
  std::vector<std::string> op;
  while (S[p] == "+" || S[p] == "-"){
    op.push_back(S[p]);
    p++;
    w = term(T, S, p);
    if (w == -1){
      return -1;
    }
    c.push_back(w);
  }
  int N = c.size();
  if (N == 1){
    return c[0];
  }
  int v = T.size();
  for (int i = 0; i < N - 1; i++){
    T.push_back(std::make_pair(op[N - 2 - i], std::vector<int>(2)));
  }
  T[v + N - 2].second[0] = c[0];
  for (int i = 0; i < N - 1; i++){
    T[v + N - 2 - i].second[1] = c[i + 1];
  }
  for (int i = 0; i < N - 2; i++){
    T[v + i].second[0] = v + i + 1;
  }
  return v;
}
int line(std::vector<std::pair<std::string, std::vector<int>>> &T, std::vector<std::string> &S, int &p){
  if (S[p] == "print"){
    int v = T.size();
    T.push_back(std::make_pair("print", std::vector<int>(1)));
    p++;
    if (S[p] != "("){
      return -1;
    }
    p++;
    int v1 = expr(T, S, p);
    if (v1 == -1){
      return -1;
    }
    T[v].second[0] = v1;
    while (S[p] == ","){
      p++;
      int v2 = expr(T, S, p);
      if (v2 == -1){
        return -1;
      }
      T[v].second.push_back(v2);
    }
    if (S[p] != ")"){
      return -1;
    }
    p++;
    return v;
  } else {
    int v1 = variable(T, S, p);
    if (v1 == -1){
      return -1;
    }
    int v = T.size();
    T.push_back(std::make_pair("=", std::vector<int>(2)));
    T[v].second[0] = v1;
    if (S[p] != "="){
      return -1;
    }
    p++;
    int v2 = expr(T, S, p);
    if (v2 == -1){
      return -1;
    }
    T[v].second[1] = v2;
    return v;
  }
}
struct modint{
  long long x;
  modint(): x(0){
  }
  modint(long long a){
    x = a % MOD;
    if (x < 0){
      x += MOD;
    }
  }
  modint(std::string s){
    long long a = stoll(s);
    x = a % MOD;
    if (x < 0){
      x += MOD;
    }
  }
  modint operator +(modint a){
    return x + a.x >= MOD ? x + a.x - MOD : x + a.x;
  }
  modint operator +=(modint a){
    x += a.x;
    if (x >= MOD){
      x -= MOD;
    }
    return (*this);
  }
  modint operator -(modint a){
    return x - a.x < 0 ? x - a.x + MOD : x - a.x;
  }
  modint operator -=(modint a){
    x -= a.x;
    if (x < MOD){
      x += MOD;
    }
    return (*this);
  }
  modint operator *(modint a){
    return x * a.x % MOD;
  }
  modint operator *=(modint a){
    x *= a.x;
    x %= MOD;
    return (*this);
  }
  modint operator /(modint a){
    return x * a.inv().x % MOD;
  }
  modint operator /=(modint a){
    x *= a.inv().x;
    x %= MOD;
    return (*this);
  }
  bool operator ==(modint a){
    return x == a.x;
  }
  bool operator !=(modint a){
    return x != a.x;
  }
  bool is_nonnegative_integer(){
    return true;
  }
  int get_nonnegative_integer(){
    return x;
  }
  bool is_negative(){
    return false;
  }
  modint abs(){
    return (*this);
  }
  std::string to_string(){
    return std::to_string(x);
  }
  modint pow(long long n){
    modint ans = 1;
    modint a(x);
    while (n > 0){
      if (n % 2 == 1){
        ans *= a;
      }
      a *= a;
      n /= 2;
    }
    return ans;
  }
  modint inv(){
    if (x == 0){
      throw std::domain_error("zero division");
    }
    return (*this).pow(MOD - 2);
  }
};
struct degree{
  std::array<int, INDETERMINATE_COUNT> d;
  degree(){
    for (int i = 0; i < INDETERMINATE_COUNT; i++){
      d[i] = 0;
    }
  }
  degree(int x){
    d[0] = x;
  }
  degree(char x){
    for (int i = 0; i < INDETERMINATE_COUNT; i++){
      d[i] = 0;
    }
    d[get_indeterminate_id(x)] = 1;
  }
  degree increment(int x){
    degree ans;
    ans.d = d;
    ans.d[x]++;
    return ans;
  }
  degree decrement(int x){
    degree ans;
    ans.d = d;
    ans.d[x]--;
    return ans;
  }
  int& operator [](int i){
    return d[i];
  }
  degree operator +(degree x){
    degree ans;
    for (int i = 0; i < INDETERMINATE_COUNT; i++){
      ans.d[i] = d[i] + x.d[i];
    }
    return ans;
  }
  bool operator ==(degree x){
    return d == x.d;
  }
  bool operator <(degree x) const {
    return d < x.d;
  }
  bool operator <=(degree x){
    for (int i = 0; i < INDETERMINATE_COUNT; i++){
      if (d[i] > x[i]){
        return false;
      }
    }
    return true;
  }
  bool operator >=(degree x){
    for (int i = 0; i < INDETERMINATE_COUNT; i++){
      if (d[i] >= x[i]){
        return true;
      }
    }
    return false;
  }
  std::string to_string(){
    std::string ans;
    for (int i = 0; i < INDETERMINATE_COUNT; i++){
      if (d[i] > 0){
        ans += INDETERMINATE[i];
        if (d[i] > 1){
          ans += '^';
          ans += std::to_string(d[i]);
        }
      }
    }
    return ans;
  }
};
template <typename F>
struct fps{
  std::vector<std::pair<degree, F>> T;
  fps(){
  }
  fps(F x){
    if (x != 0){
      T.push_back(std::make_pair(degree(), x));
    }
  }
  fps(char c){
    T.push_back(std::make_pair(degree(c), 1));
  }
  bool empty(){
    return T.empty();
  }
  F get_number(){
    for (std::pair<degree, F> t : T){
      if (t.first == degree()){
        return t.second;
      }
    }
    return 0;
  }
  bool is_nonnegative_integer(){
    return T.empty() || T.size() == 1 && T[0].first == degree() && T[0].second.is_nonnegative_integer();
  }
  int get_nonnegative_integer(){
    if (T.empty()){
      return 0;
    } else {
      return T[0].second.get_nonnegative_integer();
    }
  }
  fps trim(degree d){
    int N = T.size();
    fps ans;
    for (int i = 0; i < N; i++){
      if (T[i].first <= d){
        ans.T.push_back(T[i]);
      }
    }
    return ans;
  }
  std::string to_string(){
    if (T.empty()){
      return "0";
    }
    int N = T.size();
    std::sort(T.begin(), T.end(), [](auto x1, auto x2){
      return x1.first < x2.first;
    });
    std::string ans;
    for (int i = 0; i < N; i++){
      if (T[i].second.is_negative()){
        ans += '-';
      } else if (i > 0){
        ans += '+';
      }
      if (T[i].second.abs() != 1 || T[i].first == degree()){
        ans += T[i].second.abs().to_string();
      }
      ans += T[i].first.to_string();
    }
    return ans;
  }
};
template <typename F>
fps<F> add(fps<F> f, fps<F> g){
  std::map<degree, F> mp;
  for (std::pair<degree, F> a : f.T){
    mp[a.first] += a.second;
  }
  for (std::pair<degree, F> b : g.T){
    mp[b.first] += b.second;
  }
  fps<F> ans;
  for (std::pair<degree, F> t : mp){
    if (t.second != 0){
      ans.T.push_back(t);
    }
  }
  return ans;
}
template <typename F>
fps<F> negate(fps<F> f){
  for (std::pair<degree, F> &a : f.T){
    a.second *= -1;
  }
  return f;
}
template <typename F>
fps<F> subtract(fps<F> f, fps<F> g){
  return add(f, negate(g));
}
template <typename F>
fps<F> multiply(fps<F> f, fps<F> g, degree d){
  std::map<degree, F> mp;
  for (std::pair<degree, F> a : f.T){
    for (std::pair<degree, F> b : g.T){
      if (a.first + b.first <= d){
        mp[a.first + b.first] += a.second * b.second;
      }
    }
  }
  fps<F> ans;
  for (std::pair<degree, F> t : mp){
    if (t.second != 0){
      ans.T.push_back(t);
    }
  }
  return ans;
}
template <typename F>
fps<F> differentiate(fps<F> f, int x){
  fps<F> ans;
  for (std::pair<degree, F> t : f.T){
    if (t.first[x] != 0){
      ans.T.push_back(std::make_pair(t.first.decrement(x), t.second * t.first.d[x]));
    }
  }
  return ans;
}
template <typename F>
fps<F> integrate(fps<F> f, int x, degree d){
  fps<F> ans;
  for (std::pair<degree, F> t : f.T){
    if (t.first.increment(x) <= d){
      ans.T.push_back(std::make_pair(t.first.increment(x), t.second / (t.first.d[x] + 1)));
    }
  }
  return ans;
}
template <typename F>
fps<F> coefficient(fps<F> f, degree d){
  fps<F> ans;
  for (std::pair<degree, F> t : f.T){
    bool ok = true;
    for (int i = 0; i < INDETERMINATE_COUNT; i++){
      if (t.first[i] != d[i] && d[i] != INF){
        ok = false;
      }
    }
    if (ok){
      degree d2 = t.first;
      for (int i = 0; i < INDETERMINATE_COUNT; i++){
        if (d[i] != -1){
          d2[i] = 0;
        }
      }
      ans.T.push_back(std::make_pair(d2, t.second));
    }
  }
  return ans;
}
template <typename F>
fps<F> inverse(fps<F> f, degree d){
  F c = coefficient(f, degree()).get_number();
  if (c == 0){
    throw std::domain_error("constant term of denominator of / is zero");
  }
  f = multiply(f, fps<F>(F(1) / c), d);
  f = subtract(f, fps<F>(F(1)));
  if (f.empty()){
    return fps<F>(F(1) / c);
  }
  f = multiply(f, fps<F>(F(-1)), d);
  fps<F> f2 = f;
  fps<F> ans = 1;
  while (true){
    ans = add(ans, f2);
    f2 = multiply(f2, f, d);
    if (f2.empty()){
      break;
    }
  }
  ans = multiply(ans, fps<F>(F(1) / c), d);
  return ans;
}
template <typename F>
fps<F> divide(fps<F> f, fps<F> g, degree d){
  return multiply(f, inverse(g, d), d);
}
template <typename F>
fps<F> log(fps<F> f, degree d){
  if (coefficient(f, degree()).get_number() != 1){
    throw std::domain_error("constant term of argument of log is not 1");
  }
  f = subtract(f, fps<F>(F(1)));
  f = multiply(f, fps<F>(F(-1)), d);
  fps<F> f2 = f;
  fps<F> ans;
  int n = 1;
  while (true){
    ans = add(ans, multiply(f2, fps<F>(F(-1) / n), d));
    f2 = multiply(f2, f, d);
    if (f2.empty()){
      break;
    }
    n++;
  }
  return ans;
}
template <typename F>
fps<F> exp(fps<F> f, degree d){
  if (coefficient(f, degree()).get_number() != 0){
    throw std::domain_error("constant term of argument of log is not 1");
  }
  fps<F> f2 = f;
  fps<F> ans = 1;
  int n = 1;
  while (true){
    ans = add(ans, f2);
    n++;
    f2 = multiply(f2, f, d);
    if (f2.empty()){
      break;
    }
    f2 = multiply(f2, fps<F>(F(1) / n), d);
  }
  return ans;
}
template <typename F>
std::string solve(std::string S){
  std::vector<std::string> A = split(S);
  int N = A.size();
  std::vector<std::vector<std::string>> B(N);
  for (int i = 0; i < N; i++){
    if (!A[i].empty() && !(A[i].size() >= 2 && A[i].substr(0, 2) == "//")){
      std::vector<std::string> T = lex(A[i]);
      B[i] = T;
      if (B[i][0] == "error"){
        return "Line " + std::to_string(i + 1) + ": lexical error\n";
      }
    }
  }
  std::vector<std::vector<std::pair<std::string, std::vector<int>>>> T(N);
  std::vector<int> r(N);
  for (int i = 0; i < N; i++){
    if (!B[i].empty()){
      B[i].push_back("$");
      int p = 0;
      int res = line(T[i], B[i], p);
      if (res == -1){
        return "Line " + std::to_string(i + 1) + ": parse error\n";
      }
      if (T[i][0].first != "print"){
        r[i] = T[i][1].second[1];
      }
      B[i].pop_back();
      if (p != B[i].size()){
        return "Line " + std::to_string(i + 1) + ": parse error\n";
      }
    }
  }
  std::map<char, int> mp;
  for (int i = 0; i < N; i++){
    if (!B[i].empty()){
      int V = T[i].size();
      for (int j = 1; j < V; j++){
        if (is_variable(T[i][j].first)){
          char c = T[i][j].first[0];
          if (mp.count(c) == 0){
            return "Line " + std::to_string(i + 1) + ": undefined variable " + c + "\n";
          }
          T[i][j].first = "var" + std::to_string(mp[c]);
        }
      }
      if (T[i][0].first != "print"){
        mp[T[i][0].first[0]] = i;
      }
    }
  }
  std::vector<degree> cdeg(N);
  for (int i = 0; i < N; i++){
    for (int j = 0; j < INDETERMINATE_COUNT; j++){
      cdeg[i].d[j] = -1;
    }
  }
  std::vector<fps<F>> var(N);
  std::string ans;
  auto dfs = [&](auto get, auto dfs, int l, int v, degree d){
    if (is_number(T[l][v].first)){
      return fps<F>(T[l][v].first);
    }
    if (T[l][v].first.size() >= 3 && T[l][v].first.substr(0, 3) == "var"){
      return get(get, dfs, std::stoi(T[l][v].first.substr(3)), d);
    }
    if (is_indeterminate(T[l][v].first)){
      if (d[get_indeterminate_id(T[l][v].first)] == 0){
        return fps<F>();
      } else {
        return fps<F>(T[l][v].first[0]);
      }
    }
    if (T[l][v].first == "[]"){
      int cnt = T[l][v].second.size();
      degree d;
      for (int i = 0; i < INDETERMINATE_COUNT; i++){
        d[i] = INF;
      }
      for (int i = 0; i < cnt - 1; i += 2){
        fps<F> t = dfs(get, dfs, l, T[l][v].second[i + 1], {INF});
        if (!t.is_nonnegative_integer()){
          throw std::domain_error("[] exponent is not number");
        }
        d[get_indeterminate_id(T[l][T[l][v].second[i]].first)] = t.get_nonnegative_integer();
      }
      return coefficient(dfs(get, dfs, l, T[l][v].second[cnt - 1], d), d);
    }
    if (T[l][v].first == "+"){
      return add(dfs(get, dfs, l, T[l][v].second[0], d), dfs(get, dfs, l, T[l][v].second[1], d));
    }
    if (T[l][v].first == "-" && T[l][v].second.size() == 1){
      return negate(dfs(get, dfs, l, T[l][v].second[0], d));
    }
    if (T[l][v].first == "-" && T[l][v].second.size() == 2){
      return subtract(dfs(get, dfs, l, T[l][v].second[0], d), dfs(get, dfs, l, T[l][v].second[1], d));
    }
    if (T[l][v].first == "*"){
      return multiply(dfs(get, dfs, l, T[l][v].second[0], d), dfs(get, dfs, l, T[l][v].second[1], d), d);
    }
    if (T[l][v].first == "/"){
      return divide(dfs(get, dfs, l, T[l][v].second[0], d), dfs(get, dfs, l, T[l][v].second[1], d), d);
    }
    if (T[l][v].first == "diff"){
      if (T[l][v].second.size() == 1){
        return differentiate(dfs(get, dfs, l, T[l][v].second[0], d.increment(0)), 0);
      } else {
        int x = get_indeterminate_id(T[l][T[l][v].second[1]].first);
        return differentiate(dfs(get, dfs, l, T[l][v].second[0], d.increment(x)), x);
      }
    }
    if (T[l][v].first == "int"){
      if (T[l][v].second.size() == 1){
        return integrate(dfs(get, dfs, l, T[l][v].second[0], d), 0, d);
      } else {
        return integrate(dfs(get, dfs, l, T[l][v].second[0], d), get_indeterminate_id(T[l][T[l][v].second[1]].first), d);
      }
    }
    if (T[l][v].first == "log"){
      return log(dfs(get, dfs, l, T[l][v].second[0], d), d);
    }
    if (T[l][v].first == "exp"){
      return exp(dfs(get, dfs, l, T[l][v].second[0], d), d);
    }
  };
  auto get = [&](auto get, auto dfs, int l, degree d){
    if (cdeg[l] >= d){
      return var[l].trim(d);
    } else {
      var[l] = dfs(get, dfs, l, r[l], d);
      return var[l];
    }
  };
  for (int i = 0; i < N; i++){
    if (!B[i].empty()){
      if (T[i][0].first == "print"){
        int cnt = T[i][0].second.size();
        degree d;
        for (int j = 1; j < cnt; j++){
          d[j - 1] = std::stoi(T[i][T[i][0].second[j]].first);
        }
        std::string res = dfs(get, dfs, i, T[i][0].second[0], d).to_string();
        if (res.size() >= 5 && res.substr(0, 5) == "error"){
          return res.substr(5);
        }
        ans += res;
        ans += '\n';
      }
    }
  }
  return ans;
}
extern "C" void EMSCRIPTEN_KEEPALIVE calculate(){
  std::string S = getElementValue("input");
  setElementInnerText("hoge", solve<modint>(S));
}