#include "util.h"

extern "C" {
#include <libavutil/error.h>
}

namespace medi_cloud::util
{
    std::string get_err_msg(const int err_code)
    {
        std::string err_msg;
        av_strerror(err_code, err_msg.data(), AV_ERROR_MAX_STRING_SIZE);
        return err_msg;
    }
}
