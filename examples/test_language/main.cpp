#include<sys/time.h>
#include<random>
#include<sys/socket.h>
#include<string.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
void test_network();
void test_disk();
void test_opt();
void test_mem();
int main(int argc,char *argv[])
{
    test_disk();
    test_opt();
    test_mem();
    test_network();
    return 0;
}
void test_network()
{
    timeval time_now;
    gettimeofday(&time_now,nullptr);
    int test_cnt=10000000;
    auto ptr=getenv("NETWORK_CNT");
    if(ptr)test_cnt=std::stoi(ptr);
    fprintf(stderr,"test network start NETWORK_CNT=%d\r\n",test_cnt);
    int fd=socket(AF_INET,SOCK_DGRAM,0);;
    int fd2=socket(AF_INET,SOCK_DGRAM,0);
    {
        struct sockaddr_in addr;
        bzero(&addr,sizeof (addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(50000);
        ::bind(fd2, (struct sockaddr*)&addr, sizeof addr);
    }
    std::random_device rd;
    uint8_t data[1024]={0};
    memset(data,0,1024);
    uint8_t recv_buf[1024];
    struct sockaddr_in sockaddr;
    bzero(&sockaddr,sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(50000);
    //IP格式转换
    inet_pton(AF_INET,"127.0.0.1",&sockaddr.sin_addr);
    int exec_cnt=test_cnt;
    while(exec_cnt-->0)
    {
        if(::sendto(fd,data,1024,0,(struct sockaddr *)&sockaddr,sizeof(sockaddr))<=0)
        {
            perror("send error!");
        }
        if(::read(fd2,recv_buf,1024)<=0)
        {
            perror("recv error!");
        }
    }
    timeval time_finsh;
    gettimeofday(&time_finsh,nullptr);
    int64_t cost_ms=(time_finsh.tv_sec-time_now.tv_sec)*1000+(time_finsh.tv_usec-time_now.tv_usec)/1000;
    fprintf(stderr,"test network finish NETWORK_CNT=%d  cost [%ld]ms\r\n",test_cnt,cost_ms);
}

void test_disk()
{
    timeval time_now;
    gettimeofday(&time_now,nullptr);
    int test_cnt=10000;
    auto ptr=getenv("DISK_CNT");
    if(ptr)test_cnt=std::stoi(ptr);
    fprintf(stderr,"test disk start DISK_CNT=%d\r\n",test_cnt);
    FILE *fp=fopen("test.wb","wb+");
    if(!fp)
    {
        perror("open test_file!");
        return;
    }
    uint8_t data[1024]={0};
    memset(data,0,1024);
    int exec_cnt=test_cnt;
    while(exec_cnt-->0)
    {
        fwrite(data,1024,1,fp);
    }
    fclose(fp);
    remove("test.wb");
    timeval time_finsh;
    gettimeofday(&time_finsh,nullptr);
    int64_t cost_ms=(time_finsh.tv_sec-time_now.tv_sec)*1000+(time_finsh.tv_usec-time_now.tv_usec)/1000;
    fprintf(stderr,"test disk finish DISK_CNT=%d  cost [%ld]ms\r\n",test_cnt,cost_ms);
}

void test_opt()
{
    timeval time_now;
    gettimeofday(&time_now,nullptr);
    int test_cnt=10000000;
    auto ptr=getenv("OPERATION_CNT");
    if(ptr)test_cnt=std::stoi(ptr);
    fprintf(stderr,"test operation start OPERATION_CNT=%d\r\n",test_cnt);
    std::random_device rd;
    float result=0.0;
    int exec_cnt=test_cnt;
    while(exec_cnt-->0)
    {
        float option1=rd()/13.0;
        float option2=rd()/17.0;
        result+=option1*option2;
    }
    timeval time_finsh;
    gettimeofday(&time_finsh,nullptr);
    int64_t cost_ms=(time_finsh.tv_sec-time_now.tv_sec)*1000+(time_finsh.tv_usec-time_now.tv_usec)/1000;
    fprintf(stderr,"test operation finish OPERATION_CNT=%d result[%f] cost [%ld]ms\r\n",test_cnt,result,cost_ms);
}

void test_mem()
{
    timeval time_now;
    gettimeofday(&time_now,nullptr);
    int test_cnt=10000000;
    auto ptr=getenv("MEM_CNT");
    if(ptr)test_cnt=std::stoi(ptr);
    fprintf(stderr,"test mem malloc start MEM_CNT=%d\r\n",test_cnt);
    std::random_device rd;
    auto gap=test_cnt/10;
    int exec_cnt=test_cnt;
    while(exec_cnt-->0)
    {
        auto ptr=malloc(4096);
        if(exec_cnt%gap==0)fprintf(stderr,"gap=%d  ptr=%p\r\n",gap,ptr);
        free(ptr);
    }
    timeval time_finsh;
    gettimeofday(&time_finsh,nullptr);
    int64_t cost_ms=(time_finsh.tv_sec-time_now.tv_sec)*1000+(time_finsh.tv_usec-time_now.tv_usec)/1000;
    fprintf(stderr,"test mem malloc finish MEM_CNT=%d  cost [%ld]ms\r\n",test_cnt,cost_ms);
}