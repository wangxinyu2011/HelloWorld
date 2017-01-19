/************************************************************
Copyright (C), 2016, Leon, All Rights Reserved.
FileName: bfs.c
Description:
Author: Leon
Version: 1.0
Date:
Function:

History:
<author>    <time>  <version>   <description>
 Leon

************************************************************/

/*****************************************************************
 3X3的有9!=362880种状态，广搜最坏要搜索保存这么多中间状态。
 用一个int保存一种状态，需要1417kB空间。
 读入地图，初始化为二维数组。移动一次，刷新二维数组。保存状态
*****************************************************************/

/*
运行时间：071534628初态用例，会搜索120647次， user:0.27s, sys:0.09s
使用-O3编译，速度快不少 0.04 0.05
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true    1
#define false   0
#define QUEUE_SIZE  (362880 + 1)
#define STATES_MAX  (362880 + 1) // 9! + 1
#define HEIGHT 3
#define WIDTH  3

typedef int bool;

typedef struct point
{
    int x;
    int y;
} point_t;

#pragma pack(1)
typedef struct location
{
    int state;
    struct location *parent;
    int deep;
    char visited;
}location_t;
#pragma pack()


typedef struct queue
{
    int data[QUEUE_SIZE];
    int head;   //队列头下标，从0开始
    int tail;
} queue_t;

queue_t gQ;
location_t map[STATES_MAX] = {0};


int queue_push(struct queue *q, int state)
{
    if (!q)
        return -1;

    if (q->tail >= QUEUE_SIZE - 1)
    {
        fprintf(stderr, "queue size limit\n");
        return -1;
    }

    q->data[q->tail] = state;
    // 计算新的队尾位置
    q->tail = (q->tail + 1);

    return 0;
}

int queue_empty(struct queue *q)
{
    if (!q)
        return -1;

    return (q->head == q->tail);
}

int queue_pop(struct queue *q)
{
    if (!q)
        return 0;

    if (queue_empty(q))
        return 0;

    int result = q->data[q->head];
    //计算新的队首位置
    q->head += 1;
    
    return result;
}

/* 取得队头的节点 */
int queue_front(struct queue *q)
{
    if (queue_empty(q))
        return 0;

    return (q->data[q->head]);
}


int is_valid(point_t *point)
{
    if (point && point->x >= 0 && point->x <= HEIGHT - 1 
              && point->y >= 0 && point->y <= WIDTH - 1)
        return 1;
    return 0;
}

/* 返回*/
point_t find_blank_block(int state)
{
    int i = 8;
    point_t p = {0, 0};

    while (state % 10)
    {
        state /= 10;
        i--;
    }
    p.x = i / 3;
    p.y = i % 3;
    return p;
}

//返回静态数组
int *state_convert_array(int state)
{
    static int array[9] = {0};
    int i = 8;

    memset(&array, 0x0, sizeof(array));
    for (i = 8; i >= 0; i--)
    {
        array[i] = state % 10;
        state /= 10;
    }
    return &array[0];
}

int update_state(int state, point_t *old_blank, point_t *new_blank)
{
    int old = old_blank->x * 3 + old_blank->y;
    int new = new_blank->x * 3 + new_blank->y;
    int *array = state_convert_array(state);
    int i = 0;
    int new_state = 0;

    for (i = 0; i < 9; i++)
    {
        if (i == old)
            new_state = new_state * 10 + array[new];
        else if (i == new)
            new_state = new_state * 10 + array[old];
        else
            new_state = new_state * 10 + array[i];
    }

    return new_state;
}

// 全排列的完美hash函数
int p[] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320};
//生成key
int encode(int* s) 
{
    int x = 0;
    int i, j;
    for (i = 0; i < 9; i++) {
        int k = s[i];
        for (j = 0; j < i; j++)
            if (s[j] < s[i])k--;
        x += k * p[8 - i];
    }
    return x;
}

//从key生成对应的序列
void decode(int x, int* s) 
{
    int i, j;
    for (i = 0; i < 9; i++) {
        s[i] = x / p[8 - i];
        x %= p[8 - i];
    }
    for (i = 8; i >= 0; i--)
        for (j = 8; j > i; j--)
            if (s[j] >= s[i])s[j]++;
}


