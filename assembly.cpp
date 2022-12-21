#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

string emptystr = "empty";

// 四元组结构体
struct Quadruple {
  string s0, s1, s2, s3;
};

// 语句类型和汇编语言的对应表
map<string, string> asmMap = {
    {"j==", "JE"},   {"%", ""},    {"^", ""},    {"=", "mov"},   {"+", "add"},
    {"-", "sub"},    {"*", "mul"}, {"/", "div"}, {">=", "GE"},   {"<=", "LE"},
    {">", "GT"},     {"<", "LT"},  {"==", ""},   {"!=", "ne"},   {"||", "and"},
    {"&&", "or"},    {"!", "not"}, {"j", "jmp"}, {"ret", "RET"}, {"arg", "arg"},
    {"call", "call"}};

bool isNumber(string str) {
  return str.find_first_not_of("0123456789") == string::npos;
}

template <typename T>
int indexV(vector<T> v, T t) {
  if (find(v.begin(), v.end(), t) == v.end())
    return -1;
  else
    return find(v.begin(), v.end(), t) - v.begin();
}

template <typename T>
bool isInVector(vector<T> v, T t) {
  return find(v.begin(), v.end(), t) != v.end();
}

int main() {
  vector<Quadruple> quad;  // 四元组向量，记录多个四元组

  // 1 打开Innercode文件
  ifstream ifile("Innercode");  // 读名为Innercode的文件
  if (!ifile) {
    cout << "找不到名为Innercode的文件！" << endl;
    return 1;  // 非正常退出
  }

  // 2 根据Innercode创建四元组向量
  string line;  // line用于读取Innercode中的每一行
  while (getline(ifile, line)) {
    // cout << line << endl;
    // 2.1 读入一行，替换此行所有换行符（这个方法是在algorithm头文件里）
    line.erase(remove(line.begin(), line.end(), '\n'), line.end());

    // 2.2 按空格分割此行的内容，放入临时变量wds里
    vector<string> wds;        // wds=words，储存语句的内容
    stringstream input(line);  // 读取str到字符串流中
    string temp;
    while (getline(input, temp, ' ')) {
      wds.push_back(temp);
    }

    // 2.3 判断类型并插入四元组向量
    if (wds[1] == "if")  // 若相等compare返回0
      quad.push_back({"j==", wds[2], "1", wds[4]});
    else if (wds[1] == "goto")
      quad.push_back({"j", "_", "_", wds[2]});
    else if (wds[1] == "arg" || wds[1] == "call")
      quad.push_back({wds[1], "_", "_", wds[2]});
    else if (wds[1] == "return")
      quad.push_back({"ret", "_", "_", wds[2]});
    else if (wds.size() > 3 && wds[3] == "!")
      quad.push_back({"!", wds[4], "_", wds[1]});
    else if (wds[1][0] == 't')
      quad.push_back({wds[4], wds[3], wds[5], wds[1]});
    else
      quad.push_back({wds[2], wds[3], "_", wds[1]});
  }

  // ~1 关闭Innercode文件
  ifile.close();

  // 3 为所有转移（j==与j）记录CODE位置
  int n = 0;
  set<int> t;
  for (auto element : quad) {
    if (element.s0[0] == 'j') {  // 如果是条件转移"j=="或无条件转移"j"
      t.insert(n);
      t.insert(stoi(element.s3) - 1);
    }
    ++n;
  }
  int m = 0;
  map<int, string> quad_in;
  for (int it : t) {
    quad_in[it] = "CODE" + to_string(++m);
  }

  // 4 创建assembly.asm文件，写入固定内容[1]
  ofstream ofile("assembly.asm", fstream::out);
  string tab = "        ";  // 默认的缩进（8空格）
  string inx = "[rbp-";
  ofile << "extern printf, scanf" << endl
        << "global main" << endl
        << "main:" << endl
        << tab + "enter 100, 0" << endl;

  // 5 写入实际的汇编内容
  vector<string> data;
  for (int i = 0; i < 100; ++i) data.push_back(emptystr);
  vector<string> judge = {">", "<", "==", ">=", "<=", "!="};
  for (int i = 0; i < quad.size(); ++i) {
    if (quad_in.count(i)) ofile << quad_in[i] + ":" << endl;
    if (quad[i].s0 == "j==") {  // 条件转移，则需要记录条件
      if (isInVector(judge, quad[i - 1].s0)) {
        if (isNumber(quad[i - 1].s1))
          ofile << tab + "mov eax," + quad[i - 1].s1 << endl;
        else
          ofile << tab + "mov eax," + inx
                << to_string(indexV(data, quad[i - 1].s1) * 4 + 4) + "]"
                << endl;
        if (isNumber(quad[i - 1].s2))
          ofile << tab + "mov ebx," + quad[i - 1].s2 << endl;
        else
          ofile << tab + "mov ebx," + inx
                << to_string(indexV(data, quad[i - 1].s2) * 4 + 4) + "]"
                << endl;
        ofile << tab + "cmp eax,ebx" << endl;
        if (quad[i - 1].s0 == ">")
          ofile << tab + "" + "ja" + " " + quad_in[stoi(quad[i].s3) - 1]
                << endl;
        else if (quad[i - 1].s0 == "<")
          ofile << tab + "" + "jb" + " " + quad_in[stoi(quad[i].s3) - 1]
                << endl;
        else if (quad[i - 1].s0 == ">=")
          ofile << tab + "" + "jae" + " " + quad_in[stoi(quad[i].s3) - 1]
                << endl;
        else if (quad[i - 1].s0 == "<=")
          ofile << tab + "" + "jbe" + " " + quad_in[stoi(quad[i].s3) - 1]
                << endl;
        else if (quad[i - 1].s0 == "==")
          ofile << tab + "" + "je" + " " + quad_in[stoi(quad[i].s3) - 1]
                << endl;
        else if (quad[i - 1].s0 == "!=")
          ofile << tab + "" + "jne" + " " + quad_in[stoi(quad[i].s3) - 1]
                << endl;
      } else {
        if (isNumber(quad[i].s2))
          ofile << tab + "mov eax," + quad[i].s2;
        else
          ofile << tab + "mov eax," + inx
                << to_string(indexV(data, quad[i].s2) * 4 + 4) + "]" << endl;
        ofile << tab + "cmp eax,1" << endl
              << tab + "je " + quad_in[stoi(quad[i].s3) - 1] << endl
              << endl;
      }
    } else if (quad[i].s0 == "=") {  // 赋值
      data[indexV(data, emptystr)] = quad[i].s3;
      if (isNumber(quad[i].s1))
        ofile << tab + "mov eax," + quad[i].s1 << endl;
      else
        ofile << tab + "mov eax," + inx
              << to_string(indexV(data, quad[i].s1) * 4 + 4) + "]" << endl;
      ofile << tab + "mov " + inx
            << to_string(indexV(data, quad[i].s3) * 4 + 4) + "],eax" << endl;
    } else if (quad[i].s0 == "j") {  // 转移
      ofile << tab + "jmp " + quad_in[stoi(quad[i].s3) - 1] << endl;
    } else if (quad[i].s0 == "ret") {  // 返回
      ofile << tab + "leave" << endl << tab + "ret" << endl;
    } else if (quad[i].s0 == "arg" || quad[i].s0 == "call") {  // 函数
      if (quad[i].s0 == "arg") {
        ofile << tab + "sub rsp, 8" << endl
              << tab + "mov rsi," + inx
              << to_string(indexV(data, quad[i].s3) * 4 + 4) + "]" << endl;
      } else if (quad[i].s0 == "call" && quad[i].s3 == "output") {
        ofile << tab + "mov rdi, out_format" << endl
              << tab + "xor rax, rax" << endl
              << tab + "call printf" << endl
              << tab + "xor rax, rax" << endl
              << tab + "add rsp, 8" << endl;
      } else if (quad[i].s0 == "call" && quad[i].s3 == "input") {
        ofile << tab + "mov rsi, number" << endl
              << tab + "mov rdi, in_format" << endl
              << tab + "xor rax, rax" << endl
              << tab + "call scanf" << endl
              << tab + "mov rbx, [number]" << endl
              << tab + "add rsp, 8" << endl;
      }
    } else {  // 运算
      if (isNumber(quad[i].s1))
        ofile << tab + "mov eax," + quad[i].s1 << endl;
      else if (quad[i].s1 != "_")
        ofile << tab + "mov eax," + inx
              << to_string(indexV(data, quad[i].s1) * 4 + 4) + "]" << endl;
      if (isNumber(quad[i].s2))
        ofile << tab + "mov ebx," + quad[i].s2 << endl;
      else if (quad[i].s2 != "_")
        ofile << tab + "mov ebx," + inx
              << to_string(indexV(data, quad[i].s2) * 4 + 4) + "]" << endl;
      if (quad[i].s0 == "+" || quad[i].s0 == "-")
        ofile << tab + asmMap[quad[i].s0] + " eax,ebx" << endl;
      else if (quad[i].s0 == "*")
        ofile << tab + "mul ebx" << endl;
      else if (quad[i].s0 == "/")
        ofile << tab + "div ebx" << endl;
      else if (quad[i].s0 == "%") {
        ofile << tab + "div ebx" << endl;
        ofile << tab + "mov eax,edx" << endl;
      } else if (quad[i].s0 == "^")
        ofile << tab + "times " + quad[i].s2 + " mul eax" << endl;
      else if (quad[i].s0 == "&&" || quad[i].s0 == "||")
        ofile << tab + asmMap[quad[i].s0] + " eax,ebx" << endl;
      else if (quad[i].s0 == "!")
        ofile << tab + asmMap[quad[i].s0] + " eax" << endl;
      else if (isInVector(judge, quad[i].s0))
        ;
      else
        ofile << tab + "----运算符未定义----" << endl;
      data[indexV(data, emptystr)] = quad[i].s3;
      ofile << tab + "mov " + inx
            << to_string(indexV(data, quad[i].s3) * 4 + 4) + "],eax" << endl;
    }
  }

  // ~4 写入固定内容[2]，关闭文件
  ofile << endl
        << "section .data" << endl
        << tab + "out_format: db \"%#d\", 10, 0" << endl
        << tab + "in_format: db \"%d\", 0" << endl
        << "section .bss" << endl
        << tab + "number resb 4" << endl;
  ofile.close();

  return 0;
}
