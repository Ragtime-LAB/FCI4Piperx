#ifndef FLORID_SEQLOCK_HPP
#define FLORID_SEQLOCK_HPP

#include <atomic>

namespace florid::detail
{
    template <typename T>
    class SeqlockBuf
    {
        static_assert(std::is_trivially_copyable_v<T>);

    public:
        // no lock
        void write(const T& val)
        {
            // 1. version + 1 (odd), writing...
            m_version.fetch_add(1, std::memory_order_relaxed);
            std::atomic_thread_fence(std::memory_order_release);
            // 2. writing data
            m_data = val;

            // 3. version + 1 (even)，written done.
            std::atomic_thread_fence(std::memory_order_release);
            m_version.fetch_add(1, std::memory_order_relaxed);
        }

        template <typename Func>
        void manipulate(Func&& func)
        {
            m_version.fetch_add(1, std::memory_order_relaxed);
            std::atomic_thread_fence(std::memory_order_release);

            func(m_data);

            m_version.fetch_add(1, std::memory_order_relaxed);
            std::atomic_thread_fence(std::memory_order_release);
        };

        // optimistic reading
        T read()
        {
            T val;
            uint32_t v1, v2{};
            do
            {
                // 1. get version
                v1 = m_version.load(std::memory_order_acquire);

                // odd -> someone's writing, keep waiting...
                if (v1 & 1)
                {
                    continue;
                }

                std::atomic_thread_fence(std::memory_order_acquire);
                // 2. copy data
                val = m_data;
                std::atomic_thread_fence(std::memory_order_acquire);

                // 3. get version again
                v2 = m_version.load(std::memory_order_acquire);

                // 4. version changed(v1 != v2) means data corrupted, retry.
            }
            while (v1 != v2);

            return val;
        }

    private:
        T m_data{};
        std::atomic<int32_t> m_version{0};
    };
};

#endif //FLORID_SEQLOCK_HPP
