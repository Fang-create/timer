/*
 * @Author: your name
 * @Date: 2020-09-06 14:39:25
 * @LastEditTime: 2020-09-07 22:05:24
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /test-timer/skiplist.h
 */
#ifndef _MARK_SKIPLIST_
#define _MARK_SKIPLIST_

/* ZSETs use a specialized version of Skiplists */
#define ZSKIPLIST_MAXLEVEL 32 /* Should be enough for 2^64 elements */
#define ZSKIPLIST_P 0.25      /* Skiplist P = 1/4 */

typedef struct zskiplistNode zskiplistNode;
typedef void (*handler_pt) (zskiplistNode *node);

// 调表的节点
struct zskiplistNode {
    // sds ele;
    // double score;
    unsigned long score; // 时间戳
    handler_pt handler;  //回调
    /* struct zskiplistNode *backward; 从后向前遍历时使用*/
    struct zskiplistLevel {
        struct zskiplistNode *forward;  // 层节点前置节点
        /* unsigned long span; 这个存储的level间节点的个数，在定时器中并不需要*/ 
    } level[];
};

typedef struct zskiplist {
    // 添加一个free的函数
    struct zskiplistNode *header;  /*, *tail 并不需要知道最后一个节点, 删掉tail*/
    int length;    // node的数量
    int level;     // 层数
} zskiplist;

zskiplist *zslCreate(void);
void zslFree(zskiplist *zsl);
zskiplistNode *zslInsert(zskiplist *zsl, unsigned long score, handler_pt func);
zskiplistNode* zslMin(zskiplist *zsl);
void zslDeleteHead(zskiplist *zsl);
void zslDelete(zskiplist *zsl, zskiplistNode* zn); 

void zslPrint(zskiplist *zsl);
#endif