int is_visit(int state)
{
    int *array = state_convert_array(state);
    int key = encode(array);

    if(map[key].visited == 1)
        return 1;
    return 0;
}

int visited(int state)
{
    int *array = state_convert_array(state);
    int key = encode(array);

    if(key < 0 || key > STATES_MAX)
    {
        printf("hash error\n");
        return -1;
    }
    map[key].state = state;
    map[key].visited = 1;
    return 0;
}

void print_search_step_cnt()
{
    int i = 0;
    int cnt = 0;
    int deep[32] = {0};

    for (i = 0; i < STATES_MAX; i++)
    {
        if (map[i].visited != 0)
        {
            cnt++;
            deep[map[i].deep]++;
        
            if(map[i].deep == 0)
                printf("%s [%d]deep=0 : %09d\n", __FUNCTION__, __LINE__, map[i].state);
            if(map[i].deep == 30)
                printf("%s [%d]deep=30 : %09d\n", __FUNCTION__, __LINE__, map[i].state);

        }
    }
    printf("Search step total: %d\n", cnt);

    for (i = 0 ; i < 32; i++)
    {
        printf("%s [%d] deep[%d]=%d\n", __FUNCTION__, __LINE__, i , deep[i]);
    }
    
}

int print_best_path(location_t *location)
{
    int i = 0;
    while(location)
    {
        printf("%09d\n", location->state);
        location = location->parent;
        i++;
    }
    printf("[%d]\n", i - 1);

    return i - 1;
}


int init_game_map()
{
    static int init = 0;

    if(init)
        return 0;

    struct  timeval t1, t2;

    gettimeofday(&t1, NULL);

    memset(&gQ, 0x0, sizeof(gQ));
    memset(&map, 0x0, sizeof(map));
    init = 1;

    int start_state = 123456780;
    int curent_state = 0;
    int next_state = 0;
    int i = 0;
    point_t blank_block = {0, 0}, new_blank = {0, 0};
    int next_key = 0, curent_key = 0;
    queue_t *Q = &gQ;
    int deep = 0;
    
    //四个方向
    int dir[4][2] = 
    {
        {0, 1}, {1, 0},
        {0, -1}, { -1, 0}
    };

    queue_push(Q, start_state);
    visited(start_state);
    while (!queue_empty(Q))
    {
        curent_state = queue_front(Q);
        // printf("%d\n", curent_state);
        //找到可以移动的方块，在四个方向上移动
        blank_block = find_blank_block(curent_state);
        for (i = 0; i < 4; i++)
        {
            new_blank.x = blank_block.x + dir[i][0];
            new_blank.y = blank_block.y + dir[i][1];
            if (is_valid(&new_blank))
            {
                next_state = update_state(curent_state, &blank_block, &new_blank);
                
                // state_convert_array返回静态数组指针，及时存一下
                next_key = encode(state_convert_array(next_state));
                curent_key = encode(state_convert_array(curent_state));
                if (!is_visit(next_state))
                {
                    visited(next_state);
                    queue_push(Q, next_state);
                    map[next_key].parent = &map[curent_key];
                    map[next_key].deep = map[curent_key].deep + 1;
                }
            }
        }
        queue_pop(Q);
    }

    gettimeofday(&t2, NULL);
    printf("-----------%s [%d], Use %dus\n", __FUNCTION__, __LINE__, 
            t2.tv_sec*1000*1000 + t2.tv_usec -  t1.tv_sec*1000*1000 - t1.tv_usec);
    print_search_step_cnt();
    
    return 0;
}



int eight_block_game(char *input)
{
    int ret = 0;
    int start_state = atoi(input);
    int key = 0;

    printf("%s [%d]input=%s\n", __FUNCTION__, __LINE__, input);
    //init actions.
    init_game_map();

    key = encode(state_convert_array(start_state));

    if (!is_visit(start_state))
    {
        printf("%s [%d]Can not complete.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    ret = print_best_path(&map[key]);

    return ret; 
}

