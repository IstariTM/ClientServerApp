#include "Server.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <iostream>
#include <format>
#include <zlib.h>

using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::redirect_error;
using boost::asio::use_awaitable;
using boost::asio::awaitable;
using boost::asio::buffer;
using boost::asio::ip::tcp;
namespace this_coro = boost::asio::this_coro;

namespace {
    /**
     * @brief Compresses a given string using the zlib deflate algorithm.
     *
     * This function takes an input string and compresses it using the zlib library
     * with the best compression level. The compressed data is returned as a std::string.
     *
     * @param str The input string to be compressed.
     * @return std::string The compressed representation of the input string.
     *
     * @throws std::runtime_error If compression initialization or processing fails.
     */
    std::string compress_string(const std::string& str) {
        z_stream zs{};
        if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK)
            throw std::runtime_error("deflateInit failed");

        zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
        zs.avail_in = static_cast<uInt>(str.size());

        std::string outstring;
        std::vector<char> outbuffer(32768);

        int ret;
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer.data());
            zs.avail_out = static_cast<uInt>(outbuffer.size());

            ret = deflate(&zs, Z_FINISH);
            if (ret == Z_STREAM_ERROR) {
                deflateEnd(&zs);
                throw std::runtime_error("deflate failed");
            }
            outstring.append(outbuffer.data(), outbuffer.size() - zs.avail_out);
        } while (ret != Z_STREAM_END);

        deflateEnd(&zs);
        return outstring;
    }

    /**
     * @brief Decompresses a string using the zlib inflate algorithm.
     *
     * This function takes a compressed input string (in zlib format) and decompresses it,
     * returning the resulting uncompressed string. It throws a std::runtime_error if
     * decompression fails at any stage.
     *
     * @param str The compressed input string to decompress.
     * @return std::string The decompressed output string.
     * @throws std::runtime_error If initialization or decompression fails.
     */
    std::string decompress_string(const std::string& str) {
        z_stream zs{};
        if (inflateInit(&zs) != Z_OK)
            throw std::runtime_error("inflateInit failed");

        zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
        zs.avail_in = static_cast<uInt>(str.size());

        std::string outstring;
        std::vector<char> outbuffer(32768);

        int ret;
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer.data());
            zs.avail_out = static_cast<uInt>(outbuffer.size());

            ret = inflate(&zs, 0);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                inflateEnd(&zs);
                throw std::runtime_error("inflate failed");
            }
            outstring.append(outbuffer.data(), outbuffer.size() - zs.avail_out);
        } while (ret != Z_STREAM_END);

        inflateEnd(&zs);
        return outstring;
    }
}

Server::Server(boost::asio::io_context& io_context, short port)
    : acceptor_(boost::asio::make_strand(io_context), tcp::endpoint(tcp::v4(), port)),
      configManager_("config.txt"),
      stats_() {
    configManager_.load();
    stats_.start();
}

Server::~Server() {
    stats_.stop();
    try {
        acceptor_.close();
    } catch (...) {}
}

void Server::run() {
    co_spawn(acceptor_.get_executor(), listener(), detached);
}

awaitable<void> Server::listener() {
    try {
        for (;;) {
            tcp::socket socket = co_await acceptor_.async_accept(use_awaitable);
            co_spawn(acceptor_.get_executor(), handleClient(std::move(socket)), detached);
        }
    } catch (const std::exception& e) {
        std::cerr << "Listener exception: " << e.what() << std::endl;
    }
    co_return;
}

awaitable<void> Server::handleClient(tcp::socket socket) {
    std::cout << "[Info] Client connected\n";
    try {
        auto executor = co_await this_coro::executor;
        std::array<char, 4096> data;
        
        for (;;) {
            std::size_t n = co_await socket.async_read_some(buffer(data), use_awaitable);
            if (n == 0) {
                break;
            }

            std::string compressed_input(data.data(), n);
            std::string line;
            try {
                line = decompress_string(compressed_input);
            } catch (const std::exception& e) {
                std::cerr << "Decompression error: " << e.what() << "\n";
                continue;
            }

            if (line.empty()) continue;

            std::string response;
            if (line.starts_with("$get ")) {
                auto key = line.substr(5);
                //key.erase(key.find_last_not_of(" \r\n") + 1);
                std::string value = configManager_.get(key);
                stats_.incrementGet(key);
                auto [reads, writes] = stats_.getStats(key);
                response = std::format("{}={}\nreads={}\nwrites={}\n", key, value, reads, writes);
            } else if (line.starts_with("$set ")) {
                auto eq = line.find('=');
                if (eq != std::string::npos && eq > 5) {
                    std::string key = line.substr(5, eq - 5);
                    std::string value = line.substr(eq + 1);
                    //value.erase(value.find_last_not_of(" \r\n") + 1);
                    configManager_.set(key, value);
                    stats_.incrementSet(key);
                    auto [reads, writes] = stats_.getStats(key);
                    response = std::format("Set {}={}\nreads={}\nwrites={}\n", key, value, reads, writes);
                } else {
                    response = "Invalid $set format\n";
                }
            } else {
                response = "Unknown command\n";
            }

            std::string compressed;
            try {
                compressed = compress_string(response);
            } catch (const std::exception& e) {
                std::cerr << "Compression error: " << e.what() << std::endl;
                compressed = "Compression failed\n";
            }

            co_await boost::asio::async_write(socket, boost::asio::buffer(compressed), use_awaitable);
        }
    } catch (const boost::system::system_error& e) {
        if (e.code() == boost::asio::error::eof) {
            std::cout << "[Info] Client disconnected (EOF)\n";
        } else {
            std::cerr << "Client session exception: " << e.what() << "\n";
        }
    }
    co_return;
}
