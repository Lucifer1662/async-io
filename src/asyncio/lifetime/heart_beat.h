#include "../context.h"
#include "../interval.h"
#include <memory>

void nothing(bool) {}

template <typename Func, typename FailureFunc> struct HeartBeat {
    bool previously_succeeded = true;
    Func heart_beat;
    FailureFunc failure;
    using Return_Type = decltype(heart_beat(nothing));
    std::weak_ptr<HeartBeat> self;

    HeartBeat(Func &&heart_beat, FailureFunc &&failure)
        : heart_beat(std::forward<Func>(heart_beat))
        , failure(std::forward<FailureFunc>(failure)) {}

    void update_succeeded(bool status) {
        previously_succeeded = status;
        int w = 0;
    }

    // return true to continue interval or false to stop
    bool process() {
        // did not receive heart beat within interval
        if (!previously_succeeded) {
            failure();
            // cancel interval
            return false;
        }

        previously_succeeded = false;

        // check whether return type is bool or void
        if constexpr (std::is_same<Return_Type, bool>::value) {
            // create shared_ptr of self to give to heart beat callback so it always calls valid memory
            bool is_fine = heart_beat([self = self.lock()](bool succeeded) { self->previously_succeeded = succeeded; });
            if (!is_fine) {
                failure();
                // cancel interval
                return false;
            }
        } else {
            // create shared_ptr of self to give to heart beat callback so it always calls valid memory
            heart_beat([self = self.lock()](bool succeeded) { self->update_succeeded(succeeded); });
        }

        // continue interval
        return true;
    }
};

// heart_beat signature expected as void( void(bool succeeded) );
// failure_callback expected as void();
template <typename Func, typename FailureFunc>
void make_heart_beat(Context &context, long long interval_ms, Func &&heart_beat, FailureFunc &&failure) {

    auto heart_beat_object = std::make_shared<HeartBeat<Func, FailureFunc>>(std::forward<Func>(heart_beat),
                                                                            std::forward<FailureFunc>(failure));
    heart_beat_object->self = heart_beat_object;

    make_interval(context, interval_ms, [=]() { return heart_beat_object->process(); });
}

// // heart_beat signature expected as void( void(bool succeeded) );
// // failure_callback expected as void();
// template <typename Func, typename FailureFunc>
// void make_heart_beat(Context &context, long long interval_ms, Func &&heart_beat, FailureFunc &&failure) {
//     // get return type of function
//     auto sub = [](bool succeeded) {};
//     using Return_Type = decltype(heart_beat(sub));

//     make_interval(context, interval_ms,
//                   [heart_beat = std::forward<Func>(heart_beat), failure = std::forward<FailureFunc>(failure),
//                    previously_succeeded = std::make_shared<bool>(true)

//     ]() mutable {
//                       // did not receive heart beat within interval
//                       if (!*previously_succeeded) {
//                           failure();
//                           // cancel interval
//                           return false;
//                       }

//                       *previously_succeeded = false;

//                       // check whether return type is bool or void
//                       if constexpr (std::is_same<Return_Type, bool>::value) {
//                           bool is_fine = heart_beat([=](bool succeeded) { *previously_succeeded = succeeded; });
//                           if (!is_fine) {
//                               failure();
//                               // cancel interval
//                               return false;
//                           }
//                       } else {
//                           heart_beat([=](bool succeeded) { *previously_succeeded = succeeded; });
//                       }

//                       // continue interval
//                       return true;
//                   });
// }