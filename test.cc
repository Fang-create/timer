#include <unistd.h>
#include <iostream>
#include <vector>
#include <list>
using namespace std;

#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#include <sys/time.h>
#include <mach/task.h>
#include <mach/mach.h>
#else
#include <time.h>
#endif

class CTimerNode {
public:
    CTimerNode(int fd) : id(fd), ref(0) {}

    void Offline() {
        this->ref = 0;
    }

    bool TryKill() {
        if(this->ref == 0) return true;
        DecrRef();
        if (this->ref == 0) {
            cout << id << " is killed down" << endl;
            return true;
        }
        cout << id << " ref is " << ref << endl;
        return false;
    }

    void IncrRef() {
        this->ref++;
    }

protected:
    void DecrRef() {
        this->ref--;
    }

private:
    int ref;  // 节点计数
    int id;   // node 节点
};

const int TW_SIZE = 16;
const int EXPIRE = 10;
const int TW_MASK = TW_SIZE - 1;
static size_t iRealTick = 0;
typedef list<CTimerNode*> TimeList;
typedef TimeList::iterator TimeListIter;
typedef vector<TimeList> TimeWheel;  // 数组链表 时间轮

// 增加超时，对16取余，并增加一个计数
void AddTimeout(TimeWheel &tw, CTimerNode *p) {
    if (p) {
        p->IncrRef();  //增加一个计数
        TimeList &le = tw[(iRealTick+EXPIRE) & TW_MASK];  //找到vector的中的数组
        le.push_back(p);
    }
}

// 用来表示delay时间后调用 AddTimeout
void AddTimeoutDelay(TimeWheel &tw, CTimerNode *p, size_t delay) {
    if (p) {
        p->IncrRef();
        //[15]
        TimeList &le = tw[(iRealTick + EXPIRE + delay) & TW_MASK];  // 与15 & ，进行取余
        le.push_back(p);
    }
} 

//时间转动
void TimerShift(TimeWheel &tw)
{
    size_t tick = iRealTick;
    iRealTick++;
    TimeList &le = tw[tick & TW_MASK];
    TimeListIter iter = le.begin();
    for (; iter != le.end();) {
        CTimerNode *p = *iter;
        if (p && p->TryKill()) {
            delete p;
            le.erase(iter++);
        } else iter++;
    }
}

static time_t
current_time() {
	time_t t;
#if !defined(__APPLE__) || defined(AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER)
	struct timespec ti;
	clock_gettime(CLOCK_MONOTONIC, &ti);
	t = (time_t)ti.tv_sec;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (time_t)tv.tv_sec;
#endif
	return t;
}

int main ()
{
    TimeWheel tw(TW_SIZE);

    CTimerNode *p = new CTimerNode(10001);

    AddTimeout(tw, p);   // 增加超时

    AddTimeoutDelay(tw, p, 5);  // 增加超时 后延时,  将5s添加到时间轮中， ref = 2;

    time_t start = current_time();
    for (;;) {
        time_t now = current_time();
        if (now - start > 0)
        {
            for (int i=0; i < now-start; i++)  // 每一秒加1，iRealTick++  //
                TimerShift(tw);
            start = now; // 
            cout << "check timer shift " << iRealTick <<  endl;
        }
        usleep(1000);
    }
    return 0;
}
