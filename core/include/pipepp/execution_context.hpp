#pragma once
#include <any>
#include <array>
#include <chrono>
#include <map>
#include <shared_mutex>
#include <string>
#include <variant>

#include "kangsw/hash_index.hxx"
#include "kangsw/spinlock.hxx"

namespace pipepp {
namespace impl__ {
class option_base;
} // namespace impl__

/**
 * 실행 문맥 데이터 형식
 *
 * 계층적 타이머 정보
 * 계층적 데이터 정보
 *
 * 를 저장하고 있습니다.
 */
struct execution_context_data {
    using clock = std::chrono::system_clock;
    using debug_variant = std::variant<bool, long, double, std::string, std::any>;
    friend class execution_context;

public:
    struct timer_entity {
        char const* name;
        size_t category_level;
        clock::duration elapsed;
    };

    struct debug_data_entity {
        char const* name;
        size_t category_level;
        debug_variant data;
    };

public:
    std::vector<timer_entity> timers;
    std::vector<debug_data_entity> debug_data;
};

/**
 * 실행 문맥 클래스.
 * 디버깅 및 모니터링을 위한 클래스로,
 *
 * 1) 디버그 플래그 제어
 * 2) 디버그 데이터 저장
 * 3) 구간별 계층적 실행 시간 계측
 * 4) 옵션에 대한 접근 제어(옵션 인스턴스는 pipe에 존재, 실행 문맥은 레퍼런스 읽기 전용)
 *
 * 등의 기능을 제공합니다.
 *
 * 내부에 두 개의 데이터 버퍼를 갖고 있으며, 기본적으로 clear_records가 호출될 때마다 두 개의 버퍼를 스왑합니다.
 * 이를 통해 메모리 재할당을 방지할 수 있는데, 만약 외부에서 context를 요청한 경우 백 버퍼의 오브젝트를 넘기고 재생성합니다.
 */
class execution_context {
public:
    template <typename Ty_> using lock_type = std::unique_lock<Ty_>;
    using clock = std::chrono::system_clock;

    // TODO: 디버그 플래그 제어
    // TODO: 디버그 데이터 저장(variant<bool, long, double, string, any> [])
    // TODO: 실행 시간 계측기

public:
    struct timer_scope_indicator {
        void try_lock() const { throw; }
        void lock();
        void unlock();

    private:
        execution_context* self_ = {};
        size_t index = {};
        clock::time_point issue_;
    };

public:
    execution_context();

public: // accessor
    auto const& option() const { return *options_; }
    operator impl__::option_base const &() const { return *options_; }

public: // methods
    /**
     * @brief 입력을 추출 가능한 상태인지 확인합니다.
     * @return 현재 입력을 추출 가능하면 true
     */
    bool can_consume_read_buffer() const { return rd_buf_valid_.test(std::memory_order_relaxed); }

    /**
     * 현재 범위에 유효한 타이머를 생성합니다.
     * 카테고리를 하나 증가시킵니다.
     */
    template <size_t N>
    std::unique_lock<timer_scope_indicator> timer_scope(char const (&name)[N]);

    /**
     * 디버그 변수를 새롭게 저장합니다.
     */
    template <size_t N, typename Ty_>
    void store_debug_data(char const (&name)[N], Ty_&& value);

public:                    // internal public methods
    void _clear_records(); // invoke() 이전 호출
    void _internal__set_option(impl__::option_base const* opt) { options_ = opt; }
    void _swap_data_buff(); // invoke() 이후 호출

    /**
     * 읽기 버퍼를 추출합니다.
     * 다음 _clear_records() 호출 전까지 데이터를 추출할 수 없는 상태가 됩니다.
     */
    std::shared_ptr<execution_context_data> _consume_read_buffer();

private: // private methods
    auto& _rd() { return context_data_[!front_data_buffer_]; }
    auto& _wr() { return context_data_[front_data_buffer_]; }

private:
    class impl__::option_base const* options_ = {};

    std::array<std::shared_ptr<execution_context_data>, 2> context_data_;
    bool front_data_buffer_ = false;
    kangsw::spinlock swap_lock_;
    std::atomic_flag rd_buf_valid_;

    size_t category_level_ = 0;
};

} // namespace pipepp