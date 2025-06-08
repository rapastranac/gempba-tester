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

#include <sstream>
#include <string>
#include <spdlog/spdlog.h>

#include <BranchHandler/BranchHandler.hpp>
#include <MPI_Modules//MPI_Scheduler.hpp>
#include <DLB/DLB_Handler.hpp>
#include <Resultholder/ResultHolder.hpp>

gempba::BranchHandler& handler = gempba::BranchHandler::getInstance();
gempba::MPI_Scheduler& scheduler = gempba::MPI_Scheduler::getInstance();
gempba::DLB_Handler& dlb = gempba::DLB_Handler::getInstance();

void foo(int p_id, std::string p_message, void* p_parent = nullptr);

int main() {
    #ifdef GEMPBA_DEBUG_COMMENTS
    spdlog::set_level (spdlog::level::debug);
    #endif

    handler.passMPIScheduler(&scheduler);
    const int v_rank = scheduler.rank_me();

    spdlog::info("Rank: {}, Number of processes: {}", v_rank, scheduler.getWorldSize());

    if (v_rank == 0) {
        const std::string& v_message = "Marco!";
        scheduler.runCenter(v_message.data(), static_cast<int>(v_message.size()));
    } else {
        handler.initThreadPool(1);
        auto v_helper = [](const std::stringstream& p_ss, std::string& p_message) {
            p_message = p_ss.str();
        };
        auto v_deserializer = [&](const std::stringstream& p_ss, auto&... p_args) {
            v_helper(p_ss, p_args...);
        };
        std::function<std::shared_ptr<gempba::ResultHolderParent>(char*, int)> v_buffer_decoder = handler.constructBufferDecoder<void, std::string>(foo, v_deserializer);
        std::function<std::pair<int, std::string>()> v_result_fetcher = handler.constructResultFetcher();

        scheduler.runNode(handler, v_buffer_decoder, v_result_fetcher);
    }
    scheduler.barrier();


    int v_exit = EXIT_FAILURE;
    if (v_rank == 0) {
        auto v_word = scheduler.fetchSolution();
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
    auto v_helper_ser = [](std::string& p_first) {
        return p_first;
    };

    return v_helper_ser(p_args...);
};

void foo(int p_id, std::string p_message, void* p_parent) {
    int v_rank = handler.rank_me();
    spdlog::info("Rank: {}, received message: {}", v_rank, p_message);

    if (p_message == "Marco!") {
        spdlog::info("Rank: {}, received correct message: {}", v_rank, p_message);
    } else {
        spdlog::error("Rank: {}, received incorrect message: {}", v_rank, p_message);
    }

    std::string v_word = "Polo!";
    handler.holdSolution(v_rank, v_word, serializer);
    handler.updateRefValue(v_rank);
    spdlog::info("Rank: {}, set solution: {}", v_rank, v_word);

}
