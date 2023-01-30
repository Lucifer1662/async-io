#pragma once
#include <functional>
#include <list>
#include <tuple>
#include <utility>

#include "../buffer/buffer.h"

template <typename T, typename S> std::unique_ptr<T> dynamic_pointer_cast(std::unique_ptr<S> &&p) noexcept {
    auto converted = std::unique_ptr<T>{dynamic_cast<T *>(p.get())};
    if (converted) {
        p.release();   // no longer owns the pointer
    }
    return converted;
}

template <typename BaseOp> class AsyncOperation {
    struct Request {

        std::function<void(Buffer_Ptr, bool)> callback;
        Buffer_Ptr buffer;

        template <typename Func>
        Request(Func &&callback, Buffer_Ptr buffer)
            : callback(std::forward<Func>(callback))
            , buffer(std::move(buffer)) {}

        Request(const Request &) = default;
        Request(Request &&) = default;
        Request &operator=(const Request &) = default;
    };

    std::list<Request> operation_requests;

    std::vector<char> over_consumed_data;
    int over_consumed_data_start = 0;

  public:
    BaseOp base_operation;

    AsyncOperation(BaseOp base_operation)
        : base_operation(std::move(base_operation)) {}

    size_t wants_more() { return operation_requests.size(); }

    // call back is void(std::unique_ptr<Buffer_T>, bool) type
    template <typename Buffer_T, typename Func> void request(std::unique_ptr<Buffer_T> b, Func &&callback) {
        operation_requests.emplace_back(
            [callback = std::forward<Func>(callback)](Buffer_Ptr buffer, bool error) mutable {
                callback(dynamic_pointer_cast<Buffer_T>(std::move(buffer)), error);
            },
            std::move(b));
    }

    bool check_requests() {
        handle_old_overwrite();

        while (!operation_requests.empty()) {
            auto &front = operation_requests.front();
            auto [over_wrote, unused_data, possibly_more, finished, num_written] = normal_operation(*front.buffer);

            if (over_wrote > 0) {
                handle_overwrite(unused_data, over_wrote);
            } else {
                // handle overwrite will finish us off
                if (finished) {
                    front.callback(std::move(front.buffer), false);
                    operation_requests.pop_front();
                }
            }
            if (!possibly_more) {
                break;
            }
        }

        // if there are no operation requests remaining, or there was none to
        // begin with, then there potentially may be some more operations that
        // could be requested however if there are more operations left, then we
        // can assume we have consumed as much as possible

        // return that we want to unsubscribe
        return operation_requests.empty();
    }

  private:
    void handle_old_overwrite() {
        while (!operation_requests.empty() && over_consumed_data.size() - over_consumed_data_start > 0) {
            auto &front = operation_requests.front();

            auto [num_unused, unused_data, possibly_more, finished, num_written] =
                copy_operator(*front.buffer, over_consumed_data.data() + over_consumed_data_start,
                              over_consumed_data.size() - over_consumed_data_start);

            auto num_consumed = num_written - num_unused;
            over_consumed_data_start += num_consumed;

            // handle overwrite will finish us off
            if (finished) {
                front.callback(std::move(front.buffer), false);
                operation_requests.pop_front();
            }
        }
    }

    void handle_overwrite(char *d, int n) {
        // by default there should be 1 request completed that has over consumed
        auto number_to_process = 1;
        auto current = std::next(operation_requests.begin());
        while (operation_requests.size() > number_to_process) {
            auto &front = *current;

            auto [num_unused, unused_data, possibly_more, finished, num_written] = copy_operator(*front.buffer, d, n);
            auto num_used = num_written - num_unused;
            n -= num_used;
            d += num_used;

            if (finished) {
                // completed writing to this request
                // advance to next
                current = std::next(current);
                number_to_process++;
                continue;
            }
        }

        // save the extra data somewhere
        if (n != 0) {
            over_consumed_data.assign(d, d + n);
            over_consumed_data_start = 0;
        }

        // process requests in the same order as submitted
        // process requests that were completed
        for (size_t i = 0; i < number_to_process; i++) {
            auto &front = operation_requests.front();
            front.callback(std::move(front.buffer), false);
            operation_requests.pop_front();
        }
    }

    template <typename Func> std::tuple<int, char *, bool, bool, int> operation(Buffer &b, Func &&func) {
        int total_consumed = 0;
        for (;;) {
            auto [data, size] = b.contiguous();
            if (size == 0) {
                return {0, 0, true, true, total_consumed};
            }

            int num_written = func(data, size);
            if (num_written == -1) {
                // probably should report error at some point :/
                num_written = 0;
            }

            total_consumed += num_written;

            int num_unused = b.advance(num_written);
            auto possibly_more = num_written == size;

            if (!possibly_more || num_unused != 0) {
                return {num_unused, data + (num_written - num_unused), possibly_more, num_unused != 0, total_consumed};
            }
        }
    }

    auto normal_operation(Buffer &b) {
        return operation(b, [=](auto a, auto b) { return this->base_operation(a, b); });
    }

    auto copy_operator(Buffer &b, char *d, int n) {
        return operation(b, [=](auto data, auto size) {
            auto amount_to_copy = n < size ? n : size;
            memcpy(data, d, amount_to_copy);
            return amount_to_copy;
        });
    }
};
