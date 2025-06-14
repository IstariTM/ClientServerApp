#include "Server.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <thread>
#include <vector>
#include <iostream>

int main() {
    try {
        const int thread_count = std::thread::hardware_concurrency();
        boost::asio::io_context io_context;

        // Handle SIGINT (Ctrl+C)
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) {
            std::cout << "\n[Server] Signal received. Shutting down...\n";
            io_context.stop();
        });

        Server server(io_context, 8888);
        server.run();

        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&io_context]() {
                io_context.run();
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        std::cout << "[Server] Shutdown complete.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
