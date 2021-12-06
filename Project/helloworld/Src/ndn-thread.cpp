#include "ndn-thread.hpp"
#include "Src/ndnputsegments.hpp"

void NDNThread::run()
{
    ndn::segment::Put Putsegment;   //定义上传文件类的对象
    Putsegment.PutSegment();   //调用上传文件类的上传文件块函数
}
