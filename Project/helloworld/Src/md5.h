/*
基本处理思想：初始为128个bit 信息，将128bit 初始信息与分割后的512bit 数据经过4组16次hash循环算法，得
到128bit 的新信息，依次循环，直到与最后一组512 bit的数据hash算法后，就得到的整个文件的MD5值。

MD5主循环有四轮，每轮循环都很相似。第一轮进行16次操作。每次操作对a、b、c和d中的其中三个作一次非
线性函数运算，然后将所得结果加上第四个变量，文本的一个子分组和一个常数。再将所得结果向左环移一个不
定的数，并加上a、b、c或d中之一。最后用该结果取代a、b、c或d中之一。

四个非线性函数：
F(X,Y,Z) =(X&Y)|((~X)&Z)
G(X,Y,Z) =(X&Z)|(Y&(~Z))
H(X,Y,Z) =X^Y^Z
I(X,Y,Z)=Y^(X|(~Z))

*/

#ifndef MD5_H  
#define MD5_H  
  
#include <string>  
#include <fstream>  
  
/* Type define */  
typedef unsigned char byte;  
typedef unsigned long ulong;  
  
using std::string;  
using std::ifstream;  
  
/* MD5 declaration. */  
class MD5 {  
public:  
    MD5();  
    MD5(const void *input, size_t length);  
    MD5(const string &str);  
    MD5(ifstream &in);  
    void update(const void *input, size_t length);  
    void update(const string &str);  
    void update(ifstream &in);  
    const byte* digest();  
    string toString();  
    void reset();  
private:  
    void update(const byte *input, size_t length);  
    void final();  
    void transform(const byte block[64]);  
    void encode(const ulong *input, byte *output, size_t length);  
    void decode(const byte *input, ulong *output, size_t length);  
    string bytesToHexString(const byte *input, size_t length);  
  
    /* class uncopyable */  
    MD5(const MD5&);  
    MD5& operator=(const MD5&);  
private:  
    ulong _state[4];    /* state (ABCD) */   //四个32位的链接变量的整数参数
    ulong _count[2];    /* number of bits, modulo 2^64 (low-order word first) */  
    byte _buffer[64];  /* input buffer */  
    byte _digest[16];  /* message digest */  
    bool _finished;    /* calculate finished ? */  
  
    static const byte PADDING[64];  /* padding for calculate */  
    static const char HEX[16];  
    static const size_t BUFFER_SIZE = 1024; 
 
    //static const size_tlength = 1024; 
};  
  
#endif/*MD5_H*/
