#ifndef PRIORITY_QUEUE_HPP
#define PRIORITY_QUEUE_HPP

#include <vector>
#include <math.h>
#include <iostream>
namespace pop
{

    template <class T>
    void swap(T &a, T &b)
    {
        T temp = a;
        a = b;
        b = temp;
    }
    template <class T = int>
    class max_pq
    {
    private:
        std::vector<T> heap;
        std::size_t m_capacity;
        std::size_t m_last_index;
        T empty_value;

    public:
        max_pq(std::size_t capacity, T empty_value) : m_capacity(capacity), m_last_index(0), empty_value(empty_value) { heap.resize(m_capacity, empty_value); }
        bool is_empty() { return m_capacity == 0; }
        std::size_t get_capacity() { return m_capacity; }
        void insert(T x)
        {
            m_last_index++;
            if (m_last_index == m_capacity)
            {
                heap.push_back(x);
                m_capacity = heap.capacity();
            }
            else
            {
                heap[m_last_index] = x;
            }
            swim(m_last_index); // heapify
        }
        void show()
        {
            for (int i = 0; i <= m_last_index; ++i)
                std::cout << heap[i] << " ";
        }
        T getmax() { return heap[1]; }
        void deletemax()
        {
            if (m_last_index == 0)
            {
                // Heap is empty
                return;
            }

            int c_idx = 1;
            swap(heap[1], heap[m_last_index]);
            // std::clog << "Deleted " << heap[m_last_index] << " at index " << m_last_index << std::endl;
            heap[m_last_index] = empty_value;
            m_last_index--;

            while (2 * c_idx <= m_last_index)
            {
                int max_child_idx = 2 * c_idx; // Start with the left child
                if (2 * c_idx + 1 <= m_last_index && heap[2 * c_idx + 1] > heap[2 * c_idx])
                {
                    max_child_idx = 2 * c_idx + 1; // Right child is greater
                }

                if (heap[c_idx] >= heap[max_child_idx])
                {
                    break; // The heap property is restored
                }

                // std::clog << "Max child is " << heap[max_child_idx] << " at " << max_child_idx << std::endl;
                swap(heap[c_idx], heap[max_child_idx]);
                c_idx = max_child_idx;
            }
        }

        ~max_pq() = default;

    private:
        void swim(int idx)
        {
            int k = (idx / 2);
            while (k > 0 && heap[k] < heap[idx])
            {
                swap<T>(heap[idx], heap[k]);
                idx = k;
                k /= 2;
            }
        }
    };
}
namespace pop
{

    template <class T = int>
    class min_pq
    {
    private:
        std::vector<T> heap;
        std::size_t m_capacity;
        std::size_t m_last_index;

    public:
        min_pq(std::size_t capacity) : m_capacity(capacity + 1), m_last_index(0)
        {
            heap.resize(m_capacity + 1); // +1 to make space for 1-based indexing
        }

        bool is_empty() { return m_last_index == 0; }

        std::size_t get_capacity() { return m_last_index; }

        void insert(T x)
        {
            m_last_index++;
            if (m_last_index == m_capacity)
            {
                heap.push_back(x);
                m_capacity = heap.capacity();
            }
            else
            {
                heap[m_last_index] = x;
            }
            swim(m_last_index); // heapify
        }

        T get_min() { return heap[1]; }

        void show()
        {
            for (std::size_t i = 1; i <= m_last_index; ++i)
            {
                std::cout << heap[i] << " ";
            }
            std::cout << std::endl;
        }

        void deletemin()
        {
            if (m_last_index == 0)
            {
                // Heap is empty
                return;
            }

            int c_idx = 1;
            std::swap(heap[1], heap[m_last_index]);
            m_last_index--;

            while (2 * c_idx <= m_last_index)
            {
                int min_child_idx = 2 * c_idx; // Start with the left child
                if (2 * c_idx + 1 <= m_last_index && heap[2 * c_idx + 1] < heap[2 * c_idx])
                {
                    min_child_idx = 2 * c_idx + 1; // Right child is smaller
                }

                if (heap[c_idx] <= heap[min_child_idx])
                {
                    break; // The heap property is restored
                }

                std::swap(heap[c_idx], heap[min_child_idx]);
                c_idx = min_child_idx;
            }
        }

        ~min_pq() = default;

    private:
        void swim(int idx)
        {
            int k = (idx / 2);
            while (k > 0 && heap[k] > heap[idx])
            {
                std::swap(heap[idx], heap[k]);
                idx = k;
                k /= 2;
            }
        }
    };

} // namespace pop

#endif