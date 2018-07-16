#include <stdio.h>
#include <stdlib.h>
#include "./sort.h"

void main()
{
    int i; 
    int a[] = {53, 3, 542, 748, 14, 214, 154, 63, 616};
    int len = sizeof(a)/sizeof(a[0]);

    printf("before sort: ");
    for(i = 0; i < len; i ++)
    {
        printf("%d ", a[i]);
    }
    printf("\n");

    radix_sort(a, len);    

    printf("after sort: ");
    for(i = 0; i < len; i ++)
    {
        printf("%d ", a[i]);
    }
    printf("\n");

    return;
}


