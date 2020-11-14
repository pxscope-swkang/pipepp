#include "pipepp/execution_context.hpp"
#include <mutex>

pipepp::execution_context::execution_context()
{
    for (auto& pt : context_data_) { pt = std::make_shared<execution_context_data>(); }
}

std::shared_ptr<pipepp::execution_context_data> pipepp::execution_context::_consume_read_buffer()
{
    std::lock_guard _0{swap_lock_};

    if (!rd_buf_valid_.test(std::memory_order_acquire)) {
        return {};
    }

    // ���� ������ ũ�⸦ �̸� ������, ���� Ȯ�忡 ���� �ݺ����� �޸� ���Ҵ��� �����մϴ�.
    auto rd = _rd();
    auto& data = *(_rd() = std::make_shared<execution_context_data>());
    data.debug_data.reserve(rd->debug_data.capacity());
    data.timers.reserve(rd->timers.capacity());

    rd_buf_valid_.clear();
    return std::move(rd);
}

void pipepp::execution_context::_clear_records()
{
    _wr()->debug_data.clear();
    _wr()->timers.clear();
}

void pipepp::execution_context::_swap_data_buff()
{
    std::lock_guard _0(swap_lock_);
    rd_buf_valid_.test_and_set(std::memory_order_release);
    front_data_buffer_ = !front_data_buffer_;
}