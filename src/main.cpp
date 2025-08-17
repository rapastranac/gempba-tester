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

#include <BranchHandler/branch_handler.hpp>
#include <DLB/DLB_Handler.hpp>
#include <MPI_Modules/mpi_scheduler.hpp>
#include <Resultholder/ResultHolder.hpp>

gempba::branch_handler& handler = gempba::branch_handler::get_instance();
gempba::mpi_scheduler& scheduler = gempba::mpi_scheduler::get_instance();
gempba::DLB_Handler& dlb = gempba::DLB_Handler::getInstance();

void foo(int p_id, std::string p_message, void* p_parent = nullptr);

int main() {
#ifdef GEMPBA_DEBUG_COMMENTS
    spdlog::set_level(spdlog::level::debug);
#endif

    handler.pass_mpi_scheduler(&scheduler);
    const int v_rank = scheduler.rank_me();

    spdlog::info("Rank: {}, Number of processes: {}", v_rank, scheduler.get_world_size());

    if (v_rank == 0) {
        const std::string& v_message = "Marco!";
        gempba::task_packet v_task(v_message);
        scheduler.run_center(v_task);
    } else {
        handler.init_thread_pool(1);
        auto v_helper = [](const std::stringstream& p_ss, std::string& p_message) { p_message = p_ss.str(); };
        auto v_deserializer = [&](const std::stringstream& p_ss, auto&... p_args) { v_helper(p_ss, p_args...); };
        std::function<std::shared_ptr<gempba::ResultHolderParent>(gempba::task_packet)> v_buffer_decoder = handler.construct_buffer_decoder<void, std::string>(foo, v_deserializer);
        std::function<gempba::result()> v_result_fetcher = handler.construct_result_fetcher();

        scheduler.run_node(handler, v_buffer_decoder, v_result_fetcher);
    }
    scheduler.barrier();


    int v_exit = EXIT_FAILURE;
    if (v_rank == 0) {
        gempba::task_packet v_task = scheduler.fetch_solution();
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
    scheduler.barrier();
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

void foo(int p_id, std::string p_message, void* p_parent) {
    int v_rank = handler.rank_me();
    spdlog::info("Rank: {}, received message: {}", v_rank, p_message);

    if (p_message == "Marco!") {
        spdlog::info("Rank: {}, received correct message: {}", v_rank, p_message);
    } else {
        spdlog::error("Rank: {}, received incorrect message: {}", v_rank, p_message);
    }

    std::string v_word = "Polo!";
    std::function<gempba::task_packet(std::string&)> p_serializer = get_serializer<std::string>();
    handler.try_update_result(v_word, gempba::score::make(v_rank), p_serializer);
    spdlog::info("Rank: {}, set solution: {}", v_rank, v_word);
}
