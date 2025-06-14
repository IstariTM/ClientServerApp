#pragma once

#include "ConfigManager.hpp"
#include "Stats.hpp"

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/use_awaitable.hpp>

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::use_awaitable;

class Server {
public:
    /**
     * @brief Constructs a Server object, initializes the acceptor, configuration manager, and statistics.
     * 
     * This constructor sets up the server to listen for incoming TCP connections on the specified port.
     * It also loads the server configuration from "config.json" and starts the statistics tracking.
     * 
     * @param io_context Reference to the Boost.Asio I/O context used for asynchronous operations.
     * @param port The port number on which the server will listen for incoming connections.
     */
    Server(boost::asio::io_context& io_context, short port);

    /**
     * @brief Destructor for the Server class.
     *
     * Stops the server statistics collection and attempts to close the network acceptor.
     * Any exceptions thrown during the closing of the acceptor are caught and ignored.
     */
    ~Server();

    /**
     * @brief Starts the server's main execution loop.
     *
     * This function initiates the asynchronous listener coroutine by spawning it
     * on the acceptor's executor. The listener coroutine handles incoming client
     * connections. The coroutine is detached, meaning it will run independently
     * without blocking the current thread.
     */
    void run();

private:
    /**
     * @brief Asynchronously listens for incoming TCP connections and spawns a handler for each client.
     *
     * This coroutine runs an infinite loop, accepting new client connections using the acceptor.
     * For each accepted connection, it spawns a new coroutine to handle the client using the provided executor.
     * Any exceptions thrown during the accept loop are caught and logged to standard error.
     *
     * @return awaitable<void> An awaitable representing the asynchronous operation.
     */
    awaitable<void> listener();

    /**
     * @brief Handles communication with a connected client over a TCP socket.
     *
     * This coroutine manages the lifecycle of a client session. It reads compressed
     * commands from the client, decompresses them, processes supported commands
     * ("$get <key>" and "$set <key>=<value>"), and sends back compressed responses.
     * The function supports asynchronous I/O using Boost.Asio coroutines.
     *
     * Supported commands:
     *   - "$get <key>": Retrieves the value for the specified key from the configuration manager,
     *     increments the read statistics, and returns the value along with read/write counts.
     *   - "$set <key>=<value>": Sets the value for the specified key in the configuration manager,
     *     increments the write statistics, and returns an acknowledgment with read/write counts.
     *   - Any other command: Returns "Invalid command".
     *
     * The function handles client disconnects and logs session events.
     *
     * @param socket The TCP socket representing the client connection.
     * @return awaitable<void> Coroutine handle for asynchronous execution.
     */
    awaitable<void> handleClient(tcp::socket socket);

    tcp::acceptor acceptor_;
    ConfigManager configManager_;
    Stats stats_;
};
