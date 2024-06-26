#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main()
{
    void *ptrs[100];
    uint start_time, end_time, total_time;

    start_time = uptime(); // 记录开始时间
    for (int i = 0; i < 100; i++){
        ptrs[i] = malloc(10); 
        printf("Allocated ptr[%d]: %p\n", i, ptrs[i]);
    }
    for (int i = 1; i < 100; i += 2){
        free(ptrs[i]);
        printf("Freed ptr[%d]: %p\n", i, ptrs[i]);
    }
    for (int i = 0; i < 100; i += 2){
        free(ptrs[i]);
        printf("Freed ptr[%d]: %p\n", i, ptrs[i]);
    }
    for (int i = 0; i < 100; i++){
        ptrs[i] = malloc(101); 
        printf("Allocated ptr[%d]: %p\n", i, ptrs[i]);
    }
    end_time = uptime(); // 记录结束时间
    total_time = end_time - start_time; // 计算总时间
    printf("Total time: %d ms\n", total_time);
    return 0;
}
