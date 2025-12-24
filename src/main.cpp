/**
MIT License

Copyright (c) 2025 Andres Pastrana

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <spdlog/spdlog.h>
#include <sstream>
#include <string>

#include <gempba/gempba.hpp>

gempba::node_manager& initiate_node_manager(gempba::scheduler* p_scheduler, gempba::load_balancer* p_load_balancer) {
    if (p_scheduler->rank_me() == 0) {
        return gempba::mp::create_node_manager(p_load_balancer, nullptr);
    }
    gempba::scheduler::worker* v_worker_view = &p_scheduler->worker_view();
    return gempba::mp::create_node_manager(p_load_balancer, v_worker_view);
}

gempba::load_balancer* initiate_load_balancer(gempba::scheduler* p_scheduler, const gempba::balancing_policy p_policy) {
    if (p_scheduler->rank_me() == 0) {
        return gempba::mp::create_load_balancer(p_policy, nullptr);
    }
    gempba::scheduler::worker* v_worker_view = &p_scheduler->worker_view();
    return gempba::mp::create_load_balancer(p_policy, v_worker_view);
}

void foo(std::thread::id p_id, std::string p_message, gempba::node p_parent);

int main() {
    auto* v_scheduler = gempba::mp::create_scheduler(gempba::mp::scheduler_topology::SEMI_CENTRALIZED);
    v_scheduler->set_goal(gempba::MINIMISE, gempba::score_type::I32);

    gempba::load_balancer* v_load_balancer = initiate_load_balancer(v_scheduler, gempba::balancing_policy::QUASI_HORIZONTAL);
    gempba::node_manager& v_node_manager = initiate_node_manager(v_scheduler, v_load_balancer);

    v_node_manager.set_goal(gempba::MINIMISE, gempba::score_type::I32);
    v_scheduler->set_goal(gempba::MINIMISE, gempba::score_type::I32);

    const int v_rank = v_scheduler->rank_me();

    spdlog::info("Rank: {}, Number of processes: {}", v_rank, v_scheduler->world_size());

    constexpr int v_runnable_id = 0;
    if (v_rank == 0) {
        const std::string& v_message = "Marco!";
        const gempba::task_packet v_task(v_message);
        gempba::scheduler::center& v_center_view = v_scheduler->center_view();

        v_center_view.run(v_task, /*runnable id*/ 0);
    } else {
        v_node_manager.set_thread_pool_size(1);
        auto v_helper = [](const std::stringstream& p_ss, std::string& p_message) { p_message = p_ss.str(); };

        const std::function<std::tuple<std::string>(const gempba::task_packet&&)> v_deserializer = [&](const gempba::task_packet&& p_packet) {
            std::string v_message;
            std::stringstream v_ss;
            v_ss.write(reinterpret_cast<const char*>(p_packet.data()), static_cast<int>(p_packet.size()));
            v_helper(v_ss, v_message);
            return std::make_tuple(v_message);
        };

        gempba::scheduler::worker& v_worker_view = v_scheduler->worker_view();

        std::shared_ptr<gempba::serial_runnable> v_serial_runnable = gempba::mp::runnables::return_none::create(v_runnable_id, foo, v_deserializer);

        std::map<int, std::shared_ptr<gempba::serial_runnable>> v_runnables;
        v_runnables[v_runnable_id] = v_serial_runnable;
        v_worker_view.run(v_node_manager, v_runnables);
    }
    v_scheduler->barrier();
    v_scheduler->synchronize_stats();
    v_scheduler->barrier();


    int v_exit = EXIT_FAILURE;
    if (v_rank == 0) {
        gempba::scheduler::center& center_view = v_scheduler->center_view();
        gempba::task_packet v_task = center_view.get_result();
        std::string v_word;
        std::stringstream ss;
        ss.write(reinterpret_cast<const char*>(v_task.data()), static_cast<int>(v_task.size()));
        ss >> v_word;

        spdlog::info("Rank: {}, fetched solution: {}", v_rank, v_word);
        if (v_word == "Polo!") {
            spdlog::info("Rank: {}, received correct solution: {}", v_rank, v_word);
            v_exit = EXIT_SUCCESS;
        } else {
            spdlog::error("Rank: {}, received incorrect solution: {}", v_rank, v_word);
            v_exit = EXIT_FAILURE;
        }
    } else {
        v_exit = EXIT_SUCCESS;
    }
    v_scheduler->barrier();


    gempba::shutdown(); // Clean up GEMPBA resources
    return v_exit;
}


auto serializer = [](auto&... p_args) {
    auto v_helper_ser = [](std::string& p_first) { return p_first; };

    return v_helper_ser(p_args...);
};

template <typename T>
auto get_serializer() -> std::function<gempba::task_packet(T&)> {

    return [](T& p_arg) {
        auto v_string = serializer(p_arg); // std::string
        return gempba::task_packet{v_string};
    };
}

void foo([[maybe_unused]] std::thread::id p_id, std::string p_message, [[maybe_unused]] gempba::node p_parent) {
    int v_rank = gempba::get_node_manager().rank_me();
    spdlog::info("Rank: {}, received message: {}", v_rank, p_message);

    if (p_message == "Marco!") {
        spdlog::info("Rank: {}, received correct message: {}", v_rank, p_message);
    } else {
        spdlog::error("Rank: {}, received incorrect message: {}", v_rank, p_message);
    }

    std::string v_word = "Polo!";
    std::function<gempba::task_packet(std::string&)> v_serializer = get_serializer<std::string>();
    gempba::get_node_manager().try_update_result(v_word, gempba::score::make(v_rank), v_serializer);
    spdlog::info("Rank: {}, set solution: {}", v_rank, v_word);
}
