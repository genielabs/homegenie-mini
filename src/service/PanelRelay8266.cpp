#ifdef ESP8266
#define LWIP_INTERNAL
#include "PanelRelay8266.h"
#include "net/TimeClient.h"
#include <BearSSLHelpers.h>
#include <lwip/dns.h>
#include <lwip/tcp.h>
#include "PanelRelayRoots.h"

namespace PanelRelay {
namespace {
    constexpr char Host[] = "push.homegenie.workers.dev";
    struct Transfer {
        bool stackReady = false;
        br_ssl_client_context tls{};
        br_x509_minimal_context verifier{};
        std::unique_ptr<uint8_t[]> buffer;
        tcp_pcb* socket = nullptr;
        pbuf* received = nullptr;
        uint16_t offset = 0;
        bool closed = false;
        String request;
        String statusLine;
        size_t sent = 0;
        uint32_t started = millis();
        bool connected = false;
        Transfer() {
            // The core aborts if its 6200-byte TLS stack cannot be allocated.
            if (stack_thunk_get_refcnt() || ESP.getMaxFreeBlockSize() >= 6400) {
                stack_thunk_add_ref();
                stackReady = true;
            }
        }
        ~Transfer() {
            if (socket) {
                tcp_arg(socket, nullptr);
                tcp_recv(socket, nullptr);
                tcp_err(socket, nullptr);
                if (!connected || tcp_close(socket) != ERR_OK) tcp_abort(socket);
            }
            if (received) pbuf_free(received);
            if (stackReady) stack_thunk_del_ref();
        }
    };
    std::unique_ptr<Transfer> transfer;
    ip_addr_t resolved;
    unsigned dnsGeneration = 0;
    bool hasAddress = false, dnsFailed = false;
    void resolvedHost(const char*, const ip_addr_t* address, void* generation) {
        if (uintptr_t(generation) != dnsGeneration || !transfer) return;
        if (address) { resolved = *address; hasAddress = true; }
        else dnsFailed = true;
    }
    err_t tcpConnected(void* arg, tcp_pcb*, err_t err) {
        auto& t = *static_cast<Transfer*>(arg);
        t.connected = err == ERR_OK;
        t.closed = !t.connected;
        return ERR_OK;
    }
    err_t tcpReceived(void* arg, tcp_pcb*, pbuf* packet, err_t) {
        auto& t = *static_cast<Transfer*>(arg);
        if (!packet) t.closed = true;
        else if (t.received) pbuf_cat(t.received, packet);
        else t.received = packet;
        return ERR_OK;
    }
    void tcpFailed(void* arg, err_t) {
        auto& t = *static_cast<Transfer*>(arg);
        t.socket = nullptr; // lwIP already released the PCB.
        t.closed = true;
    }
    void finish(int status = 0) {
        if (status < 200 || status >= 300) IO::Logger::warn("Panel relay delivery failed (%d)", status);
        transfer.reset();
    }
}
bool busy() { return bool(transfer); }

void startRequest(const String& body, const char* key) {
    transfer.reset(new(std::nothrow) Transfer);
    if (!transfer) return;
    auto& t = *transfer;
    if (!t.stackReady || !t.request.reserve(body.length() + strlen(key) + 180)) { finish(); return; }
    t.request = String("POST / HTTP/1.1\r\nHost: ") + Host + "\r\nConnection: close\r\nContent-Type: application/json\r\nx-hg-key: " +
                key + "\r\nContent-Length: " + body.length() + "\r\n\r\n" + body;
    hasAddress = dnsFailed = false;
    auto result = dns_gethostbyname(Host, &resolved, resolvedHost, reinterpret_cast<void*>(++dnsGeneration));
    if (result == ERR_OK) hasAddress = true;
    else if (result != ERR_INPROGRESS) finish();
}

// Advance at most one TLS record operation per pass through the firmware loop.
void loop() {
    if (!transfer) return;
    auto& t = *transfer;
    if (!WiFi.isConnected() || dnsFailed || millis() - t.started >= 15000) { finish(); return; }
    if (!hasAddress) return;
    auto& engine = t.tls.eng;
    if (!t.socket) {
        if (t.closed) { finish(); return; }
        t.socket = tcp_new();
        if (!t.socket) { finish(); return; }
        tcp_arg(t.socket, &t);
        tcp_err(t.socket, tcpFailed);
        tcp_recv(t.socket, tcpReceived);
        if (tcp_connect(t.socket, &resolved, 443, tcpConnected) != ERR_OK) finish();
        return;
    }
    if (!t.connected) return;
    if (!t.buffer) {
        size_t freeBlock = ESP.getMaxFreeBlockSize();
        size_t capacity = freeBlock >= BR_SSL_BUFSIZE_MONO + 2048 ? BR_SSL_BUFSIZE_MONO : 4096 + 325;
        t.buffer.reset(new(std::nothrow) uint8_t[capacity]);
        if (!t.buffer) { finish(); return; }
        br_ssl_client_init_full(&t.tls, &t.verifier, RelayAnchors, sizeof(RelayAnchors) / sizeof(RelayAnchors[0]));
        auto epoch = Net::TimeClient::getNTPClient().getEpochTime();
        br_x509_minimal_set_time(&t.verifier, epoch / 86400 + 719528, epoch % 86400);
        br_ssl_engine_set_buffer(&engine, t.buffer.get(), capacity, 0);
        uint32_t entropy[8];
        for (auto& word : entropy) word = RANDOM_REG32;
        br_ssl_engine_inject_entropy(&engine, entropy, sizeof entropy);
        if (!br_ssl_client_reset(&t.tls, Host, 0)) { finish(); return; }
        return;
    }
    size_t len;
    unsigned state = br_ssl_engine_current_state(&engine);
    if (state & BR_SSL_SENDREC) {
        size_t room = tcp_sndbuf(t.socket);
        if (room) {
            auto data = BearSSL::thunk_br_ssl_engine_sendrec_buf(&engine, &len);
            uint16_t n = std::min(room, len);
            err_t err = tcp_write(t.socket, data, n, TCP_WRITE_FLAG_COPY);
            if (err == ERR_OK) {
                tcp_output(t.socket);
                BearSSL::thunk_br_ssl_engine_sendrec_ack(&engine, n);
            } else if (err != ERR_MEM) finish();
        }
    } else if (state & BR_SSL_RECVAPP) {
        auto data = BearSSL::thunk_br_ssl_engine_recvapp_buf(&engine, &len);
        for (size_t i = 0; i < len; ++i) {
            if (data[i] == '\n') {
                int status = t.statusLine.startsWith("HTTP/1.") ? t.statusLine.substring(9, 12).toInt() : 0;
                finish(status);
                return;
            }
            if (t.statusLine.length() >= 160) { finish(); return; }
            t.statusLine += char(data[i]);
        }
        BearSSL::thunk_br_ssl_engine_recvapp_ack(&engine, len);
    } else if ((state & BR_SSL_SENDAPP) && t.sent < t.request.length()) {
        auto data = BearSSL::thunk_br_ssl_engine_sendapp_buf(&engine, &len);
        size_t n = std::min(len, size_t(t.request.length() - t.sent));
        memcpy(data, t.request.c_str() + t.sent, n);
        BearSSL::thunk_br_ssl_engine_sendapp_ack(&engine, n);
        t.sent += n;
        br_ssl_engine_flush(&engine, 0);
    } else if ((state & BR_SSL_RECVREC) && t.received) {
        auto data = BearSSL::thunk_br_ssl_engine_recvrec_buf(&engine, &len);
        uint16_t n = pbuf_copy_partial(t.received, data, std::min(len, size_t(t.received->tot_len - t.offset)), t.offset);
        t.offset += n;
        if (t.offset == t.received->tot_len) {
            pbuf_free(t.received);
            t.received = nullptr;
            t.offset = 0;
        }
        tcp_recved(t.socket, n);
        BearSSL::thunk_br_ssl_engine_recvrec_ack(&engine, n);
    } else if ((state & BR_SSL_CLOSED) || t.closed) finish();
}
}
#endif
