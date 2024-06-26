#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#include <stddef.h>

typedef long Align; // 用于内存块对齐的类型

union header { // 内存块的头部结构
  struct {
    union header *ptr;  // 指向下一个空闲内存块的指针
    uint size;          // 当前内存块的大小（单位是sizeof(header)）
  } s;
  Align x;              // 强制对齐占位符
};

typedef union header Header;

static Header base;    // 空闲列表的初始块
static Header *freep = 0; // 空闲列表的起始指针

void free(void *ap);
void coalesce();
static Header *morecore(uint nu);

void free(void *ap) {
  Header *bp, *p;

  bp = (Header *)ap - 1; // 获取指向块头部的指针
  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) 
      break;
  }
  // if(bp + bp->s.size == p->s.ptr){
  //   bp->s.size += p->s.ptr->s.size;
  //   bp->s.ptr = p->s.ptr->s.ptr;
  // } else
  //   bp->s.ptr = p->s.ptr;
  // if(p + p->s.size == bp){
  //   p->s.size += bp->s.size;
  //   p->s.ptr = bp->s.ptr;
  // } else
  //   p->s.ptr = bp;
   freep = p;

}

void reset(void *ap) {
  Header *bp, *p;

  bp = (Header *)ap - 1; 
  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) 
      break;
  }

  bp->s.ptr = p->s.ptr;
  p->s.ptr = bp;
  if (bp + bp->s.size == bp->s.ptr) {
    bp->s.size += bp->s.ptr->s.size;
    bp->s.ptr = bp->s.ptr->s.ptr;
  }
  if (p + p->s.size == bp) {
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  }
  freep = p;
}

void *malloc(uint nbytes) {
  Header *p, *prevp;
  Header *bestp = NULL, *bestprevp = NULL;
  uint nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
  int tried_to_coalesce = 0;

  if ((prevp = freep) == NULL) { 
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  while (1) {
    for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
      if (p->s.size >= nunits) {
        if (bestp == NULL || p->s.size < bestp->s.size) {
          bestp = p;
          bestprevp = prevp;
        }
      }
      if (p == freep) {
        if (bestp != NULL) {
          p = bestp;
          prevp = bestprevp;
          if (p->s.size == nunits) {
            prevp->s.ptr = p->s.ptr;
          } else {
            p->s.size -= nunits;
            p += p->s.size;
            p->s.size = nunits;
          }
          freep = prevp;
          return (void *)(p + 1);
        }
        if (tried_to_coalesce) {
          printf("请求更多内存\n");
          p = morecore(nunits);
          if (p == 0) {
            return 0; 
          }
          tried_to_coalesce = 0;
          break;
        }
        coalesce(); 
        tried_to_coalesce = 1; 
      }
    }
  }
}


static Header *morecore(uint nu) {
  char *cp;
  Header *up;

  if (nu < 4096)
    nu = 4096;
  cp = sbrk(nu * sizeof(Header));
  if (cp == (char *) -1)
    return 0;
  up = (Header *) cp;
  up->s.size = nu;
  free((void *)(up + 1));
  return freep;
}

void coalesce() {

  printf("正在合并空闲块\n");

  Header *p, *next;

  
  for (p = freep; p != NULL && p->s.ptr != freep; p = p->s.ptr) {
    next = p->s.ptr;
    if ((char *)p + p->s.size == (char *)next) {
      p->s.size += next->s.size;
      p->s.ptr = next->s.ptr;
    }
  }
  printf("合并成功\n");

}

// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"
// #include "kernel/param.h"

// // Memory allocator by Kernighan and Ritchie,
// // The C programming Language, 2nd ed.  Section 8.7.

// typedef long Align;

// union header {
//   struct {
//     union header *ptr;
//     uint size;
//   } s;
//   Align x;
// };

// typedef union header Header;

// static Header base;
// static Header *freep;

// void
// free(void *ap)
// {
//   Header *bp, *p;

//   bp = (Header*)ap - 1;
//   for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
//     if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
//       break;
//   if(bp + bp->s.size == p->s.ptr){
//     bp->s.size += p->s.ptr->s.size;
//     bp->s.ptr = p->s.ptr->s.ptr;
//   } else
//     bp->s.ptr = p->s.ptr;
//   if(p + p->s.size == bp){
//     p->s.size += bp->s.size;
//     p->s.ptr = bp->s.ptr;
//   } else
//     p->s.ptr = bp;
//   freep = p;
// }

// static Header*
// morecore(uint nu)
// {
//   char *p;
//   Header *hp;

//   if(nu < 4096)
//     nu = 4096;
//   p = sbrk(nu * sizeof(Header));
//   if(p == (char*)-1)
//     return 0;
//   hp = (Header*)p;
//   hp->s.size = nu;
//   free((void*)(hp + 1));
//   return freep;
// }

// void*
// malloc(uint nbytes)
// {
//   Header *p, *prevp;
//   uint nunits;

//   nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
//   if((prevp = freep) == 0){
//     base.s.ptr = freep = prevp = &base;
//     base.s.size = 0;
//   }
//   for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
//     if(p->s.size >= nunits){
//       if(p->s.size == nunits)
//         prevp->s.ptr = p->s.ptr;
//       else {
//         p->s.size -= nunits;
//         p += p->s.size;
//         p->s.size = nunits;
//       }
//       freep = prevp;
//       return (void*)(p + 1);
//     }
//     if(p == freep)
//       if((p = morecore(nunits)) == 0)
//         return 0;
//   }
// }
