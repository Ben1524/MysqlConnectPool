/**
*@ClassName MPSCQueue
*@Author cxk
*@Data 25-6-2 下午11:08
*/
//

#ifndef MYSQLCONNECTPOOL_MPSCQUEUE_H
#define MYSQLCONNECTPOOL_MPSCQUEUE_H

#include <atomic>

namespace cxk
{

/**
 * @brief 实现多写单读队列（MPSC Queue）
 * @tparam T 
 */
template <typename T>
class MPSCQueue
{
public:
    MPSCQueue()
            : head_(new BufferNode), tail_(head_.load(std::memory_order_relaxed))
    {
    }
    ~MPSCQueue()
    {
        T output;
        while (this->dequeue(output))
        {
        }
        BufferNode *front = head_.load(std::memory_order_relaxed);
        delete front;
    }

    void enqueue(const T &output)
    {
        BufferNode *newNode = new BufferNode(output);
        BufferNode *prevhead{head_.exchange(newNode, std::memory_order_acq_rel)};// 获取语义确保后续操作不会重排到前面
        prevhead->next_.store(newNode, std::memory_order_release);
    }
    void enqueue(T &&output)
    {
        BufferNode *newNode = new BufferNode(std::move(output));
        BufferNode *prevhead{head_.exchange(newNode, std::memory_order_acq_rel)};// 获取语义确保后续操作不会重排到前面
        prevhead->next_.store(newNode, std::memory_order_release);
    }
    bool dequeue(T &output)
    {
        // 因为只有一个消费者，所以可以使用 relaxed 语义来获取尾部节点，只需要保证原子性即可
        BufferNode *tail = tail_.load(std::memory_order_relaxed);
        BufferNode *next = tail->next_.load(std::memory_order_acquire);

        if (next == nullptr)
        {
            return false;
        }
        output = std::move(*(next->dataPtr_));
        delete next->dataPtr_;
        tail_.store(next, std::memory_order_release);
        delete tail;
        return true;
    }

    bool empty()
    {
        BufferNode *tail = tail_.load(std::memory_order_relaxed);
        BufferNode *next = tail->next_.load(std::memory_order_acquire);
        return next == nullptr;
    }
private:
    struct BufferNode
    {
        BufferNode() = default;

        BufferNode(const T &data) : dataPtr_(new T(data))
        {
        }

        BufferNode(T &&data) : dataPtr_(new T(std::move(data)))
        {
        }

        T *dataPtr_;
        std::atomic<BufferNode *> next_{nullptr};
    };

    std::atomic<BufferNode *> head_;
    std::atomic<BufferNode *> tail_;
};
}


#endif //MYSQLCONNECTPOOL_MPSCQUEUE_H
