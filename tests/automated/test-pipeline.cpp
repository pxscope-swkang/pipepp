#include <algorithm>
#include <memory>
#include <vector>
#include <format>

#include "catch.hpp"
#include "pipepp/pipepp.h"

namespace pipepp_test::pipelines {
using namespace pipepp;

struct my_shared_data : public base_shared_context {
    PIPEPP_DECLARE_OPTION_CLASS(my_shared_data);
    int level = 0;
};

struct exec_0 {
    PIPEPP_DECLARE_OPTION_CLASS(exec_0);
    PIPEPP_OPTION_FULL(bool, is_first, false, "debug.show");

    using input_type = std::tuple<double>;
    using output_type = std::tuple<double, double>;

    pipe_error operator()(execution_context& so, input_type const& i, output_type& o)
    {
        PIPEPP_REGISTER_CONTEXT(so);

        using namespace std::literals;

        PIPEPP_ELAPSE_SCOPE("Timer Default");
        printf(std::format("is_first? {}\n", is_first(so)).c_str());
        std::this_thread::sleep_for(1ms);
        auto [val] = i;
        auto& [a, b] = o;
        a = val;
        b = val * 2.0;

        PIPEPP_STORE_DEBUG_DATA("Sample Variable", a);
        return pipe_error::ok;
    }

    static auto factory()
    {
        return make_executor<exec_0>();
    }
};

struct exec_1 {
    using input_type = std::tuple<double, double>;
    using output_type = std::tuple<double>;

    pipe_error operator()(execution_context& so, input_type const& i, output_type& o)
    {
        using namespace std::literals;
        std::this_thread::sleep_for(1ms);
        auto& [a, b] = i;
        auto& [D] = o;
        D = sqrt(a * b);
        return pipe_error::ok;
    }

    static auto factory()
    {
        return make_executor<exec_1>();
    }
};

static void link_1_0(my_shared_data const&, exec_1::output_type const& i, exec_0::input_type& o)
{
    auto& [val] = i;
    auto& [a] = o;

    a = val;
}

//TEST_CASE("pipeline compilation", "")
//{
//    constexpr int NUM_CASE = 128;
//    std::vector<char> cases(NUM_CASE);
//    std::vector<int> order(NUM_CASE);
//    std::atomic_int ordering = 0;
//
//    using pipeline_type = pipeline<my_shared_data, exec_0>;
//    auto pl = pipeline_type::make("0.0", 64, &exec_0::factory);
//    auto _0 = pl->front();
//    auto _1_0
//      = _0.create_and_link_output(
//        "1.0", 64, link_as_is, &exec_1::factory);
//    auto _1_1
//      = _0.create_and_link_output(
//        "1.1", 64, link_as_is, &exec_1::factory);
//    auto _2
//      = _1_1.create_and_link_output(
//              "2.0", 64, &link_1_0, &exec_0::factory)
//          .add_output_handler([&](pipe_error, my_shared_data const& so, exec_0::output_type const& val) {
//              auto [a, b] = val;
//              order[ordering++] = so.level;
//              std::print("level {:<4}: {:>10.3}, {:>10.3}\n", so.level, a, b);
//              cases[so.level] += 1;
//          });
//
//    exec_0::is_first(_0.options(), true);
//    pl->launch();
//
//    using namespace std::literals;
//    for (int iter = 0; iter < cases.size(); ++iter) {
//        while (!pl->can_suply()) { std::this_thread::sleep_for(100us); }
//        if (auto exec_result = _0.consume_execution_result()) {
//            CHECK(exec_result->debug_data.size() == 1);
//            CHECK(exec_result->timers.size() == 2);
//        }
//
//        pl->suply({iter * 0.1}, [iter](my_shared_data& so) { so.level = iter; });
//    }
//    pl->sync();
//    REQUIRE(std::ranges::count(cases, 1) == cases.size());
//    REQUIRE(std::is_sorted(cases.begin(), cases.end()));
//}
} // namespace pipepp_test::pipelines
